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
/* Default device address used on first boot or when the EEPROM holds no
 * valid data.  The active address is persisted in Flash-emulated EEPROM and
 * is runtime-configurable via the device-address holding register. */
#define MODBUS_DEVICE_ADDRESS    ((uint8_t)1U)       /* 1–247, unique per node */
#define MODBUS_ADDR_MIN          ((uint8_t)1U)        /* lowest valid unicast address */
#define MODBUS_ADDR_MAX          ((uint8_t)247U)      /* highest valid unicast address */
#define MODBUS_BAUD_RATE         115200U

/* --------------------------------------------------------------------------
 * Waveform output-frequency limits (VCO tuning range, set via DAC voltage)
 * -------------------------------------------------------------------------- */
#define WAVEFORM_MIN_FREQ_HZ     ((uint16_t)1U)
#define WAVEFORM_MAX_FREQ_HZ     ((uint16_t)1000U)

/* First-boot default sweep range (used when EEPROM holds no valid data).
 * Must lie within WAVEFORM_MIN_FREQ_HZ .. WAVEFORM_MAX_FREQ_HZ. */
#define WAVEFORM_DEFAULT_MIN_FREQ_HZ  ((uint16_t)1U)
#define WAVEFORM_DEFAULT_MAX_FREQ_HZ  ((uint16_t)10U)

/* Dynamic DAC sample count per sweep ramp.
 * The number of DAC samples used for each sweep is chosen at runtime from the
 * sweep rate so the DAC sample rate never exceeds DAC_MAX_SAMPLE_RATE_HZ:
 *
 *     samples = clamp(DAC_MAX_SAMPLE_RATE_HZ / sweep_rate,
 *                     WAVEFORM_MIN_SAMPLES, WAVEFORM_MAX_SAMPLES)
 *
 * Low sweep rates use more samples (finer ramp); high rates use fewer (coarser)
 * so the same DAC reaches much higher sweep rates.  Each sample's DAC voltage is
 * derived from the calibration table (see waveform.c build_dac_buffer), so the
 * swept frequency is linear even when the freq-to-voltage curve is not.
 *
 * WAVEFORM_MAX_SAMPLES sizes the statically allocated DMA buffer and caps the
 * point count at 100 steps per sweep. */
#define WAVEFORM_MIN_SAMPLES     ((uint16_t)2U)     /* coarsest ramp (2 levels)   */
#define WAVEFORM_MAX_SAMPLES     ((uint16_t)100U)   /* finest ramp / DMA buf size */

/* --------------------------------------------------------------------------
 * Active waveform sweep-ramp rate limits (excluding inter-sweep pause)
 * -------------------------------------------------------------------------- */
#define WAVEFORM_MIN_SWEEP_RATE_HZ     ((uint32_t)1U)
#define WAVEFORM_MAX_SWEEP_RATE_HZ     ((uint32_t)500000U)   /* 500 k sweeps/s (DAC_MAX_SAMPLE_RATE_HZ / WAVEFORM_MIN_SAMPLES) */
#define WAVEFORM_DEFAULT_SWEEP_RATE_HZ ((uint32_t)10000U)    /* 10 k sweeps/s */

/* Maximum configurable hold time between sweeps.  At the 1 MSPS DAC limit,
 * this reserves at most 10,000 additional half-word samples (20 KB) in DMA. */
#define WAVEFORM_MAX_PAUSE_US          10000U
#define WAVEFORM_MAX_PAUSE_SAMPLES     WAVEFORM_MAX_PAUSE_US

/* --------------------------------------------------------------------------
 * Calibration
 * -------------------------------------------------------------------------- */
#define CALIBRATION_POINTS       ((uint8_t)10U)      /* Freq/Voltage pairs */

/* --------------------------------------------------------------------------
 * DAC
 * -------------------------------------------------------------------------- */
#define DAC_FULL_SCALE_MV        ((uint16_t)3300U)   /* mV at DAC output = 4095 */
#define DAC_RESOLUTION           ((uint16_t)4095U)   /* 12-bit */
#define DAC_MAX_SAMPLE_RATE_HZ   ((uint32_t)1000000U) /* ~1 MSPS DAC update ceiling */
#define WAVEFORM_MIN_TRIANGLE_SAMPLES ((uint16_t)3U)
#define WAVEFORM_MAX_TRIANGLE_SWEEP_RATE_HZ \
  ((uint32_t)(DAC_MAX_SAMPLE_RATE_HZ / WAVEFORM_MIN_TRIANGLE_SAMPLES))

