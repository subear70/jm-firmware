/*
 * config.h
 * Purpose:      Central compile-time configuration for the waveform-node firmware
 * Dependencies: (none — included by all modules)
 */
#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

/* --------------------------------------------------------------------------
 * Modbus
 * -------------------------------------------------------------------------- */
#define MODBUS_DEVICE_ADDRESS    ((uint8_t)1U)       /* 1–247, unique per node */
#define MODBUS_BAUD_RATE         115200U

/* --------------------------------------------------------------------------
 * Waveform sweep limits
 * -------------------------------------------------------------------------- */
#define WAVEFORM_MIN_FREQ_HZ     ((uint16_t)1U)
#define WAVEFORM_MAX_FREQ_HZ     ((uint16_t)1000U)

/* Number of DAC steps per sweep direction (up or down).
 * Full cycle (up + down) = 2 × WAVEFORM_STEPS timer ticks. */
#define WAVEFORM_STEPS           ((uint16_t)100U)

/* --------------------------------------------------------------------------
 * Calibration
 * -------------------------------------------------------------------------- */
#define CALIBRATION_POINTS       ((uint8_t)10U)      /* Freq/Voltage pairs */

/* --------------------------------------------------------------------------
 * DAC
 * -------------------------------------------------------------------------- */
#define DAC_FULL_SCALE_MV        ((uint16_t)3300U)   /* mV at DAC output = 4095 */
#define DAC_RESOLUTION           ((uint16_t)4095U)   /* 12-bit */

/* --------------------------------------------------------------------------
 * TIM2 — waveform step timer
 *
 * TIM2 clock: APB1 × 2 = 45 MHz × 2 = 90 MHz (default CubeMX clock tree,
 *             APB1 prescaler = 4 on 180 MHz SYSCLK).
 *
 * Prescaler = 8999  → tick period = 90 MHz / 9000 = 10 kHz  (0.1 ms / tick)
 * Period    = 99    → ISR period  = 100 ticks × 0.1 ms = 10 ms per step
 *
 * Full sweep duration = 2 × WAVEFORM_STEPS × 10 ms = 2000 ms
 * -------------------------------------------------------------------------- */
#define TIM2_PRESCALER           ((uint32_t)8999U)
#define TIM2_PERIOD              ((uint32_t)99U)

/* --------------------------------------------------------------------------
 * TIM6 — Modbus inter-frame gap timer
 *
 * At 115200 baud, one character (11 bits) = 95.5 µs.
 * 3.5-character silence = 334 µs → use 500 µs for safe margin.
 *
 * TIM6 clock = 90 MHz (same APB1 path as TIM2).
 * Prescaler  = 89   → tick = 90 MHz / 90 = 1 MHz (1 µs / tick)
 * Period     = 499  → 500 µs timeout
 * -------------------------------------------------------------------------- */
#define TIM6_PRESCALER           ((uint32_t)89U)
#define TIM6_PERIOD              ((uint32_t)499U)

/* --------------------------------------------------------------------------
 * EEPROM — emulated in internal Flash (STM32F446RE Sector 7)
 * Sector 7: 128 KB starting at 0x08060000
 * Erase-before-write; writes are infrequent (only on calibration save).
 * -------------------------------------------------------------------------- */
#define EEPROM_FLASH_SECTOR      FLASH_SECTOR_7
#define EEPROM_FLASH_ADDR        ((uint32_t)0x08060000U)
#define EEPROM_MAGIC             ((uint16_t)0xAB12U)

/* --------------------------------------------------------------------------
 * Debug mode — set to 1 to communicate via ST-Link VCP (USART2 → USB on PC)
 * instead of the RS485 transceiver.  RS485 DE/RE toggling is skipped.
 * Set to 0 for production (RS485 half-duplex with direction control).
 *
 * Hardware: in debug mode keep Nucleo solder bridges SB13/SB14 intact and
 * leave the RS485 transceiver disconnected from PA2/PA3.
 * -------------------------------------------------------------------------- */
#define DEBUG_VCP_MODE           1

/* --------------------------------------------------------------------------
 * GPIO — RS485 DE/RE direction control
 * Note: PA4 is the DAC1 CH1 output pin. DE/RE is routed to PA1.
 * -------------------------------------------------------------------------- */
#define RS485_DE_RE_PORT         GPIOA
#define RS485_DE_RE_PIN          GPIO_PIN_1

/* --------------------------------------------------------------------------
 * GPIO — Nucleo onboard LED (LD2)
 * -------------------------------------------------------------------------- */
#define LED_PORT                 GPIOA
#define LED_PIN                  GPIO_PIN_5

#endif /* CONFIG_H */
