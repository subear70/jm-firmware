/*
 * modbus_registers.c
 * Purpose:      Modbus register map handlers — bridges Modbus FC dispatcher
 *               with the Waveform and EEPROM modules
 * Dependencies: modbus_registers.h, modbus_rtu.h, waveform.h, eeprom.h, config.h
 */
#include "modbus_registers.h"
#include "modbus_rtu.h"
#include "waveform.h"
#include "eeprom.h"
#include "config.h"
#include <string.h>

/* --------------------------------------------------------------------------
 * Staging registers — hold values written by Modbus before apply/save
 * -------------------------------------------------------------------------- */
static uint16_t s_min_freq_hz  = WAVEFORM_MIN_FREQ_HZ;
static uint16_t s_max_freq_hz  = WAVEFORM_MAX_FREQ_HZ;
static uint16_t s_output_en    = 0U;

/* 20 registers: [freq0, v0, freq1, v1, ..., freq9, v9] */
static uint16_t s_cal_regs[20] = { 0U };

/* --------------------------------------------------------------------------
 * Helpers
 * -------------------------------------------------------------------------- */

/** Push the current staging values into the waveform module. */
static void apply_sweep_params(void)
{
    Waveform_Stop();
    Waveform_SetSweepParams(s_min_freq_hz, s_max_freq_hz);
    if (s_output_en)
        Waveform_Start();
}

/** Convert the staging calibration registers to CalibrationPoint_t array
 *  and push into the waveform module. */
static void apply_calibration(void)
{
    CalibrationPoint_t pts[CALIBRATION_POINTS];
    for (uint8_t i = 0U; i < CALIBRATION_POINTS; i++)
    {
        pts[i].freq_hz    = s_cal_regs[i * 2U];
        pts[i].voltage_mv = s_cal_regs[i * 2U + 1U];
    }
    Waveform_SetCalibrationData(pts, CALIBRATION_POINTS);
    /* Re-apply sweep so new calibration voltages are calculated */
    apply_sweep_params();
}

/* --------------------------------------------------------------------------
 * Public register access callbacks
 * -------------------------------------------------------------------------- */

uint8_t MBReg_ReadHolding(uint16_t addr, uint16_t *value)
{
    if (addr == REG_MIN_FREQ_HZ)
    {
        *value = s_min_freq_hz;
    }
    else if (addr == REG_MAX_FREQ_HZ)
    {
        *value = s_max_freq_hz;
    }
    else if (addr == REG_OUTPUT_ENABLE)
    {
        *value = s_output_en;
    }
    else if (addr == REG_SAVE_CALIBRATION)
    {
        *value = 0U;  /* Write-only trigger — always reads 0 */
    }
    else if (addr >= REG_CAL_BASE && addr <= REG_CAL_END)
    {
        *value = s_cal_regs[addr - REG_CAL_BASE];
    }
    else
    {
        return MB_EX_ILLEGAL_ADDRESS;
    }
    return 0U;
}

uint8_t MBReg_WriteHolding(uint16_t addr, uint16_t value)
{
    if (addr == REG_MIN_FREQ_HZ)
    {
        if (value < WAVEFORM_MIN_FREQ_HZ || value >= s_max_freq_hz)
            return MB_EX_ILLEGAL_VALUE;
        s_min_freq_hz = value;
        apply_sweep_params();
    }
    else if (addr == REG_MAX_FREQ_HZ)
    {
        if (value > WAVEFORM_MAX_FREQ_HZ || value <= s_min_freq_hz)
            return MB_EX_ILLEGAL_VALUE;
        s_max_freq_hz = value;
        apply_sweep_params();
    }
    else if (addr == REG_OUTPUT_ENABLE)
    {
        s_output_en = (value != 0U) ? 1U : 0U;
        if (s_output_en)
            Waveform_Start();
        else
            Waveform_Stop();
    }
    else if (addr == REG_SAVE_CALIBRATION)
    {
        /* Any write to this register triggers EEPROM save */
        apply_calibration();

        CalibrationPoint_t pts[CALIBRATION_POINTS];
        for (uint8_t i = 0U; i < CALIBRATION_POINTS; i++)
        {
            pts[i].freq_hz    = s_cal_regs[i * 2U];
            pts[i].voltage_mv = s_cal_regs[i * 2U + 1U];
        }
        EepromStatus_t st = EEPROM_SaveConfig(s_min_freq_hz, s_max_freq_hz,
                                              pts, CALIBRATION_POINTS);
        if (st != EEPROM_OK)
            return MB_EX_ILLEGAL_VALUE;
    }
    else if (addr >= REG_CAL_BASE && addr <= REG_CAL_END)
    {
        s_cal_regs[addr - REG_CAL_BASE] = value;
        /* Calibration is not live-applied until REG_SAVE_CALIBRATION is written */
    }
    else
    {
        return MB_EX_ILLEGAL_ADDRESS;
    }
    return 0U;
}

uint8_t MBReg_ReadInput(uint16_t addr, uint16_t *value)
{
    if (addr == REG_IN_DEVICE_STATUS)
    {
        WaveformStatus_t st = Waveform_GetStatus();
        uint16_t flags = 0U;
        if (st & WAVEFORM_RUNNING)   flags |= STATUS_BIT_RUNNING;
        if (!(st & WAVEFORM_ERR_RANGE)) flags |= STATUS_BIT_CONFIG_VALID;
        if (st & WAVEFORM_ERR_RANGE) flags |= STATUS_BIT_FREQ_ERR;
        if (st & WAVEFORM_ERR_CAL)   flags |= STATUS_BIT_CAL_INVALID;
        *value = flags;
    }
    else if (addr == REG_IN_CURRENT_FREQ)
    {
        *value = Waveform_GetCurrentFrequency_Hz();
    }
    else if (addr == REG_IN_CURRENT_VOLT)
    {
        *value = Waveform_GetCurrentVoltage_mV();
    }
    else
    {
        return MB_EX_ILLEGAL_ADDRESS;
    }
    return 0U;
}