/* --------------------------------------------------------------------------
 * TIM2 — DAC sweep trigger timer (drives DAC-DMA, one sample per update event)
 *
 * TIM2 clock: APB1 × 2 = 45 MHz × 2 = 90 MHz (default CubeMX clock tree,
 *             APB1 prescaler = 4 on 180 MHz SYSCLK).
 *
 * TIM2 runs with prescaler = 0 (counts at the full 90 MHz kernel clock) and
 * generates a TRGO update event that triggers each DAC conversion.  The
 * auto-reload value (ARR) selects the DAC sample rate and hence the sweep
 * repetition rate:
 *
 *   f_sample = TIM2_CLK_HZ / (ARR + 1)
 *   sweep_rate = f_sample / N          (N = active DAC sample count)
 *   => ARR = TIM2_CLK_HZ / (sweep_rate * N) - 1
 *
 * N is chosen at runtime from the sweep rate (see WAVEFORM_MIN/MAX_SAMPLES);
 * ARR is recomputed at runtime by Waveform_SetSweepRate(); the value below is
 * only the power-on default.
 * -------------------------------------------------------------------------- */
#define TIM2_CLK_HZ              ((uint32_t)90000000U)
#define TIM2_PRESCALER           ((uint32_t)0U)

/* ARR for a given active ramp rate and DAC sample count N. */
#define WAVEFORM_TIM2_ARR(rate, samples)  \
    ((uint32_t)(TIM2_CLK_HZ / ((uint32_t)(rate) * (uint32_t)(samples))) - 1U)

/* Power-on default ARR (overwritten at runtime). Uses the sample count the
 * default rate resolves to: DAC_MAX_SAMPLE_RATE_HZ / default_rate. */
#define WAVEFORM_DEFAULT_SAMPLES ((uint16_t)(DAC_MAX_SAMPLE_RATE_HZ / WAVEFORM_DEFAULT_SWEEP_RATE_HZ))
#define TIM2_PERIOD              WAVEFORM_TIM2_ARR(WAVEFORM_DEFAULT_SWEEP_RATE_HZ, WAVEFORM_DEFAULT_SAMPLES)

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
 * Erase-before-write; persistent setting changes are coalesced per Modbus frame.
 * -------------------------------------------------------------------------- */
#define EEPROM_FLASH_SECTOR      FLASH_SECTOR_7
#define EEPROM_FLASH_ADDR        ((uint32_t)0x08060000U)
#define EEPROM_MAGIC             ((uint16_t)0xAB13U)

/* --------------------------------------------------------------------------
 * VCP mode — set to 1 to communicate via the onboard ST-Link VCP
 * (USART2 → USB on PC).  RS485 DE/RE toggling is skipped.
 * Set to 0 to use USART1 (PA9/PA10) with the RS485 transceiver and
 * half-duplex direction control.
 *
 * Hardware: in VCP mode keep Nucleo solder bridges SB13/SB14 intact and
 * leave the RS485 transceiver disconnected from PA2/PA3.
 * -------------------------------------------------------------------------- */
#define VCP_MODE                 1

/* --------------------------------------------------------------------------
 * Modbus transport UART selection
 *
 * The Modbus RTU link runs on either USART, selected at compile time by
 * VCP_MODE so a single switch changes both the UART and the RS485 DE/RE
 * handling:
 *
 *   VCP_MODE = 1  → USART2 on PA2 (TX) / PA3 (RX)   — onboard ST-Link VCP
 *   VCP_MODE = 0  → USART1 on PA9 (TX) / PA10 (RX)  — RS485 transceiver
 *
 * All UART-specific code (handle, init, MSP, IRQ, callbacks) references the
 * macros below, so flipping VCP_MODE is enough to move the link.
 * -------------------------------------------------------------------------- */
#if VCP_MODE
  #define MODBUS_UART_INSTANCE      USART2
  #define MODBUS_UART_IRQn          USART2_IRQn
  #define MODBUS_UART_IRQHandler    USART2_IRQHandler
  #define MODBUS_UART_CLK_ENABLE()  __HAL_RCC_USART2_CLK_ENABLE()
  #define MODBUS_UART_CLK_DISABLE() __HAL_RCC_USART2_CLK_DISABLE()
  #define MODBUS_UART_GPIO_PORT     GPIOA
  #define MODBUS_UART_TX_PIN        GPIO_PIN_2
  #define MODBUS_UART_RX_PIN        GPIO_PIN_3
  #define MODBUS_UART_GPIO_AF       GPIO_AF7_USART2
#else
  #define MODBUS_UART_INSTANCE      USART1
  #define MODBUS_UART_IRQn          USART1_IRQn
  #define MODBUS_UART_IRQHandler    USART1_IRQHandler
  #define MODBUS_UART_CLK_ENABLE()  __HAL_RCC_USART1_CLK_ENABLE()
  #define MODBUS_UART_CLK_DISABLE() __HAL_RCC_USART1_CLK_DISABLE()
  #define MODBUS_UART_GPIO_PORT     GPIOA
  #define MODBUS_UART_TX_PIN        GPIO_PIN_9
  #define MODBUS_UART_RX_PIN        GPIO_PIN_10
  #define MODBUS_UART_GPIO_AF       GPIO_AF7_USART1
#endif

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
