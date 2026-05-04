/*
 * eeprom.c
 * Purpose:      Flash-emulated EEPROM driver (STM32F446RE Sector 7)
 *               Persists waveform config and calibration data.
 *               Strategy: erase sector, then program word-by-word.
 *               Write frequency is low (only on explicit calibration save).
 * Dependencies: eeprom.h, config.h
 */
#include "eeprom.h"
#include "config.h"
#include <string.h>

/* --------------------------------------------------------------------------
 * Stored layout at EEPROM_FLASH_ADDR:
 *
 *  Offset  Size  Field
 *  0       2     magic (EEPROM_MAGIC)
 *  2       2     min_freq_hz
 *  4       2     max_freq_hz
 *  6       1     num_cal_points
 *  7       1     (reserved/pad)
 *  8       40    cal_points[10]  (each 4 bytes: freq_hz uint16 + voltage_mv uint16)
 *  Total:  48 bytes
 * -------------------------------------------------------------------------- */

typedef struct __attribute__((packed))
{
    uint16_t           magic;
    uint16_t           min_freq_hz;
    uint16_t           max_freq_hz;
    uint8_t            num_cal_points;
    uint8_t            reserved;
    CalibrationPoint_t cal_points[CALIBRATION_POINTS];
} EepromBlock_t;

/* Round up to 32-bit word count for Flash programming */
#define EEPROM_BLOCK_WORDS  ((sizeof(EepromBlock_t) + 3U) / 4U)

/* --------------------------------------------------------------------------
 * Public API
 * -------------------------------------------------------------------------- */

void EEPROM_Init(void)
{
    /* Nothing to initialise — Flash is always accessible for reads */
}

EepromStatus_t EEPROM_SaveConfig(uint16_t                 min_freq_hz,
                                  uint16_t                 max_freq_hz,
                                  const CalibrationPoint_t *points,
                                  uint8_t                  num_points)
{
    /* Build the block in a word-aligned buffer (0xFF = erased Flash value) */
    uint8_t buf[EEPROM_BLOCK_WORDS * 4U];
    memset(buf, 0xFFU, sizeof(buf));

    EepromBlock_t blk;
    memset(&blk, 0, sizeof(blk));
    blk.magic          = EEPROM_MAGIC;
    blk.min_freq_hz    = min_freq_hz;
    blk.max_freq_hz    = max_freq_hz;
    blk.num_cal_points = num_points;

    uint8_t n = (num_points > CALIBRATION_POINTS) ? CALIBRATION_POINTS : num_points;
    memcpy(blk.cal_points, points, (size_t)n * sizeof(CalibrationPoint_t));
    memcpy(buf, &blk, sizeof(blk));

    HAL_FLASH_Unlock();

    /* Erase the sector */
    FLASH_EraseInitTypeDef erase;
    erase.TypeErase    = FLASH_TYPEERASE_SECTORS;
    erase.Sector       = EEPROM_FLASH_SECTOR;
    erase.NbSectors    = 1U;
    erase.VoltageRange = FLASH_VOLTAGE_RANGE_3;  /* 2.7 V – 3.6 V supply */

    uint32_t sector_error = 0U;
    if (HAL_FLASHEx_Erase(&erase, &sector_error) != HAL_OK)
    {
        HAL_FLASH_Lock();
        return EEPROM_ERR_WRITE;
    }

    /* Program word by word */
    uint32_t        addr = EEPROM_FLASH_ADDR;
    const uint32_t *src  = (const uint32_t *)(const void *)buf;

    for (uint32_t i = 0U; i < EEPROM_BLOCK_WORDS; i++)
    {
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr, (uint64_t)src[i]) != HAL_OK)
        {
            HAL_FLASH_Lock();
            return EEPROM_ERR_WRITE;
        }
        addr += 4U;
    }

    HAL_FLASH_Lock();
    return EEPROM_OK;
}

EepromStatus_t EEPROM_LoadConfig(uint16_t          *min_freq_hz,
                                  uint16_t          *max_freq_hz,
                                  CalibrationPoint_t *points,
                                  uint8_t           *num_points)
{
    /* Flash is memory-mapped — read directly without HAL overhead */
    const EepromBlock_t *blk = (const EepromBlock_t *)EEPROM_FLASH_ADDR;

    if (blk->magic != EEPROM_MAGIC) return EEPROM_ERR_INVALID;

    *min_freq_hz = blk->min_freq_hz;
    *max_freq_hz = blk->max_freq_hz;
    *num_points  = (blk->num_cal_points > CALIBRATION_POINTS)
                   ? CALIBRATION_POINTS : blk->num_cal_points;

    memcpy(points, blk->cal_points, (size_t)(*num_points) * sizeof(CalibrationPoint_t));
    return EEPROM_OK;
}
