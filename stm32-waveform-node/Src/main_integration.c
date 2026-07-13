/*
 * main_integration.c
 * Purpose:  USER CODE sections to insert into CubeMX-generated main.c
 *           for the stm32-waveform-node project.
 *
 * Instructions:
 *   Copy each block into the matching USER CODE BEGIN / END section in main.c.
 *   Do NOT paste this file verbatim — it is a reference document only.
 */

/* ============================================================
 * USER CODE BEGIN Includes
 * ============================================================ */
#include "config.h"
#include "modbus_rtu.h"
#include "modbus_registers.h"
#include "waveform.h"
#include "eeprom.h"
/* ============================================================ */


/* ============================================================
 * USER CODE BEGIN PV   (Private Variables)
 * ============================================================ */
/* (Modbus owns its single-byte UART RX buffer internally.) */
/* ============================================================ */


/* ============================================================
 * USER CODE BEGIN 2   (after MX_*_Init calls, before while loop)
 *
 * Integration flow:
 *   UART2 RX ISR  → Modbus_RxByteCallback() → TIM6 gap timer
 *   TIM6 ISR      → Modbus_FrameTimeoutCallback()
 *   main loop     → Modbus_Process() → MBReg_WriteHolding() → Waveform_*()
 *   TIM2 TRGO     → DAC conversion trigger → DMA1_Stream5 → DAC output
 *                   (hardware-driven circular sweep; no per-step ISR)
 * ============================================================ */

    /* Initialise modules */
    Modbus_Init(&huart_modbus, &htim6);
    Waveform_Init(&hdac, &htim2);
    EEPROM_Init();

    /* Restore sweep params, calibration and device address from the
       Flash-emulated EEPROM. Falls back to compile-time defaults when no
       valid data is stored. Output stays disabled until the master enables it. */
    MBReg_Init();

    /* Blink onboard LED 3 times to confirm successful startup */
    for (uint8_t i = 0U; i < 3U; i++)
    {
        HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_SET);
        HAL_Delay(150U);
        HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET);
        HAL_Delay(150U);
    }

/* ============================================================ */


/* ============================================================
 * USER CODE BEGIN WHILE   (inside the while(1) main loop)
 * ============================================================ */

        /* Process any complete Modbus frame received since last iteration */
        Modbus_Process();

/* ============================================================ */


/* ============================================================
 * HAL_UART_RxCpltCallback   (weak override — add to main.c or stm32f4xx_it.c)
 * ============================================================ */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == MODBUS_UART_INSTANCE)
    {
        /* Feed received byte into Modbus frame buffer; re-arms UART internally */
        Modbus_RxByteCallback();
    }
}
/* ============================================================ */


/* ============================================================
 * HAL_UART_TxCpltCallback   (weak override)
 * ============================================================ */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == MODBUS_UART_INSTANCE)
    {
        /* Release RS485 bus (DE/RE LOW) */
        Modbus_TxCompleteCallback();
    }
}
/* ============================================================ */


/* ============================================================
 * HAL_TIM_PeriodElapsedCallback   (weak override)
 * Delegates to the correct module — no logic in this callback.
 * ============================================================ */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2)
    {
        /* TIM2 update events trigger the DAC-DMA sweep in hardware;
           no ISR work is required here. */
    }
    else if (htim->Instance == TIM6)
    {
        /* 3.5-character silence expired — frame complete */
        Modbus_FrameTimeoutCallback();
    }
}
/* ============================================================ */
