/*
 * modbus_rtu.c
 * Purpose:      Modbus RTU slave — frame reception, CRC validation, FC dispatch,
 *               RS485 direction control
 * Dependencies: modbus_rtu.h, modbus_registers.h, config.h
 */
#include "modbus_rtu.h"
#include "modbus_registers.h"
#include "config.h"
#include <string.h>

/* --------------------------------------------------------------------------
 * CRC16 lookup table — Modbus polynomial 0xA001 (reflected 0x8005)
 * -------------------------------------------------------------------------- */
static const uint16_t s_crc_table[256] = {
    0x0000U, 0xC0C1U, 0xC181U, 0x0140U, 0xC301U, 0x03C0U, 0x0280U, 0xC241U,
    0xC601U, 0x06C0U, 0x0780U, 0xC741U, 0x0500U, 0xC5C1U, 0xC481U, 0x0440U,
    0xCC01U, 0x0CC0U, 0x0D80U, 0xCD41U, 0x0F00U, 0xCFC1U, 0xCE81U, 0x0E40U,
    0x0A00U, 0xCAC1U, 0xCB81U, 0x0B40U, 0xC901U, 0x09C0U, 0x0880U, 0xC841U,
    0xD801U, 0x18C0U, 0x1980U, 0xD941U, 0x1B00U, 0xDBC1U, 0xDA81U, 0x1A40U,
    0x1E00U, 0xDEC1U, 0xDF81U, 0x1F40U, 0xDD01U, 0x1DC0U, 0x1C80U, 0xDC41U,
    0x1400U, 0xD4C1U, 0xD581U, 0x1540U, 0xD701U, 0x17C0U, 0x1680U, 0xD641U,
    0xD201U, 0x12C0U, 0x1380U, 0xD341U, 0x1100U, 0xD1C1U, 0xD081U, 0x1040U,
    0xF001U, 0x30C0U, 0x3180U, 0xF141U, 0x3300U, 0xF3C1U, 0xF281U, 0x3240U,
    0x3600U, 0xF6C1U, 0xF781U, 0x3740U, 0xF501U, 0x35C0U, 0x3480U, 0xF441U,
    0x3C00U, 0xFCC1U, 0xFD81U, 0x3D40U, 0xFF01U, 0x3FC0U, 0x3E80U, 0xFE41U,
    0xFA01U, 0x3AC0U, 0x3B80U, 0xFB41U, 0x3900U, 0xF9C1U, 0xF881U, 0x3840U,
    0x2800U, 0xE8C1U, 0xE981U, 0x2940U, 0xEB01U, 0x2BC0U, 0x2A80U, 0xEA41U,
    0xEE01U, 0x2EC0U, 0x2F80U, 0xEF41U, 0x2D00U, 0xEDC1U, 0xEC81U, 0x2C40U,
    0xE401U, 0x24C0U, 0x2580U, 0xE541U, 0x2700U, 0xE7C1U, 0xE681U, 0x2640U,
    0x2200U, 0xE2C1U, 0xE381U, 0x2340U, 0xE101U, 0x21C0U, 0x2080U, 0xE041U,
    0xA001U, 0x60C0U, 0x6180U, 0xA141U, 0x6300U, 0xA3C1U, 0xA281U, 0x6240U,
    0x6600U, 0xA6C1U, 0xA781U, 0x6740U, 0xA501U, 0x65C0U, 0x6480U, 0xA441U,
    0x6C00U, 0xACC1U, 0xAD81U, 0x6D40U, 0xAF01U, 0x6FC0U, 0x6E80U, 0xAE41U,
    0xAA01U, 0x6AC0U, 0x6B80U, 0xAB41U, 0x6900U, 0xA9C1U, 0xA881U, 0x6840U,
    0x7800U, 0xB8C1U, 0xB981U, 0x7940U, 0xBB01U, 0x7BC0U, 0x7A80U, 0xBA41U,
    0xBE01U, 0x7EC0U, 0x7F80U, 0xBF41U, 0x7D00U, 0xBDC1U, 0xBC81U, 0x7C40U,
    0xB401U, 0x74C0U, 0x7580U, 0xB541U, 0x7700U, 0xB7C1U, 0xB681U, 0x7640U,
    0x7200U, 0xB2C1U, 0xB381U, 0x7340U, 0xB101U, 0x71C0U, 0x7080U, 0xB041U,
    0x5000U, 0x90C1U, 0x9181U, 0x5140U, 0x9301U, 0x53C0U, 0x5280U, 0x9241U,
    0x9601U, 0x56C0U, 0x5780U, 0x9741U, 0x5500U, 0x95C1U, 0x9481U, 0x5440U,
    0x9C01U, 0x5CC0U, 0x5D80U, 0x9D41U, 0x5F00U, 0x9FC1U, 0x9E81U, 0x5E40U,
    0x5A00U, 0x9AC1U, 0x9B81U, 0x5B40U, 0x9901U, 0x59C0U, 0x5880U, 0x9841U,
    0x8801U, 0x48C0U, 0x4980U, 0x8941U, 0x4B00U, 0x8BC1U, 0x8A81U, 0x4A40U,
    0x4E00U, 0x8EC1U, 0x8F81U, 0x4F40U, 0x8D01U, 0x4DC0U, 0x4C80U, 0x8C41U,
    0x4400U, 0x84C1U, 0x8581U, 0x4540U, 0x8701U, 0x47C0U, 0x4680U, 0x8641U,
    0x8201U, 0x42C0U, 0x4380U, 0x8341U, 0x4100U, 0x81C1U, 0x8081U, 0x4040U,
};

