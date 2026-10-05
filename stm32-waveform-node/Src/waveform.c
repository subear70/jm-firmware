/*
 * waveform.c
 * Purpose:      VCO frequency-sweep module — precomputes a full up+down DAC
 *               ramp buffer and streams it to DAC1 by circular DMA, triggered
 *               by TIM2 TRGO.  The active sweep-ramp rate (up to
 *               WAVEFORM_MAX_SWEEP_RATE_HZ) is set by the TIM2 auto-reload.
 * Dependencies: waveform.h, config.h
 */
#include "waveform.h"
#include "config.h"
#include <string.h>

/* --------------------------------------------------------------------------
 * Module state
 * -------------------------------------------------------------------------- */
static DAC_HandleTypeDef *s_hdac  = NULL;
static TIM_HandleTypeDef *s_htim2 = NULL;

static volatile WaveformStatus_t s_status = WAVEFORM_STOPPED;

static CalibrationPoint_t s_cal_table[CALIBRATION_POINTS];
static volatile uint8_t   s_num_cal_points = 0U;

static volatile uint16_t s_min_freq_hz  = 0U;
static volatile uint16_t s_max_freq_hz  = 0U;

/* Active sweep-ramp rate; the configured pause extends each complete cycle. */
static volatile uint32_t s_sweep_rate_hz = WAVEFORM_DEFAULT_SWEEP_RATE_HZ;
static volatile uint16_t s_pause_us = 0U;
static volatile uint8_t s_triangle_enabled = 0U;

/* DAC sample buffer streamed by DMA in circular mode.
 * [0 .. s_active_samples-1] is a single rising ramp; appended pause samples hold
 * the calibrated low endpoint. The ramp flies back there before the pause.
 * 12-bit right-aligned DAC codes; DMA is configured for half-word transfers. */
static uint16_t          s_dac_buffer[WAVEFORM_MAX_SAMPLES + WAVEFORM_MAX_PAUSE_SAMPLES];

/* Active number of DAC samples in the current sweep ramp — recomputed from the
 * sweep rate so the DAC update rate stays <= DAC_MAX_SAMPLE_RATE_HZ. */
static volatile uint16_t s_active_samples = WAVEFORM_MIN_SAMPLES;
static volatile uint16_t s_pause_samples = 0U;
static volatile uint16_t s_dma_samples = WAVEFORM_MIN_SAMPLES;

/* --------------------------------------------------------------------------
 * Internal — integer linear interpolation helpers
 * No floating point used anywhere.
 * -------------------------------------------------------------------------- */

/**
 * @brief  Interpolate DAC voltage (mV) for a given frequency (Hz)
 *         using the calibration table.
 *         Assumes the table is sorted ascending by freq_hz.
 *         Clamps to table endpoints for out-of-range frequencies.
 */
/**
 * @brief  Interpolate DAC voltage (mV) at a fractional sweep frequency given as
 *         freq_num / denom Hz (denom > 0), using the calibration table.
 *         Working in the scaled domain preserves sub-Hz precision so a sweep
 *         spanning only a few Hz still yields a distinct value at every sample
 *         (avoids integer-Hz truncation collapsing the ramp into a few steps).
 *         Assumes the table is sorted ascending by freq_hz; clamps to the table
 *         endpoints for out-of-range frequencies.
 */
