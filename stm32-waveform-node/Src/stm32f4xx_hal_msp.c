/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file         stm32f4xx_hal_msp.c
  * @brief        This file provides code for the MSP Initialization
  *               and de-Initialization codes.
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
/* USER CODE BEGIN Includes */
#include "config.h"
extern DMA_HandleTypeDef hdma_dac1;
extern DMA_HandleTypeDef hdma_dac2;
/* USER CODE END Includes */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */
/**
  * Initializes the Global MSP.
  */
void HAL_MspInit(void)
{

  /* USER CODE BEGIN MspInit 0 */

  /* USER CODE END MspInit 0 */

  __HAL_RCC_SYSCFG_CLK_ENABLE();
  __HAL_RCC_PWR_CLK_ENABLE();

  HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_0);

  /* System interrupt init*/

  /* USER CODE BEGIN MspInit 1 */

  /* USER CODE END MspInit 1 */
}

/**
  * @brief DAC MSP Initialization
  * @param hdac: DAC handle pointer
  * @retval None
  */
void HAL_DAC_MspInit(DAC_HandleTypeDef* hdac)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(hdac->Instance==DAC)
  {
    /* USER CODE BEGIN DAC_MspInit 0 */

    /* USER CODE END DAC_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_DAC_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**DAC GPIO Configuration
    PA4     ------> DAC_OUT1
    */
    GPIO_InitStruct.Pin = GPIO_PIN_4;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* USER CODE BEGIN DAC_MspInit 1 */

    /* DAC1 DMA Init — DMA1_Stream5, Channel 7 (memory-to-peripheral, circular) */
    hdma_dac1.Instance                 = DMA1_Stream5;
    hdma_dac1.Init.Channel             = DMA_CHANNEL_7;
    hdma_dac1.Init.Direction           = DMA_MEMORY_TO_PERIPH;
    hdma_dac1.Init.PeriphInc           = DMA_PINC_DISABLE;
    hdma_dac1.Init.MemInc              = DMA_MINC_ENABLE;
    hdma_dac1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_dac1.Init.MemDataAlignment    = DMA_MDATAALIGN_HALFWORD;
    hdma_dac1.Init.Mode                = DMA_CIRCULAR;
    hdma_dac1.Init.Priority            = DMA_PRIORITY_HIGH;
    hdma_dac1.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_dac1) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(hdac, DMA_Handle1, hdma_dac1);

    /* DAC1 channel 2 DMA Init — DMA1_Stream6, Channel 7. */
    hdma_dac2.Instance                 = DMA1_Stream6;
    hdma_dac2.Init.Channel             = DMA_CHANNEL_7;
    hdma_dac2.Init.Direction           = DMA_MEMORY_TO_PERIPH;
    hdma_dac2.Init.PeriphInc           = DMA_PINC_DISABLE;
    hdma_dac2.Init.MemInc              = DMA_MINC_ENABLE;
    hdma_dac2.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_dac2.Init.MemDataAlignment    = DMA_MDATAALIGN_HALFWORD;
    hdma_dac2.Init.Mode                = DMA_CIRCULAR;
    hdma_dac2.Init.Priority            = DMA_PRIORITY_HIGH;
    hdma_dac2.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_dac2) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(hdac, DMA_Handle2, hdma_dac2);
    /* USER CODE END DAC_MspInit 1 */
  }
}

/**
  * @brief DAC MSP De-Initialization
  * @param hdac: DAC handle pointer
  * @retval None
  */
void HAL_DAC_MspDeInit(DAC_HandleTypeDef* hdac)
{
  if(hdac->Instance==DAC)
  {
    /* USER CODE BEGIN DAC_MspDeInit 0 */

    /* USER CODE END DAC_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_DAC_CLK_DISABLE();

    /**DAC GPIO Configuration
    PA4     ------> DAC_OUT1
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_4);

    /* USER CODE BEGIN DAC_MspDeInit 1 */
    HAL_DMA_DeInit(hdac->DMA_Handle1);
    HAL_DMA_DeInit(hdac->DMA_Handle2);
    /* USER CODE END DAC_MspDeInit 1 */
  }
}

