/*
 * modbus_registers.h
 * Purpose:      Register address constants and read/write callbacks for the
 *               Modbus register map
 * Dependencies: stdint.h, modbus_rtu.h (exception codes)
 */
#ifndef MODBUS_REGISTERS_H
#define MODBUS_REGISTERS_H

#include <stdint.h>

/* --------------------------------------------------------------------------
 * Holding register addresses (FC03 / FC06 / FC16)
 * -------------------------------------------------------------------------- */
#define REG_MIN_FREQ_HZ          0x0000U  /* uint16, Hz, R/W */
#define REG_MAX_FREQ_HZ          0x0001U  /* uint16, Hz, R/W */
#define REG_OUTPUT_ENABLE        0x0002U  /* uint16, 0/1,  R/W */
#define REG_SWEEP_RATE_KHZ       0x0003U  /* uint16, active ramp rate in kHz, R/W (applied live, auto-persisted) */
#define REG_CAL_BASE             0x0004U  /* 20 regs: [freq0,v0, freq1,v1, ...] */
#define REG_CAL_END              0x0017U  /* last calibration register (inclusive) */
#define REG_DEVICE_ADDRESS       0x0018U  /* uint16, Modbus address 1–247, R/W (persisted) */
#define REG_SWEEP_PAUSE_US       0x0019U  /* uint16, inter-sweep pause in us, 0–WAVEFORM_MAX_PAUSE_US, R/W (persisted) */
#define REG_WAVEFORM_TRIANGLE    0x001AU  /* bool, 0=sawtooth, 1=triangle, R/W (persisted) */
#define REG_DAC_CHANNEL          0x001BU  /* uint16, 1=DAC1 CH1, 2=DAC1 CH2, R/W (volatile) */

/* --------------------------------------------------------------------------
 * Input register addresses (FC04)
 * -------------------------------------------------------------------------- */
#define REG_IN_DEVICE_STATUS     0x0000U  /* bit flags, see below */
#define REG_IN_CURRENT_FREQ      0x0001U  /* Hz */
#define REG_IN_CURRENT_VOLT      0x0002U  /* mV */

/* --------------------------------------------------------------------------
 * Device status bit flags (REG_IN_DEVICE_STATUS)
 * -------------------------------------------------------------------------- */
#define STATUS_BIT_RUNNING       (1U << 0)  /* Sweep active */
#define STATUS_BIT_CONFIG_VALID  (1U << 1)  /* Sweep params valid */
#define STATUS_BIT_FREQ_ERR      (1U << 2)  /* Frequency range error */
#define STATUS_BIT_CAL_INVALID   (1U << 3)  /* No valid calibration */

/* --------------------------------------------------------------------------
 * Register access callbacks — called by modbus_rtu.c dispatcher
 * Return 0 on success, or MB_EX_* on error.
 * -------------------------------------------------------------------------- */

/**
 * @brief  Initialise the register module: restore sweep params, calibration
 *         and device address from Flash-emulated EEPROM.  Falls back to the
 *         compile-time defaults when no valid data is stored.
 *         Call after EEPROM_Init() and Waveform_Init().
 */
void MBReg_Init(void);

/**
 * @brief  Get the active Modbus device address (loaded from EEPROM or default).
 * @return Device address in the range MODBUS_ADDR_MIN..MODBUS_ADDR_MAX
 */
uint8_t MBReg_GetDeviceAddress(void);

/**
 * @brief  Flush any pending configuration changes to Flash-emulated EEPROM.
 *         Call once after a Modbus frame has been dispatched (see
 *         Modbus_Process).  Writes that change sweep params, sweep rate or
 *         calibration set a dirty flag; this coalesces a multi-register (FC16)
 *         write into a single Flash erase/program cycle.  No-op when nothing
 *         changed.
 */
void MBReg_CommitIfDirty(void);

/**
 * @brief  Read a holding register.
 * @param  addr   Register address (0x0000 – 0x001B)
 * @param  value  Output: register value
 * @return 0 on success, MB_EX_ILLEGAL_ADDRESS if address unknown
 */
uint8_t MBReg_ReadHolding(uint16_t addr, uint16_t *value);

/**
 * @brief  Write a holding register.
 *         Side-effects: may call Waveform_* or EEPROM_* functions.
 * @param  addr   Register address
 * @param  value  Value to write
 * @return 0 on success, MB_EX_* on error
 */
uint8_t MBReg_WriteHolding(uint16_t addr, uint16_t value);

/** Toggle the shared Output Enable state from a local control such as B1. */
void MBReg_ToggleOutputEnable(void);

/**
 * @brief  Read an input register.
 * @param  addr   Register address (0x0000 – 0x0002)
 * @param  value  Output: register value
 * @return 0 on success, MB_EX_ILLEGAL_ADDRESS if address unknown
 */
uint8_t MBReg_ReadInput(uint16_t addr, uint16_t *value);

#endif /* MODBUS_REGISTERS_H */