static uint16_t interp_voltage_scaled(int32_t freq_num, int32_t denom)
{
    if (s_num_cal_points == 0U || denom <= 0) return 0U;

    int32_t first_f = (int32_t)s_cal_table[0].freq_hz * denom;
    int32_t last_f  = (int32_t)s_cal_table[s_num_cal_points - 1U].freq_hz * denom;

    /* Clamp to table range (scaled domain) */
    if (freq_num <= first_f) return s_cal_table[0].voltage_mv;
    if (freq_num >= last_f)  return s_cal_table[s_num_cal_points - 1U].voltage_mv;

    /* Find the surrounding calibration pair */
    for (uint8_t i = 0U; i < (uint8_t)(s_num_cal_points - 1U); i++)
    {
        int32_t f1 = (int32_t)s_cal_table[i].freq_hz * denom;
        int32_t f2 = (int32_t)s_cal_table[i + 1U].freq_hz * denom;
        if (freq_num >= f1 && freq_num <= f2)
        {
            int32_t v1 = (int32_t)s_cal_table[i].voltage_mv;
            int32_t v2 = (int32_t)s_cal_table[i + 1U].voltage_mv;
            int32_t df = f2 - f1;
            if (df == 0) return (uint16_t)v1;

            /* voltage = v1 + (freq_num - f1) * (v2 - v1) / df   (scaled domain) */
            int64_t result = (int64_t)v1
                           + ((int64_t)(freq_num - f1) * (int64_t)(v2 - v1)) / (int64_t)df;
            if (result < 0)       result = 0;
            if (result > 0xFFFF)  result = 0xFFFF;
            return (uint16_t)result;
        }
    }
    return 0U;
}

/**
 * @brief  Reverse interpolation — derive frequency (Hz) from voltage (mV).
 *         Assumes calibration table voltages are also monotonically increasing.
 */
static uint16_t interp_freq(uint16_t voltage_mv)
{
    if (s_num_cal_points == 0U) return 0U;

    if (voltage_mv <= s_cal_table[0].voltage_mv)
        return s_cal_table[0].freq_hz;
    if (voltage_mv >= s_cal_table[s_num_cal_points - 1U].voltage_mv)
        return s_cal_table[s_num_cal_points - 1U].freq_hz;

    for (uint8_t i = 0U; i < (uint8_t)(s_num_cal_points - 1U); i++)
    {
        if (voltage_mv >= s_cal_table[i].voltage_mv &&
            voltage_mv <= s_cal_table[i + 1U].voltage_mv)
        {
            int32_t v1 = (int32_t)s_cal_table[i].voltage_mv;
            int32_t v2 = (int32_t)s_cal_table[i + 1U].voltage_mv;
            int32_t f1 = (int32_t)s_cal_table[i].freq_hz;
            int32_t f2 = (int32_t)s_cal_table[i + 1U].freq_hz;
            int32_t dv = v2 - v1;
            if (dv == 0) return (uint16_t)f1;

            int32_t result = f1 + ((int32_t)(voltage_mv - (uint16_t)v1) * (f2 - f1)) / dv;
            return (uint16_t)result;
        }
    }
    return 0U;
}

/**
 * @brief  Choose the number of DAC samples for the current sweep rate so the
 *         DAC update rate stays within DAC_MAX_SAMPLE_RATE_HZ.
 *         samples = clamp(DAC_MAX_SAMPLE_RATE_HZ / rate,
 *                         WAVEFORM_MIN_SAMPLES, WAVEFORM_MAX_SAMPLES)
 */
static uint16_t compute_active_samples(uint32_t rate_hz)
{
    if (rate_hz == 0U) rate_hz = 1U;
    uint32_t n = DAC_MAX_SAMPLE_RATE_HZ / rate_hz;
    if (n > (uint32_t)WAVEFORM_MAX_SAMPLES) n = (uint32_t)WAVEFORM_MAX_SAMPLES;
    if (n < (uint32_t)WAVEFORM_MIN_SAMPLES) n = (uint32_t)WAVEFORM_MIN_SAMPLES;
    if (s_triangle_enabled && n < (uint32_t)WAVEFORM_MIN_TRIANGLE_SAMPLES)
        n = (uint32_t)WAVEFORM_MIN_TRIANGLE_SAMPLES;
    return (uint16_t)n;
}

