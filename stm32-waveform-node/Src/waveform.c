/*
 * waveform.c
 * Purpose:      VCO frequency-sweep module — linear voltage ramp output via DAC1,
 *               driven by TIM2 ISR, calibrated via a frequency/voltage lookup table
 * Dependencies: waveform.h, config.h
 */
#include "waveform.h"
#include "config.h"
#include <string.h>

/* --------------------------------------------------------------------------
 * Module state — all volatile because they are read/written from ISR context
 * -------------------------------------------------------------------------- */
static DAC_HandleTypeDef *s_hdac  = NULL;
static TIM_HandleTypeDef *s_htim2 = NULL;

static volatile WaveformStatus_t s_status = WAVEFORM_STOPPED;

static CalibrationPoint_t s_cal_table[CALIBRATION_POINTS];
static volatile uint8_t   s_num_cal_points = 0U;

static volatile uint16_t s_min_freq_hz  = 0U;
static volatile uint16_t s_max_freq_hz  = 0U;
static volatile uint16_t s_min_volt_mv  = 0U;  /* Voltage for min_freq_hz */
static volatile uint16_t s_max_volt_mv  = 0U;  /* Voltage for max_freq_hz */

/* Current sweep position */
static volatile uint16_t s_step_index   = 0U;   /* 0 .. WAVEFORM_STEPS-1 */
static volatile int8_t   s_direction    = 1;    /* +1 = ramping up, -1 = down */
static volatile uint16_t s_current_volt_mv  = 0U;
static volatile uint16_t s_current_freq_hz  = 0U;

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
static uint16_t interp_voltage(uint16_t freq_hz)
{
    if (s_num_cal_points == 0U) return 0U;

    /* Clamp to table range */
    if (freq_hz <= s_cal_table[0].freq_hz)
        return s_cal_table[0].voltage_mv;
    if (freq_hz >= s_cal_table[s_num_cal_points - 1U].freq_hz)
        return s_cal_table[s_num_cal_points - 1U].voltage_mv;

    /* Binary search for surrounding pair */
    for (uint8_t i = 0U; i < (uint8_t)(s_num_cal_points - 1U); i++)
    {
        if (freq_hz >= s_cal_table[i].freq_hz &&
            freq_hz <= s_cal_table[i + 1U].freq_hz)
        {
            int32_t f1 = (int32_t)s_cal_table[i].freq_hz;
            int32_t f2 = (int32_t)s_cal_table[i + 1U].freq_hz;
            int32_t v1 = (int32_t)s_cal_table[i].voltage_mv;
            int32_t v2 = (int32_t)s_cal_table[i + 1U].voltage_mv;
            int32_t df = f2 - f1;
            if (df == 0) return (uint16_t)v1;

            /* voltage = v1 + (freq - f1) * (v2 - v1) / (f2 - f1) */
            int32_t result = v1 + ((int32_t)(freq_hz - (uint16_t)f1) * (v2 - v1)) / df;
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
 * @brief  Calculate the DAC voltage (mV) for step index i.
 *         Formula: V(i) = min_volt + i * (max_volt - min_volt) / (WAVEFORM_STEPS - 1)
 *         Integer-only; uses 32-bit intermediate to prevent overflow.
 */
static uint16_t voltage_at_step(uint16_t step_i)
{
    if (s_min_volt_mv >= s_max_volt_mv) return s_min_volt_mv;
    uint32_t range = (uint32_t)(s_max_volt_mv - s_min_volt_mv);
    uint32_t v     = (uint32_t)s_min_volt_mv
                   + (range * (uint32_t)step_i) / (uint32_t)(WAVEFORM_STEPS - 1U);
    return (uint16_t)v;
}

/* --------------------------------------------------------------------------
 * Public API
 * -------------------------------------------------------------------------- */

void Waveform_Init(DAC_HandleTypeDef *hdac, TIM_HandleTypeDef *htim2)
{
    s_hdac  = hdac;
    s_htim2 = htim2;
    s_status        = WAVEFORM_STOPPED;
    s_num_cal_points = 0U;
    s_step_index    = 0U;
    s_direction     = 1;
    s_current_volt_mv = 0U;
    s_current_freq_hz = 0U;
}

void Waveform_SetSweepParams(uint16_t min_freq_hz, uint16_t max_freq_hz)
{
    /* Validate */
    if (min_freq_hz < WAVEFORM_MIN_FREQ_HZ ||
        max_freq_hz > WAVEFORM_MAX_FREQ_HZ ||
        min_freq_hz >= max_freq_hz)
    {
        s_status = (WaveformStatus_t)(s_status | WAVEFORM_ERR_RANGE);
        return;
    }

    /* Clear range error */
    s_status = (WaveformStatus_t)(s_status & ~(uint8_t)WAVEFORM_ERR_RANGE);

    s_min_freq_hz = min_freq_hz;
    s_max_freq_hz = max_freq_hz;

    /* Pre-compute endpoint voltages if calibration data is available */
    if (s_num_cal_points > 0U)
    {
        s_min_volt_mv = interp_voltage(min_freq_hz);
        s_max_volt_mv = interp_voltage(max_freq_hz);
    }
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

    /* Recompute endpoint voltages using new table */
    if (s_min_freq_hz < s_max_freq_hz)
    {
        s_min_volt_mv = interp_voltage(s_min_freq_hz);
        s_max_volt_mv = interp_voltage(s_max_freq_hz);
    }

    s_status = (WaveformStatus_t)(s_status & ~(uint8_t)WAVEFORM_ERR_CAL);
}

void Waveform_Start(void)
{
    /* Block start if there are any error flags */
    if ((s_status & WAVEFORM_ERR_RANGE) || (s_status & WAVEFORM_ERR_CAL))
        return;
    if (s_num_cal_points == 0U) return;

    s_step_index  = 0U;
    s_direction   = 1;
    s_status      = WAVEFORM_RUNNING;

    HAL_DAC_Start(s_hdac, DAC_CHANNEL_1);
    HAL_TIM_Base_Start_IT(s_htim2);
}

void Waveform_Stop(void)
{
    HAL_TIM_Base_Stop_IT(s_htim2);
    /* Drive DAC output to 0 V */
    HAL_DAC_SetValue(s_hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, 0U);
    s_status          = WAVEFORM_STOPPED;
    s_current_volt_mv = 0U;
    s_current_freq_hz = 0U;
}

WaveformStatus_t Waveform_GetStatus(void)
{
    return s_status;
}

uint16_t Waveform_GetCurrentFrequency_Hz(void)
{
    return s_current_freq_hz;
}

uint16_t Waveform_GetCurrentVoltage_mV(void)
{
    return s_current_volt_mv;
}

/* --------------------------------------------------------------------------
 * TIM2 ISR callback — advances DAC one step per tick
 *
 * Timer period:
 *   Prescaler = TIM2_PRESCALER (8999)  → tick = 90 MHz / 9000 = 0.1 ms
 *   Period    = TIM2_PERIOD    (99)    → ISR fires every 10 ms
 *   Full sweep (up+down) = 2 × WAVEFORM_STEPS × 10 ms = 2000 ms
 * -------------------------------------------------------------------------- */
void Waveform_TIM2_Callback(void)
{
    if (s_status != WAVEFORM_RUNNING) return;

    /* Calculate voltage for current step (integer, no float) */
    uint16_t volt_mv = voltage_at_step(s_step_index);

    /* Convert mV to 12-bit DAC value:
     * dac = volt_mv * DAC_RESOLUTION / DAC_FULL_SCALE_MV
     * Max intermediate: 3300 * 4095 = 13 513 500  → fits in uint32_t */
    uint32_t dac_val = ((uint32_t)volt_mv * (uint32_t)DAC_RESOLUTION)
                       / (uint32_t)DAC_FULL_SCALE_MV;

    HAL_DAC_SetValue(s_hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, dac_val);

    /* Update state variables read by Modbus input registers */
    s_current_volt_mv = volt_mv;
    s_current_freq_hz = interp_freq(volt_mv);

    /* Advance step and reverse direction at endpoints */
    if (s_direction > 0)
    {
        if (s_step_index >= (uint16_t)(WAVEFORM_STEPS - 1U))
        {
            s_direction   = -1;
            s_step_index  = (uint16_t)(WAVEFORM_STEPS - 2U);
        }
        else
        {
            s_step_index++;
        }
    }
    else
    {
        if (s_step_index == 0U)
        {
            s_direction  = 1;
            s_step_index = 1U;
        }
        else
        {
            s_step_index--;
        }
    }
}