/* --------------------------------------------------------------------------
 * Module state
 * -------------------------------------------------------------------------- */
static UART_HandleTypeDef *s_huart = NULL;
static TIM_HandleTypeDef  *s_htim6 = NULL;

static uint8_t           s_rx_buf[MB_MAX_FRAME_SIZE];
static volatile uint16_t s_rx_len      = 0U;
static volatile uint8_t  s_frame_ready = 0U;
static uint8_t           s_rx_byte;          /* single-byte ISR landing buffer */

static uint8_t           s_tx_buf[MB_MAX_FRAME_SIZE];

/* --------------------------------------------------------------------------
 * Internal helpers
 * -------------------------------------------------------------------------- */

/**
 * @brief  Compute Modbus CRC16 over len bytes of buf.
 */
static uint16_t crc16_calc(const uint8_t *buf, uint16_t len)
{
    uint16_t crc = 0xFFFFU;
    while (len--)
    {
        uint8_t idx = (uint8_t)(crc ^ *buf++);
        crc = (uint16_t)((crc >> 8U) ^ s_crc_table[idx]);
    }
    return crc;
}

/** Assert RS485 transmit direction (DE/RE HIGH). No-op in VCP debug mode. */
static inline void rs485_tx_enable(void)
{
#if !DEBUG_VCP_MODE
    HAL_GPIO_WritePin(RS485_DE_RE_PORT, RS485_DE_RE_PIN, GPIO_PIN_SET);
#endif
}

/** Release RS485 bus for reception (DE/RE LOW). No-op in VCP debug mode. */
static inline void rs485_rx_enable(void)
{
#if !DEBUG_VCP_MODE
    HAL_GPIO_WritePin(RS485_DE_RE_PORT, RS485_DE_RE_PIN, GPIO_PIN_RESET);
#endif
}

/**
 * @brief  Append CRC to tx_buf[0..len-1] and transmit via UART interrupt.
 *         Asserts DE/RE before transmit; DE/RE is released in TxCpltCallback.
 */
static void send_response(uint8_t *buf, uint16_t len)
{
    uint16_t crc = crc16_calc(buf, len);
    buf[len]     = (uint8_t)(crc & 0xFFU);
    buf[len + 1U] = (uint8_t)(crc >> 8U);
    rs485_tx_enable();
    HAL_UART_Transmit_IT(s_huart, buf, len + 2U);
}

/**
 * @brief  Build and send a Modbus exception response.
 * @param  device_addr  Slave address byte
 * @param  fc           Original function code
 * @param  ex_code      Exception code (MB_EX_*)
 */
static void send_exception(uint8_t device_addr, uint8_t fc, uint8_t ex_code)
{
    s_tx_buf[0] = device_addr;
    s_tx_buf[1] = fc | 0x80U;
    s_tx_buf[2] = ex_code;
    send_response(s_tx_buf, 3U);
}

/* --------------------------------------------------------------------------
 * FC handlers (called only for unicast frames with valid CRC)
 * -------------------------------------------------------------------------- */