/** Convert the requested pause to held DAC samples at the current sample rate. */
static uint16_t compute_pause_samples(uint32_t rate_hz,
                                      uint16_t active_samples,
                                      uint16_t pause_us)
{
    if (rate_hz == 0U || active_samples == 0U || pause_us == 0U)
        return 0U;

    uint32_t arr = WAVEFORM_TIM2_ARR(rate_hz, active_samples);
    if (arr < 1U) arr = 1U;

    uint64_t numerator = (uint64_t)pause_us * (uint64_t)TIM2_CLK_HZ;
    uint64_t denominator = 1000000ULL * ((uint64_t)arr + 1ULL);
    uint64_t samples = (numerator + denominator - 1ULL) / denominator;
    if (samples > WAVEFORM_MAX_PAUSE_SAMPLES)
        samples = WAVEFORM_MAX_PAUSE_SAMPLES;
    return (uint16_t)samples;
}

/** Convert a voltage in mV to a 12-bit right-aligned DAC code. */
static uint16_t mv_to_dac(uint16_t volt_mv)
{
    /* Max intermediate: 3300 * 4095 = 13 513 500 → fits in uint32_t */
    uint32_t code = ((uint32_t)volt_mv * (uint32_t)DAC_RESOLUTION)
                    / (uint32_t)DAC_FULL_SCALE_MV;
    if (code > (uint32_t)DAC_RESOLUTION) code = (uint32_t)DAC_RESOLUTION;
    return (uint16_t)code;
}

/** Convert a 12-bit DAC code back to a voltage in mV. */
static uint16_t dac_to_mv(uint16_t dac_code)
{
    uint32_t mv = ((uint32_t)dac_code * (uint32_t)DAC_FULL_SCALE_MV)
                  / (uint32_t)DAC_RESOLUTION;
    return (uint16_t)mv;
}

/**
 * @brief  Precompute the selected waveform sample buffer streamed by DMA.
 *         Sawtooth mode rises from min_freq_hz to max_freq_hz; triangle mode
 *         rises and falls across s_active_samples points. Each frequency maps
 *         to its DAC voltage
 *         through the calibration table (interp_voltage_scaled).  The frequency
 *         is kept as a scaled fraction so the swept frequency is linear and every
 *         sample is distinct even over a narrow (few-Hz) sweep range.
 *         A single rising ramp fills [0 .. s_active_samples-1]; pause points
 *         hold the calibrated low endpoint before the next ramp starts.
 */
static void build_dac_buffer(void)
{
    uint16_t n = s_active_samples;
    if (n < WAVEFORM_MIN_SAMPLES) n = WAVEFORM_MIN_SAMPLES;

    int32_t denom   = (int32_t)(n - 1U);
    int32_t f_start = (int32_t)s_min_freq_hz;
    int32_t f_span  = (int32_t)s_max_freq_hz - f_start;

    for (uint16_t i = 0U; i < n; i++)
    {
        int32_t position_num = (int32_t)i;
        if (s_triangle_enabled && n > 2U)
        {
            uint16_t peak_left = (uint16_t)(denom / 2);
            uint16_t peak_right = (uint16_t)((denom + 1) / 2);
            if (i < peak_left)
                position_num = ((int32_t)i * denom) / peak_left;
            else if (i <= peak_right)
                position_num = denom;
            else
                position_num = ((int32_t)(n - 1U - i) * denom) /
                               (int32_t)(denom - peak_right);
        }

        /* Keep frequency in a scaled fractional domain for calibration. */
        int32_t freq_num = f_start * denom + f_span * position_num;
        if (freq_num < 0) freq_num = 0;
        s_dac_buffer[i] = mv_to_dac(interp_voltage_scaled(freq_num, denom));
    }

    for (uint16_t i = 0U; i < s_pause_samples; i++)
        s_dac_buffer[n + i] = s_dac_buffer[0];
}

/**
 * @brief  Program the TIM2 auto-reload so the DAC-DMA streams the sample
 *         buffer at the configured sweep rate.
 *         ARR = TIM2_CLK_HZ / (rate * s_active_samples) - 1
 */
