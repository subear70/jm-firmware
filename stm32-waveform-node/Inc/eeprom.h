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
 * @param  points        Calibration point array
 * @param  num_points    Number of entries in points[] (max CALIBRATION_POINTS)
 * @return EEPROM_OK on success, EEPROM_ERR_WRITE on I2C error
 */
EepromStatus_t EEPROM_SaveConfig(uint16_t                 min_freq_hz,
                                 uint16_t                 max_freq_hz,
                                 const CalibrationPoint_t *points,
                                 uint8_t                  num_points);

/**
 * @brief  Restore waveform configuration and calibration data from EEPROM.
 * @param  min_freq_hz  Output: minimum sweep frequency (Hz)
 * @param  max_freq_hz  Output: maximum sweep frequency (Hz)
 * @param  points       Output: calibration array (caller provides CALIBRATION_POINTS entries)
 * @param  num_points   Output: number of valid calibration entries read
 * @return EEPROM_OK on success, EEPROM_ERR_INVALID if no valid data found
 */
EepromStatus_t EEPROM_LoadConfig(uint16_t          *min_freq_hz,
                                 uint16_t          *max_freq_hz,
                                 CalibrationPoint_t *points,
                                 uint8_t           *num_points);

#endif /* EEPROM_H */
