/*
 * modbus_rtu.h
 * Purpose:      Public API for the Modbus RTU slave core
 * Dependencies: stm32f4xx_hal.h, config.h
 */
#ifndef MODBUS_RTU_H
#define MODBUS_RTU_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

/* Supported function codes */
#define MB_FC_READ_HOLDING      0x03U
#define MB_FC_READ_INPUT        0x04U
#define MB_FC_WRITE_SINGLE      0x06U
#define MB_FC_WRITE_MULTIPLE    0x10U

/* Modbus exception codes */
#define MB_EX_ILLEGAL_FUNCTION  0x01U
#define MB_EX_ILLEGAL_ADDRESS   0x02U
#define MB_EX_ILLEGAL_VALUE     0x03U

/* Broadcast address — execute writes, do not respond */
#define MB_BROADCAST_ADDR       0x00U

/* Maximum ADU frame size (address + PDU + CRC) */
#define MB_MAX_FRAME_SIZE       256U

/**
 * @brief  Initialise the Modbus RTU slave.
 *         Arms the UART for single-byte interrupt reception.
 *         Call from USER CODE BEGIN 2 in main.c.
 * @param  huart  UART handle wired to the Modbus link (USART2 ST-Link VCP /
 *                USART1 RS485, selected via VCP_MODE in config.h)
 * @param  htim6  TIM6 handle used for the 3.5-character inter-frame gap
 */
void Modbus_Init(UART_HandleTypeDef *huart, TIM_HandleTypeDef *htim6);

/**
 * @brief  Read the byte received into the internal single-byte buffer, append
 *         it to the frame buffer, and re-arm UART reception.
 *         Call from HAL_UART_RxCpltCallback.
 */
void Modbus_RxByteCallback(void);

/**
 * @brief  Signal end of frame (3.5-character silence expired).
 *         Call from HAL_TIM_PeriodElapsedCallback when htim == htim6.
 */
void Modbus_FrameTimeoutCallback(void);

/**
 * @brief  Process any pending complete frame.  Non-blocking; call from main loop.
 */
void Modbus_Process(void);

/**
 * @brief  Release the RS485 bus (DE/RE low) after TX completes.
 *         Call from HAL_UART_TxCpltCallback.
 */
void Modbus_TxCompleteCallback(void);

#endif /* MODBUS_RTU_H */