static void apply_timer_rate(void)
{
    uint32_t arr = WAVEFORM_TIM2_ARR(s_sweep_rate_hz, s_active_samples);
    if (arr < 1U) arr = 1U;   /* keep at least one tick per sample */

    __HAL_TIM_SET_PRESCALER(s_htim2, TIM2_PRESCALER);
    __HAL_TIM_SET_AUTORELOAD(s_htim2, arr);
    /* Force the new prescaler/ARR to load immediately and realign the sweep */
    (void)HAL_TIM_GenerateEvent(s_htim2, TIM_EVENTSOURCE_UPDATE);
}

/**
 * @brief  Silence the DAC DMA interrupts used during circular streaming.
 *         Circular DAC streaming needs no per-cycle CPU work; the buffer just
 *         loops forever.  HAL_DAC_Start_DMA leaves several interrupts enabled
 *         that are harmful at high sweep rates (small buffers):
 *           - DMA half/complete (HT/TC) fire every DMA cycle and, at MHz rates,
 *             saturate the CPU with interrupts.
 *           - The DAC DMA-underrun IRQ (DAC_IT_DMAUDR1) shares the TIM6_DAC IRQ
 *             with the Modbus timer; on a momentary underrun HAL's handler
 *             clears DMAEN1 and *permanently stops the sweep*.
 *         Disabling all three lets the DMA free-run; a stray underrun merely
 *         repeats a sample instead of freezing or stalling the firmware.
 */
static void dac_dma_silence_irq(void)
{
    if (s_hdac == NULL) return;

    /* Ignore DAC DMA underruns (do not let HAL stop the stream on one) */
    __HAL_DAC_DISABLE_IT(s_hdac, DAC_IT_DMAUDR1);

    if (s_hdac->DMA_Handle1 != NULL)
        __HAL_DMA_DISABLE_IT(s_hdac->DMA_Handle1, DMA_IT_HT | DMA_IT_TC);
}

static void restart_dma_stream(void)
{
    if (!(s_status & WAVEFORM_RUNNING)) return;

    build_dac_buffer();
    HAL_TIM_Base_Stop(s_htim2);
    HAL_DAC_Stop_DMA(s_hdac, DAC_CHANNEL_1);
    apply_timer_rate();
    __HAL_DAC_CLEAR_FLAG(s_hdac, DAC_FLAG_DMAUDR1);
    (void)HAL_DAC_Start_DMA(s_hdac, DAC_CHANNEL_1,
                            (uint32_t *)s_dac_buffer, s_dma_samples,
                            DAC_ALIGN_12B_R);
    dac_dma_silence_irq();
    HAL_TIM_Base_Start(s_htim2);
}

/* --------------------------------------------------------------------------
 * Public API
 * -------------------------------------------------------------------------- */

void Waveform_Init(DAC_HandleTypeDef *hdac, TIM_HandleTypeDef *htim2)
{
    s_hdac  = hdac;
    s_htim2 = htim2;
    s_status         = WAVEFORM_STOPPED;
    s_num_cal_points = 0U;
    s_sweep_rate_hz  = WAVEFORM_DEFAULT_SWEEP_RATE_HZ;
    s_pause_us       = 0U;
    s_triangle_enabled = 0U;
    s_active_samples = compute_active_samples(WAVEFORM_DEFAULT_SWEEP_RATE_HZ);
    s_pause_samples  = 0U;
    s_dma_samples    = s_active_samples;
    memset(s_dac_buffer, 0, sizeof(s_dac_buffer));
}

void Waveform_SetSweepParams(uint16_t min_freq_hz, uint16_t max_freq_hz)
{
    /* No range/ordering limits are enforced — any frequency pair is accepted.
     * Clear any previously latched range error. */
    s_status = (WaveformStatus_t)(s_status & ~(uint8_t)WAVEFORM_ERR_RANGE);

    s_min_freq_hz = min_freq_hz;
    s_max_freq_hz = max_freq_hz;
    /* Per-sample voltages are derived from the calibration table in
     * build_dac_buffer() when the sweep (re)starts. */
}

