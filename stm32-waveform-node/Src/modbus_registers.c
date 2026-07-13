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
/* First-boot default calibration span (matches the default sweep range). */
#define DEFAULT_CAL_MIN_FREQ_HZ  WAVEFORM_DEFAULT_MIN_FREQ_HZ
#define DEFAULT_CAL_MAX_FREQ_HZ  WAVEFORM_DEFAULT_MAX_FREQ_HZ

static uint16_t s_min_freq_hz  = WAVEFORM_DEFAULT_MIN_FREQ_HZ;
static uint16_t s_max_freq_hz  = WAVEFORM_DEFAULT_MAX_FREQ_HZ;
static uint16_t s_output_en    = 0U;

/* Sweep repetition rate (full up+down cycles per second), stored internally in
 * Hz.  Configured via REG_SWEEP_RATE_KHZ (0x0003) in kHz units; applied live,
 * persisted to EEPROM automatically, and restored on boot by MBReg_Init(). */
static uint32_t s_sweep_rate_hz = WAVEFORM_DEFAULT_SWEEP_RATE_HZ;

/* Active Modbus device address (restored from EEPROM by MBReg_Init) */
static uint8_t  s_device_addr  = MODBUS_DEVICE_ADDRESS;

/* 20 registers: [freq0, v0, freq1, v1, ..., freq9, v9] */
static uint16_t s_cal_regs[20] = { 0U };

/* Dirty flags: set when a persistent value changes, flushed to EEPROM once at
 * the end of the Modbus frame by MBReg_CommitIfDirty().  Coalescing avoids one
 * Flash sector erase/program per register during a bulk (FC16) write. */
static uint8_t s_persist_dirty = 0U;  /* sweep params / rate / calibration changed */
static uint8_t s_cal_dirty     = 0U;  /* calibration table needs re-applying */

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

/** Persist the current staging config (sweep params, calibration, device
 *  address) to Flash-emulated EEPROM. */
static EepromStatus_t persist_config(void)
{
    CalibrationPoint_t pts[CALIBRATION_POINTS];
    for (uint8_t i = 0U; i < CALIBRATION_POINTS; i++)
    {
        pts[i].freq_hz    = s_cal_regs[i * 2U];
        pts[i].voltage_mv = s_cal_regs[i * 2U + 1U];
    }
    return EEPROM_SaveConfig(s_min_freq_hz, s_max_freq_hz, s_sweep_rate_hz,
                             pts, CALIBRATION_POINTS, s_device_addr);
}

/** Load a linear default calibration into the staging registers:
 *  frequency ramps DEFAULT_CAL_MIN_FREQ_HZ .. DEFAULT_CAL_MAX_FREQ_HZ and
 *  voltage ramps 0 .. DAC full-scale, evenly across CALIBRATION_POINTS. */
static void load_default_calibration(void)
{
    const uint32_t freq_span = (uint32_t)(DEFAULT_CAL_MAX_FREQ_HZ - DEFAULT_CAL_MIN_FREQ_HZ);
    const uint32_t volt_span = (uint32_t)DAC_FULL_SCALE_MV;
    const uint32_t divisor   = (uint32_t)(CALIBRATION_POINTS - 1U);

    for (uint8_t i = 0U; i < CALIBRATION_POINTS; i++)
    {
        uint16_t freq = (uint16_t)((uint32_t)DEFAULT_CAL_MIN_FREQ_HZ
                                   + (freq_span * (uint32_t)i) / divisor);
        uint16_t volt = (uint16_t)((volt_span * (uint32_t)i) / divisor);
        s_cal_regs[i * 2U]      = freq;
        s_cal_regs[i * 2U + 1U] = volt;
    }
}

/* --------------------------------------------------------------------------
 * Public API
 * -------------------------------------------------------------------------- */

