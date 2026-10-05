/*
 * eeprom.h
 * Purpose:      Public API for Flash-emulated EEPROM persistence
 * Dependencies: stm32f4xx_hal.h, waveform.h, config.h
 */
#ifndef EEPROM_H
#define EEPROM_H

#include "stm32f4xx_hal.h"
#include "waveform.h"
#include <stdint.h>

typedef enum
{
    EEPROM_OK          = 0,
    EEPROM_ERR_WRITE   = 1,
    EEPROM_ERR_READ    = 2,
    EEPROM_ERR_INVALID = 3, /**< Magic number mismatch — no valid data stored */
} EepromStatus_t;

/**
 * @brief  Initialise the EEPROM module.  Must be called before Save/Load.
 */
void EEPROM_Init(void);

/**
 * @brief  Persist waveform configuration and calibration data to EEPROM.
 * @param  min_freq_hz   Minimum sweep frequency (Hz)
 * @param  max_freq_hz   Maximum sweep frequency (Hz)
 * @param  sweep_rate_hz Active sweep-ramp rate (excluding inter-sweep pause)
 * @param  pause_us      Hold time between sweeps (microseconds)
 * @param  triangle      Waveform mode (0=sawtooth, 1=triangle)
 * @param  points        Calibration point array
 * @param  num_points    Number of entries in points[] (max CALIBRATION_POINTS)
 * @param  device_addr   Modbus device address (1–247) to persist
 * @return EEPROM_OK on success, EEPROM_ERR_WRITE on Flash erase/program error
 */
EepromStatus_t EEPROM_SaveConfig(uint16_t                 min_freq_hz,
                                 uint16_t                 max_freq_hz,
                                 uint32_t                 sweep_rate_hz,
                                 uint16_t                 pause_us,
                                 uint8_t                  triangle,
                                 const CalibrationPoint_t *points,
                                 uint8_t                  num_points,
                                 uint8_t                  device_addr);

/**
 * @brief  Restore waveform configuration and calibration data from EEPROM.
 * @param  min_freq_hz   Output: minimum sweep frequency (Hz)
 * @param  max_freq_hz   Output: maximum sweep frequency (Hz)
 * @param  sweep_rate_hz Output: active sweep-ramp rate (Hz), excluding pause
 * @param  pause_us      Output: hold time between sweeps (microseconds)
 * @param  triangle      Output: waveform mode (0=sawtooth, 1=triangle)
 * @param  points        Output: calibration array (caller provides CALIBRATION_POINTS entries)
 * @param  num_points    Output: number of valid calibration entries read
 * @param  device_addr   Output: persisted Modbus device address (1–247)
 * @return EEPROM_OK on success, EEPROM_ERR_INVALID if no valid data found
 */
EepromStatus_t EEPROM_LoadConfig(uint16_t          *min_freq_hz,
                                 uint16_t          *max_freq_hz,
                                 uint32_t          *sweep_rate_hz,
                                 uint16_t          *pause_us,
                                  uint8_t           *triangle,
                                 CalibrationPoint_t *points,
                                 uint8_t           *num_points,
                                 uint8_t           *device_addr);

#endif /* EEPROM_H */