void Waveform_SetCalibrationData(const CalibrationPoint_t *points,
                                 uint8_t                   num_points)
{
    if (points == NULL || num_points == 0U || num_points > CALIBRATION_POINTS)
    {
        s_status = (WaveformStatus_t)(s_status | WAVEFORM_ERR_CAL);
        return;
    }

    memcpy(s_cal_table, points, (size_t)num_points * sizeof(CalibrationPoint_t));
    s_num_cal_points = num_points;

    s_status = (WaveformStatus_t)(s_status & ~(uint8_t)WAVEFORM_ERR_CAL);
}

void Waveform_SetSweepRate(uint32_t sweeps_per_sec)
{
    uint32_t max_sweep_rate = s_triangle_enabled
        ? WAVEFORM_MAX_TRIANGLE_SWEEP_RATE_HZ
        : WAVEFORM_MAX_SWEEP_RATE_HZ;
    if (sweeps_per_sec < WAVEFORM_MIN_SWEEP_RATE_HZ)
        sweeps_per_sec = WAVEFORM_MIN_SWEEP_RATE_HZ;
    else if (sweeps_per_sec > max_sweep_rate)
        sweeps_per_sec = max_sweep_rate;

    s_sweep_rate_hz = sweeps_per_sec;

    uint16_t new_samples = compute_active_samples(s_sweep_rate_hz);
    uint16_t new_pause_samples = compute_pause_samples(s_sweep_rate_hz,
                                                       new_samples,
                                                       s_pause_us);

    /* Apply live if a sweep is currently running */
    if (s_status & WAVEFORM_RUNNING)
    {
        if (new_samples != s_active_samples || new_pause_samples != s_pause_samples)
        {
            /* Sample count changed: rebuild the ramp and restart the circular
             * DMA with the new transfer length.  Halt TIM2 first so no TRGO /
             * DAC conversion occurs while the DMA is being reconfigured — that
             * would latch a DMA-underrun the moment HAL_DAC_Start_DMA re-enables
             * the underrun IRQ, which HAL services by killing the stream.
             * Mirrors the safe start-last ordering in Waveform_Start(). */
            s_active_samples = new_samples;
            s_pause_samples = new_pause_samples;
            s_dma_samples = (uint16_t)(s_active_samples + s_pause_samples);
            build_dac_buffer();

            HAL_TIM_Base_Stop(s_htim2);
            HAL_DAC_Stop_DMA(s_hdac, DAC_CHANNEL_1);
            apply_timer_rate();
            __HAL_DAC_CLEAR_FLAG(s_hdac, DAC_FLAG_DMAUDR1);
            (void)HAL_DAC_Start_DMA(s_hdac, DAC_CHANNEL_1,
                                    (uint32_t *)s_dac_buffer, s_dma_samples,
                                    DAC_ALIGN_12B_R);
            dac_dma_silence_irq();
            HAL_TIM_Base_Start(s_htim2);
        }
        else
        {
            apply_timer_rate();
        }
    }
    else
    {
        s_active_samples = new_samples;
        s_pause_samples = new_pause_samples;
        s_dma_samples = (uint16_t)(s_active_samples + s_pause_samples);
    }
}

uint32_t Waveform_GetSweepRate_Hz(void)
{
    return s_sweep_rate_hz;
}

void Waveform_SetPauseUs(uint16_t pause_us)
{
    if (pause_us > WAVEFORM_MAX_PAUSE_US)
        pause_us = WAVEFORM_MAX_PAUSE_US;

    s_pause_us = pause_us;
    s_pause_samples = compute_pause_samples(s_sweep_rate_hz,
                                            s_active_samples,
                                            s_pause_us);
    s_dma_samples = (uint16_t)(s_active_samples + s_pause_samples);

    restart_dma_stream();
}

uint16_t Waveform_GetPauseUs(void)
{
    return s_pause_us;
}