void MBReg_Init(void)
{
    uint16_t           min_f    = WAVEFORM_DEFAULT_MIN_FREQ_HZ;
    uint16_t           max_f    = WAVEFORM_DEFAULT_MAX_FREQ_HZ;
    uint32_t           rate     = WAVEFORM_DEFAULT_SWEEP_RATE_HZ;
    CalibrationPoint_t pts[CALIBRATION_POINTS];
    uint8_t            n_pts    = 0U;
    uint8_t            dev_addr = MODBUS_DEVICE_ADDRESS;

    if (EEPROM_LoadConfig(&min_f, &max_f, &rate, pts, &n_pts, &dev_addr) != EEPROM_OK)
    {
        /* No valid data — first boot, erased sector, corrupt magic, or a
           torn write.  Seed a sensible default configuration: a linear
           1..10 Hz / 0..DAC-max calibration plus the compile-time sweep
           defaults, apply it, then persist so subsequent boots load a valid
           block. */
        load_default_calibration();
        apply_calibration();               /* pushes calibration + sweep params */
        Waveform_SetSweepRate(s_sweep_rate_hz);
        (void)persist_config();
        return;
    }

    /* Guard against a corrupt/out-of-range stored address */
    if (dev_addr < MODBUS_ADDR_MIN || dev_addr > MODBUS_ADDR_MAX)
        dev_addr = MODBUS_DEVICE_ADDRESS;

    /* Guard against a corrupt/out-of-range stored sweep rate */
    if (rate < WAVEFORM_MIN_SWEEP_RATE_HZ || rate > WAVEFORM_MAX_SWEEP_RATE_HZ)
        rate = WAVEFORM_DEFAULT_SWEEP_RATE_HZ;

    s_device_addr   = dev_addr;
    s_min_freq_hz   = min_f;
    s_max_freq_hz   = max_f;
    s_sweep_rate_hz = rate;

    for (uint8_t i = 0U; (i < n_pts) && (i < CALIBRATION_POINTS); i++)
    {
        s_cal_regs[i * 2U]      = pts[i].freq_hz;
        s_cal_regs[i * 2U + 1U] = pts[i].voltage_mv;
    }

    Waveform_SetCalibrationData(pts, n_pts);
    Waveform_SetSweepParams(min_f, max_f);
    Waveform_SetSweepRate(rate);
    /* Output stays disabled until master sends Output Enable command */
}

uint8_t MBReg_GetDeviceAddress(void)
{
    return s_device_addr;
}

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
    else if (addr == REG_SWEEP_RATE_KHZ)
    {
        *value = (uint16_t)(s_sweep_rate_hz / 1000U);  /* Hz -> kHz */
    }
    else if (addr >= REG_CAL_BASE && addr <= REG_CAL_END)
    {
        *value = s_cal_regs[addr - REG_CAL_BASE];
    }
    else if (addr == REG_DEVICE_ADDRESS)
    {
        *value = s_device_addr;
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
        /* No range/ordering limits — any frequency value is accepted */
        s_min_freq_hz = value;
        apply_sweep_params();
        s_persist_dirty = 1U;
    }
    else if (addr == REG_MAX_FREQ_HZ)
    {
        /* No range/ordering limits — any frequency value is accepted */
        s_max_freq_hz = value;
        apply_sweep_params();
        s_persist_dirty = 1U;
    }
    else if (addr == REG_OUTPUT_ENABLE)
    {
        s_output_en = (value != 0U) ? 1U : 0U;
        if (s_output_en)
            Waveform_Start();
        else
            Waveform_Stop();
    }
    else if (addr == REG_SWEEP_RATE_KHZ)
    {
        uint32_t rate = (uint32_t)value * 1000U;  /* kHz -> Hz */
        if (rate < WAVEFORM_MIN_SWEEP_RATE_HZ || rate > WAVEFORM_MAX_SWEEP_RATE_HZ)
            return MB_EX_ILLEGAL_VALUE;
        s_sweep_rate_hz = rate;
        Waveform_SetSweepRate(rate);
        /* Applied live; persisted to EEPROM at end-of-frame */
        s_persist_dirty = 1U;
    }
    else if (addr >= REG_CAL_BASE && addr <= REG_CAL_END)
    {
        s_cal_regs[addr - REG_CAL_BASE] = value;
        /* Applied and persisted at end-of-frame by MBReg_CommitIfDirty() */
        s_cal_dirty     = 1U;
        s_persist_dirty = 1U;
    }
    else if (addr == REG_DEVICE_ADDRESS)
    {
        if (value < MODBUS_ADDR_MIN || value > MODBUS_ADDR_MAX)
            return MB_EX_ILLEGAL_VALUE;

        s_device_addr = (uint8_t)value;

        /* Persist immediately so the new address survives a reset. The
         * response to this request is still sent from the old address
         * (the dispatcher echoes the addressed byte); subsequent frames
         * must target the new address. */
        if (persist_config() != EEPROM_OK)
            return MB_EX_ILLEGAL_VALUE;
    }
    else
    {
        return MB_EX_ILLEGAL_ADDRESS;
    }
    return 0U;
}

void MBReg_CommitIfDirty(void)
{
    if (s_cal_dirty)
    {
        /* Push staged calibration into the waveform module (also re-applies the
         * current sweep so new voltages take effect). */
        apply_calibration();
        s_cal_dirty = 0U;
    }
    if (s_persist_dirty)
    {
        (void)persist_config();
        s_persist_dirty = 0U;
    }
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
