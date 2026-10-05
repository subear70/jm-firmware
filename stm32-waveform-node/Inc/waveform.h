/*
 * waveform.h
 * Purpose:      Public API for the VCO frequency-sweep waveform module
 * Dependencies: stdint.h, config.h
 */
#ifndef WAVEFORM_H
#define WAVEFORM_H

#include "stm32f4xx_hal.h"
#include "config.h"
#include <stdint.h>

/* --------------------------------------------------------------------------
 * Types
 * -------------------------------------------------------------------------- */

/** One entry in the frequency-to-voltage calibration table. */
typedef struct
{
    uint16_t freq_hz;    /**< Jammer output frequency at this DAC voltage */
    uint16_t voltage_mv; /**< DAC output voltage (mV) required to reach freq_hz */
} CalibrationPoint_t;

/** Waveform module status flags (may be ORed). */
typedef enum
{
    WAVEFORM_STOPPED   = 0x00U, /**< Output idle, DAC held at 0 V */
    WAVEFORM_RUNNING   = 0x01U, /**< Sweep active */
    WAVEFORM_ERR_RANGE = 0x02U, /**< min_freq_hz >= max_freq_hz, or out-of-range */
    WAVEFORM_ERR_CAL   = 0x04U, /**< No valid calibration data loaded */
} WaveformStatus_t;

/* --------------------------------------------------------------------------
 * API
 * -------------------------------------------------------------------------- */

/**
 * @brief  Initialise the waveform module.
 *         Must be called once before any other Waveform_* function.
 * @param  hdac   Pointer to HAL DAC handle (DAC1, triggered by TIM2 TRGO)
 * @param  htim2  Pointer to HAL TIM2 handle (DAC-DMA sweep trigger timer)
 */
void Waveform_Init(DAC_HandleTypeDef *hdac, TIM_HandleTypeDef *htim2);

/**
 * @brief  Set the active sweep-ramp rate, excluding the inter-sweep pause.
 *         Reprograms the TIM2 auto-reload so the DAC-DMA streams the sample
 *         buffer at sweep_rate * N samples/second, where N is the dynamic DAC
 *         sample count chosen from the rate (WAVEFORM_MIN/MAX_SAMPLES).  If N
 *         changes the ramp buffer is rebuilt and the DMA restarted.
 *         Takes effect immediately while running.
 * @param  sweeps_per_sec  Requested rate, clamped to the supported limits.
 *         Triangle mode is additionally capped to WAVEFORM_MAX_TRIANGLE_SWEEP_RATE_HZ.
 */
void Waveform_SetSweepRate(uint32_t sweeps_per_sec);

/**
 * @brief  Return the currently configured active sweep-ramp rate.
 * @return Sweep rate in Hz, excluding the inter-sweep pause
 */
uint32_t Waveform_GetSweepRate_Hz(void);

/**
 * @brief  Set the hold time at the calibrated low DAC level between sweeps.
 *         The requested pause is rounded up to the next DAC sample interval.
 * @param  pause_us  Pause in microseconds, clamped to WAVEFORM_MAX_PAUSE_US.
 */
void Waveform_SetPauseUs(uint16_t pause_us);

/**
 * @brief  Return the configured hold time between completed sweeps.
 * @return Pause in microseconds
 */
uint16_t Waveform_GetPauseUs(void);

/**
 * @brief  Select triangle mode when nonzero; zero selects sawtooth mode.
 *         Rebuilds the DMA buffer immediately if the waveform is running.
 * @param  enabled  Boolean waveform selection (0 = sawtooth, 1 = triangle)
 */
void Waveform_SetTriangleEnabled(uint8_t enabled);

/**
 * @brief  Set the frequency sweep range.
 *         Internally interpolates the required DAC voltage range from the
 *         calibration table.  Stops and restarts the sweep if running.
 *         No range or ordering limits are enforced — any frequency pair is
 *         accepted (out-of-calibration frequencies clamp to the table ends).
 * @param  min_freq_hz  Sweep start frequency in Hz
 * @param  max_freq_hz  Sweep end   frequency in Hz
 */
void Waveform_SetSweepParams(uint16_t min_freq_hz, uint16_t max_freq_hz);

/**
 * @brief  Load the frequency-to-voltage calibration table.
 *         Points must be sorted ascending by freq_hz.
 *         Clears WAVEFORM_ERR_CAL on success.
 * @param  points     Array of CalibrationPoint_t entries
 * @param  num_points Number of valid entries (1 – CALIBRATION_POINTS)
 */
void Waveform_SetCalibrationData(const CalibrationPoint_t *points,
                                 uint8_t                   num_points);

/**
 * @brief  Start the frequency sweep.
 *         Requires valid sweep params and calibration data.
 *         No-op if WAVEFORM_ERR_RANGE or WAVEFORM_ERR_CAL is set.
 */
void Waveform_Start(void);

/**
 * @brief  Stop the sweep and drive the DAC output to 0 V.
 */
void Waveform_Stop(void);

/**
 * @brief  Return current module status flags.
 * @return WaveformStatus_t bitmask
 */
WaveformStatus_t Waveform_GetStatus(void);

/**
 * @brief  Return the current sweep frequency derived by reverse interpolation
 *         of the current DAC voltage against the calibration table.
 * @return Frequency in Hz, or 0 if not running / no calibration data
 */
uint16_t Waveform_GetCurrentFrequency_Hz(void);

/**
 * @brief  Return the current DAC output voltage.
 * @return Voltage in mV
 */
uint16_t Waveform_GetCurrentVoltage_mV(void);

#endif /* WAVEFORM_H */
