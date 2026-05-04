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
/* Single-byte buffer used to receive Modbus data via UART interrupt */
static uint8_t s_uart_rx_byte;
/* ============================================================ */


/* ============================================================
 * USER CODE BEGIN 2   (after MX_*_Init calls, before while loop)
 *
 * Integration flow:
 *   UART2 RX ISR  → Modbus_RxByteCallback() → TIM6 gap timer
 *   TIM6 ISR      → Modbus_FrameTimeoutCallback()
 *   main loop     → Modbus_Process() → MBReg_WriteHolding() → Waveform_*()
 *   TIM2 ISR      → Waveform_TIM2_Callback() → DAC output
 * ============================================================ */

    /* Initialise modules */
    Modbus_Init(&huart2, &htim6);
    Waveform_Init(&hdac, &htim2);
    EEPROM_Init();

    /* Restore configuration from EEPROM (non-blocking; uses HAL_Delay) */
    {
        uint16_t           min_f = WAVEFORM_MIN_FREQ_HZ;
        uint16_t           max_f = WAVEFORM_MAX_FREQ_HZ;
        CalibrationPoint_t pts[CALIBRATION_POINTS];
        uint8_t            n_pts = 0U;

        if (EEPROM_LoadConfig(&min_f, &max_f, pts, &n_pts) == EEPROM_OK)
        {
            Waveform_SetCalibrationData(pts, n_pts);
            Waveform_SetSweepParams(min_f, max_f);
            /* Output stays disabled until master sends Output Enable command */
        }
    }

    /* Blink onboard LED 3 times to confirm successful startup */
    for (uint8_t i = 0U; i < 3U; i++)
    {
        HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_SET);
        HAL_Delay(150U);
        HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET);
        HAL_Delay(150U);
    }

    /* Arm first UART RX byte */
    HAL_UART_Receive_IT(&huart2, &s_uart_rx_byte, 1U);

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
    if (huart->Instance == USART2)
    {
        /* Feed byte into Modbus frame buffer; re-arms UART internally */
        Modbus_RxByteCallback(s_uart_rx_byte);
    }
}
/* ============================================================ */


/* ============================================================
 * HAL_UART_TxCpltCallback   (weak override)
 * ============================================================ */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2)
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
        /* Advance waveform DAC one step */
        Waveform_TIM2_Callback();
    }
    else if (htim->Instance == TIM6)
    {
        /* 3.5-character silence expired — frame complete */
        Modbus_FrameTimeoutCallback();
    }
}
/* ============================================================ */