void Waveform_SetTriangleEnabled(uint8_t enabled)
{
    uint8_t triangle_enabled = (enabled != 0U) ? 1U : 0U;
    if (triangle_enabled == s_triangle_enabled) return;

    s_triangle_enabled = triangle_enabled;
    uint32_t max_sweep_rate = triangle_enabled
        ? WAVEFORM_MAX_TRIANGLE_SWEEP_RATE_HZ
        : WAVEFORM_MAX_SWEEP_RATE_HZ;
    if (s_sweep_rate_hz > max_sweep_rate)
        s_sweep_rate_hz = max_sweep_rate;
    s_active_samples = compute_active_samples(s_sweep_rate_hz);
    s_pause_samples = compute_pause_samples(s_sweep_rate_hz,
                                            s_active_samples,
                                            s_pause_us);
    s_dma_samples = (uint16_t)(s_active_samples + s_pause_samples);
    restart_dma_stream();
}

void Waveform_Start(void)
{
    /* Block start if there are any error flags */
    if ((s_status & WAVEFORM_ERR_RANGE) || (s_status & WAVEFORM_ERR_CAL))
        return;
    if (s_num_cal_points == 0U) return;

    /* Already running — nothing to do (avoid restarting the DMA stream) */
    if (s_status & WAVEFORM_RUNNING) return;

    /* Choose the sample count for the current rate, then precompute the ramp */
    s_active_samples = compute_active_samples(s_sweep_rate_hz);
    s_pause_samples = compute_pause_samples(s_sweep_rate_hz,
                                             s_active_samples,
                                             s_pause_us);
    s_dma_samples = (uint16_t)(s_active_samples + s_pause_samples);
    build_dac_buffer();

    /* Route the DAC conversion trigger to TIM2 TRGO for DMA streaming */
    DAC_ChannelConfTypeDef sConfig = {0};
    sConfig.DAC_Trigger      = DAC_TRIGGER_T2_TRGO;
    sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
    HAL_DAC_Stop(s_hdac, DAC_CHANNEL_1);
    (void)HAL_DAC_ConfigChannel(s_hdac, &sConfig, DAC_CHANNEL_1);

    /* Program the sweep rate, then start circular DMA + trigger timer */
    apply_timer_rate();

    if (HAL_DAC_Start_DMA(s_hdac, DAC_CHANNEL_1,
                          (uint32_t *)s_dac_buffer, s_dma_samples,
                          DAC_ALIGN_12B_R) != HAL_OK)
    {
        return;
    }
    dac_dma_silence_irq();
    HAL_TIM_Base_Start(s_htim2);

    s_status = WAVEFORM_RUNNING;
}

void Waveform_Stop(void)
{
    HAL_TIM_Base_Stop(s_htim2);
    HAL_DAC_Stop_DMA(s_hdac, DAC_CHANNEL_1);

    /* Switch the channel to software update and drive the output to 0 V */
    DAC_ChannelConfTypeDef sConfig = {0};
    sConfig.DAC_Trigger      = DAC_TRIGGER_NONE;
    sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
    (void)HAL_DAC_ConfigChannel(s_hdac, &sConfig, DAC_CHANNEL_1);
    HAL_DAC_SetValue(s_hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, 0U);
    HAL_DAC_Start(s_hdac, DAC_CHANNEL_1);

    s_status = WAVEFORM_STOPPED;
}

WaveformStatus_t Waveform_GetStatus(void)
{
    return s_status;
}

uint16_t Waveform_GetCurrentFrequency_Hz(void)
{
    return interp_freq(Waveform_GetCurrentVoltage_mV());
}

uint16_t Waveform_GetCurrentVoltage_mV(void)
{
    if (!(s_status & WAVEFORM_RUNNING)) return 0U;
    if (s_hdac == NULL || s_hdac->DMA_Handle1 == NULL) return 0U;

    /* Derive the sample currently being output from the DMA transfer counter.
     * NDTR counts down from s_active_samples to 1. */
    uint16_t n = s_dma_samples;
    uint32_t remaining = __HAL_DMA_GET_COUNTER(s_hdac->DMA_Handle1);
    uint16_t index = 0U;
    if (remaining > 0U && remaining <= n)
        index = (uint16_t)(n - remaining);

    return dac_to_mv(s_dac_buffer[index]);
}
