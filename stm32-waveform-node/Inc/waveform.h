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
 * @param  hdac   Pointer to HAL DAC handle (DAC1)
 * @param  htim2  Pointer to HAL TIM2 handle (step timer)
 */
void Waveform_Init(DAC_HandleTypeDef *hdac, TIM_HandleTypeDef *htim2);

/**
 * @brief  Set the frequency sweep range.
 *         Internally interpolates the required DAC voltage range from the
 *         calibration table.  Stops and restarts the sweep if running.
 * @param  min_freq_hz  Sweep start frequency in Hz (must be < max_freq_hz)
 * @param  max_freq_hz  Sweep end   frequency in Hz
 *         Sets WAVEFORM_ERR_RANGE if min >= max or values are out of
 *         WAVEFORM_MIN_FREQ_HZ / WAVEFORM_MAX_FREQ_HZ bounds.
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

/**
 * @brief  Advance the DAC by one sweep step.
 *         Call from HAL_TIM_PeriodElapsedCallback when htim == htim2.
 */
void Waveform_TIM2_Callback(void);

#endif /* WAVEFORM_H */