/** FC03 — Read Holding Registers */
static void handle_fc03(uint8_t addr, const uint8_t *pdu, uint16_t pdu_len)
{
    if (pdu_len < 5U) { send_exception(addr, MB_FC_READ_HOLDING, MB_EX_ILLEGAL_VALUE); return; }

    uint16_t start = (uint16_t)(((uint16_t)pdu[1] << 8U) | pdu[2]);
    uint16_t count = (uint16_t)(((uint16_t)pdu[3] << 8U) | pdu[4]);

    if (count == 0U || count > 125U)
    {
        send_exception(addr, MB_FC_READ_HOLDING, MB_EX_ILLEGAL_VALUE);
        return;
    }

    uint8_t  byte_count = (uint8_t)(count * 2U);
    s_tx_buf[0] = addr;
    s_tx_buf[1] = MB_FC_READ_HOLDING;
    s_tx_buf[2] = byte_count;

    for (uint16_t i = 0U; i < count; i++)
    {
        uint16_t value = 0U;
        uint8_t  ex    = MBReg_ReadHolding((uint16_t)(start + i), &value);
        if (ex != 0U)
        {
            send_exception(addr, MB_FC_READ_HOLDING, ex);
            return;
        }
        s_tx_buf[3U + i * 2U]      = (uint8_t)(value >> 8U);
        s_tx_buf[3U + i * 2U + 1U] = (uint8_t)(value & 0xFFU);
    }
    send_response(s_tx_buf, (uint16_t)(3U + byte_count));
}

/** FC04 — Read Input Registers */
static void handle_fc04(uint8_t addr, const uint8_t *pdu, uint16_t pdu_len)
{
    if (pdu_len < 5U) { send_exception(addr, MB_FC_READ_INPUT, MB_EX_ILLEGAL_VALUE); return; }

    uint16_t start = (uint16_t)(((uint16_t)pdu[1] << 8U) | pdu[2]);
    uint16_t count = (uint16_t)(((uint16_t)pdu[3] << 8U) | pdu[4]);

    if (count == 0U || count > 125U)
    {
        send_exception(addr, MB_FC_READ_INPUT, MB_EX_ILLEGAL_VALUE);
        return;
    }

    uint8_t byte_count = (uint8_t)(count * 2U);
    s_tx_buf[0] = addr;
    s_tx_buf[1] = MB_FC_READ_INPUT;
    s_tx_buf[2] = byte_count;

    for (uint16_t i = 0U; i < count; i++)
    {
        uint16_t value = 0U;
        uint8_t  ex    = MBReg_ReadInput((uint16_t)(start + i), &value);
        if (ex != 0U)
        {
            send_exception(addr, MB_FC_READ_INPUT, ex);
            return;
        }
        s_tx_buf[3U + i * 2U]      = (uint8_t)(value >> 8U);
        s_tx_buf[3U + i * 2U + 1U] = (uint8_t)(value & 0xFFU);
    }
    send_response(s_tx_buf, (uint16_t)(3U + byte_count));
}

/** FC06 — Write Single Register */
static void handle_fc06(uint8_t addr, const uint8_t *pdu, uint16_t pdu_len,
                        uint8_t is_broadcast)
{
    if (pdu_len < 5U)
    {
        if (!is_broadcast) send_exception(addr, MB_FC_WRITE_SINGLE, MB_EX_ILLEGAL_VALUE);
        return;
    }

    uint16_t reg_addr = (uint16_t)(((uint16_t)pdu[1] << 8U) | pdu[2]);
    uint16_t value    = (uint16_t)(((uint16_t)pdu[3] << 8U) | pdu[4]);

    uint8_t ex = MBReg_WriteHolding(reg_addr, value);
    if (ex != 0U)
    {
        if (!is_broadcast) send_exception(addr, MB_FC_WRITE_SINGLE, ex);
        return;
    }

    if (!is_broadcast)
    {
        /* Echo the request as response */
        memcpy(s_tx_buf, pdu, 5U);
        s_tx_buf[0] = addr;
        send_response(s_tx_buf, 5U);
    }
}

/** FC16 — Write Multiple Registers */
static void handle_fc16(uint8_t addr, const uint8_t *pdu, uint16_t pdu_len,
                        uint8_t is_broadcast)
{
    if (pdu_len < 7U)
    {
        if (!is_broadcast) send_exception(addr, MB_FC_WRITE_MULTIPLE, MB_EX_ILLEGAL_VALUE);
        return;
    }

    uint16_t start      = (uint16_t)(((uint16_t)pdu[1] << 8U) | pdu[2]);
    uint16_t count      = (uint16_t)(((uint16_t)pdu[3] << 8U) | pdu[4]);
    uint8_t  byte_count = pdu[5];

    if (count == 0U || count > 123U || byte_count != (uint8_t)(count * 2U))
    {
        if (!is_broadcast) send_exception(addr, MB_FC_WRITE_MULTIPLE, MB_EX_ILLEGAL_VALUE);
        return;
    }

    if (pdu_len < (uint16_t)(6U + byte_count + 2U)) /* +2 for CRC already in pdu_len? no, pdu includes CRC */
    {
        if (!is_broadcast) send_exception(addr, MB_FC_WRITE_MULTIPLE, MB_EX_ILLEGAL_VALUE);
        return;
    }

    for (uint16_t i = 0U; i < count; i++)
    {
        uint16_t value = (uint16_t)(((uint16_t)pdu[6U + i * 2U] << 8U) |
                                              pdu[7U + i * 2U]);
        uint8_t ex = MBReg_WriteHolding((uint16_t)(start + i), value);
        if (ex != 0U)
        {
            if (!is_broadcast) send_exception(addr, MB_FC_WRITE_MULTIPLE, ex);
            return;
        }
    }

    if (!is_broadcast)
    {
        s_tx_buf[0] = addr;
        s_tx_buf[1] = MB_FC_WRITE_MULTIPLE;
        s_tx_buf[2] = (uint8_t)(start >> 8U);
        s_tx_buf[3] = (uint8_t)(start & 0xFFU);
        s_tx_buf[4] = (uint8_t)(count >> 8U);
        s_tx_buf[5] = (uint8_t)(count & 0xFFU);
        send_response(s_tx_buf, 6U);
    }
}