/**
  * @brief TIM_Base MSP Initialization
  * @param htim_base: TIM_Base handle pointer
  * @retval None
  */
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* htim_base)
{
  if(htim_base->Instance==TIM2)
  {
    /* USER CODE BEGIN TIM2_MspInit 0 */

    /* USER CODE END TIM2_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_TIM2_CLK_ENABLE();
    /* TIM2 interrupt Init */
    HAL_NVIC_SetPriority(TIM2_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM2_IRQn);
    /* USER CODE BEGIN TIM2_MspInit 1 */

    /* USER CODE END TIM2_MspInit 1 */
  }
  else if(htim_base->Instance==TIM6)
  {
    /* USER CODE BEGIN TIM6_MspInit 0 */

    /* USER CODE END TIM6_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_TIM6_CLK_ENABLE();
    /* TIM6 interrupt Init */
    HAL_NVIC_SetPriority(TIM6_DAC_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);
    /* USER CODE BEGIN TIM6_MspInit 1 */

    /* USER CODE END TIM6_MspInit 1 */
  }
}

/**
  * @brief TIM_Base MSP De-Initialization
  * @param htim_base: TIM_Base handle pointer
  * @retval None
  */
void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* htim_base)
{
  if(htim_base->Instance==TIM2)
  {
    /* USER CODE BEGIN TIM2_MspDeInit 0 */

    /* USER CODE END TIM2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_TIM2_CLK_DISABLE();
    /* TIM2 interrupt DeInit */
    HAL_NVIC_DisableIRQ(TIM2_IRQn);
    /* USER CODE BEGIN TIM2_MspDeInit 1 */

    /* USER CODE END TIM2_MspDeInit 1 */
  }
  else if(htim_base->Instance==TIM6)
  {
    /* USER CODE BEGIN TIM6_MspDeInit 0 */

    /* USER CODE END TIM6_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_TIM6_CLK_DISABLE();
    /* TIM6 interrupt DeInit */
    HAL_NVIC_DisableIRQ(TIM6_DAC_IRQn);
    /* USER CODE BEGIN TIM6_MspDeInit 1 */

    /* USER CODE END TIM6_MspDeInit 1 */
  }
}

/**
  * @brief UART MSP Initialization
  * @param huart: UART handle pointer
  * @retval None
  */
void HAL_UART_MspInit(UART_HandleTypeDef* huart)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(huart->Instance==MODBUS_UART_INSTANCE)
  {
    /* Peripheral clock enable */
    MODBUS_UART_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /* Modbus UART GPIO Configuration (TX / RX, alternate function) */
    GPIO_InitStruct.Pin = MODBUS_UART_TX_PIN | MODBUS_UART_RX_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = MODBUS_UART_GPIO_AF;
    HAL_GPIO_Init(MODBUS_UART_GPIO_PORT, &GPIO_InitStruct);

    /* Modbus UART interrupt Init */
    HAL_NVIC_SetPriority(MODBUS_UART_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(MODBUS_UART_IRQn);
  }
}

/**
  * @brief UART MSP De-Initialization
  * @param huart: UART handle pointer
  * @retval None
  */
void HAL_UART_MspDeInit(UART_HandleTypeDef* huart)
{
  if(huart->Instance==MODBUS_UART_INSTANCE)
  {
    /* Peripheral clock disable */
    MODBUS_UART_CLK_DISABLE();

    /* Modbus UART GPIO deinit */
    HAL_GPIO_DeInit(MODBUS_UART_GPIO_PORT, MODBUS_UART_TX_PIN | MODBUS_UART_RX_PIN);

    /* Modbus UART interrupt DeInit */
    HAL_NVIC_DisableIRQ(MODBUS_UART_IRQn);
  }
}