/* --------------------------------------------------------------------------
 * Public API
 * -------------------------------------------------------------------------- */

void Modbus_Init(UART_HandleTypeDef *huart, TIM_HandleTypeDef *htim6)
{
    s_huart = huart;
    s_htim6 = htim6;
    s_rx_len = 0U;
    s_frame_ready = 0U;
    rs485_rx_enable();
    /* Arm single-byte interrupt reception */
    HAL_UART_Receive_IT(s_huart, &s_rx_byte, 1U);
}

void Modbus_RxByteCallback(uint8_t byte)
{
    /* Restart inter-frame gap timer on every received byte */
    __HAL_TIM_SET_COUNTER(s_htim6, 0U);
    HAL_TIM_Base_Stop_IT(s_htim6);
    HAL_TIM_Base_Start_IT(s_htim6);

    if (s_rx_len < MB_MAX_FRAME_SIZE)
    {
        s_rx_buf[s_rx_len++] = byte;
    }
    else
    {
        /* Buffer overflow — discard and reset */
        s_rx_len = 0U;
    }

    /* Re-arm for the next byte */
    HAL_UART_Receive_IT(s_huart, &s_rx_byte, 1U);
}

void Modbus_FrameTimeoutCallback(void)
{
    HAL_TIM_Base_Stop_IT(s_htim6);
    if (s_rx_len >= 4U)  /* Minimum valid frame: addr + fc + CRC(2) */
    {
        s_frame_ready = 1U;
    }
    else
    {
        s_rx_len = 0U;  /* Too short — discard */
    }
}

void Modbus_Process(void)
{
    if (!s_frame_ready) return;

    /* Snapshot and clear the flag before processing */
    uint16_t frame_len  = s_rx_len;
    s_frame_ready = 0U;
    s_rx_len      = 0U;

    uint8_t device_addr  = s_rx_buf[0];
    uint8_t is_broadcast = (device_addr == MB_BROADCAST_ADDR) ? 1U : 0U;

    /* Filter: accept only our address or broadcast */
    if (!is_broadcast && device_addr != MODBUS_DEVICE_ADDRESS) return;

    /* Validate CRC — last two bytes are CRC (lo, hi) */
    uint16_t recv_crc = (uint16_t)(s_rx_buf[frame_len - 2U] |
                                   ((uint16_t)s_rx_buf[frame_len - 1U] << 8U));
    uint16_t calc_crc = crc16_calc(s_rx_buf, frame_len - 2U);
    if (recv_crc != calc_crc) return;  /* Silently discard bad CRC */

    uint8_t fc = s_rx_buf[1];

    switch (fc)
    {
        case MB_FC_READ_HOLDING:
            if (is_broadcast) return;  /* Reads never broadcast */
            handle_fc03(device_addr, s_rx_buf, frame_len);
            break;
        case MB_FC_READ_INPUT:
            if (is_broadcast) return;
            handle_fc04(device_addr, s_rx_buf, frame_len);
            break;
        case MB_FC_WRITE_SINGLE:
            handle_fc06(device_addr, s_rx_buf, frame_len, is_broadcast);
            break;
        case MB_FC_WRITE_MULTIPLE:
            handle_fc16(device_addr, s_rx_buf, frame_len, is_broadcast);
            break;
        default:
            if (!is_broadcast)
                send_exception(device_addr, fc, MB_EX_ILLEGAL_FUNCTION);
            break;
    }
}

void Modbus_TxCompleteCallback(void)
{
    rs485_rx_enable();
}
