/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
DAC_HandleTypeDef hdac;

TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DAC_Init(void);
static void MX_TIM2_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint16_t sawtooth100[100] = {0, 41, 82, 123, 164, 205, 246, 287, 328, 369,
                        410, 451, 492, 533, 574, 615, 656, 697, 738, 779,
                        820, 861, 902, 943, 984, 1025, 1066, 1107, 1148, 1189,
                        1230, 1271, 1312, 1353, 1394, 1435, 1476, 1517, 1558, 1599,
                        1640, 1681, 1722, 1763, 1804, 1845, 1886, 1927, 1968, 2009,
                        2050, 2091, 2132, 2173, 2214, 2255, 2296, 2337, 2378, 2419,
                        2460, 2501, 2542, 2583, 2624, 2665, 2706, 2747, 2788, 2829,
                        2870, 2911, 2952, 2993, 3034, 3075, 3116, 3157, 3198, 3239,
                        3280, 3321, 3362, 3403, 3444, 3485, 3526, 3567, 3608, 3649,
                        3690, 3731, 3772, 3813, 3854, 3895, 3936, 3977, 4018, 4059};
uint16_t triangle100[100] = {0, 81, 162, 243, 324, 405, 486, 567, 648, 729,
                        810, 891, 972, 1053, 1134, 1215, 1296, 1377, 1458, 1539,
                        1620, 1701, 1782, 1863, 1944, 2025, 2106, 2187, 2268, 2349,
                        2430, 2511, 2592, 2673, 2754, 2835, 2916, 2997, 3078, 3159,
                        3240, 3321, 3402, 3483, 3564, 3645, 3726, 3807, 3888, 3969,
                        4050, 3969, 3888, 3807, 3726, 3645, 3564, 3483, 3402, 3321,
                        3240, 3159, 3078, 2997, 2916, 2835, 2754, 2673, 2592, 2511,
                        2430, 2349, 2268, 2187, 2106, 2025, 1944, 1863, 1782, 1701,
                        1620, 1539, 1458, 1377, 1296, 1215, 1134, 1053, 972, 891,
                        810, 729, 648, 567, 486, 405, 324, 243, 162, 81};
uint16_t sinewave100[100] = {0, 4, 16, 36, 64, 100, 144, 195, 253, 319,
                        391, 470, 555, 646, 742, 844, 950, 1061, 1176, 1294,
                        1415, 1538, 1664, 1791, 1919, 2048, 2176, 2304, 2431, 2557,
                        2680, 2801, 2919, 3034, 3145, 3251, 3353, 3449, 3540, 3625,
                        3704, 3776, 3842, 3900, 3951, 3995, 4031, 4059, 4079, 4091,
                        4095, 4091, 4079, 4059, 4031, 3995, 3951, 3900, 3842, 3776,
                        3704, 3625, 3540, 3449, 3353, 3251, 3145, 3034, 2919, 2801,
                        2680, 2557, 2431, 2304, 2176, 2048, 1919, 1791, 1664, 1538,
                        1415, 1294, 1176, 1061, 950, 844, 742, 646, 555, 470,
                        391, 319, 253, 195, 144, 100, 64, 36, 16, 4};

uint16_t sawtooth10[10]  = {0, 410, 820, 1230, 1640, 2050, 2460, 2870, 3280, 3690};
uint16_t triangle10[10]  = {0, 810, 1620, 2430, 3240, 4050, 3240, 2430, 1620, 810};
uint16_t sinewave10[10]  = {0, 391, 1415, 2680, 3704, 4095, 3704, 2680, 1415, 391};

/* 0.5V – 1.0V  (DAC 620–1241) */
uint16_t sawtooth_0p5_1v[100] = {620, 626, 633, 639, 645, 651, 658, 664, 670, 676,
                        683, 689, 695, 702, 708, 714, 720, 727, 733, 739,
                        745, 752, 758, 764, 771, 777, 783, 789, 796, 802,
                        808, 814, 821, 827, 833, 840, 846, 852, 858, 865,
                        871, 877, 883, 890, 896, 902, 909, 915, 921, 927,
                        934, 940, 946, 952, 959, 965, 971, 978, 984, 990,
                        996, 1003, 1009, 1015, 1021, 1028, 1034, 1040, 1047, 1053,
                        1059, 1065, 1072, 1078, 1084, 1090, 1097, 1103, 1109, 1116,
                        1122, 1128, 1134, 1141, 1147, 1153, 1159, 1166, 1172, 1178,
                        1185, 1191, 1197, 1203, 1210, 1216, 1222, 1228, 1235, 1241};
uint16_t triangle_0p5_1v[100] = {620, 632, 645, 657, 670, 682, 695, 707, 719, 732,
                        744, 757, 769, 781, 794, 806, 819, 831, 844, 856,
                        868, 881, 893, 906, 918, 931, 943, 955, 968, 980,
                        993, 1005, 1017, 1030, 1042, 1055, 1067, 1080, 1092, 1104,
                        1117, 1129, 1142, 1154, 1166, 1179, 1191, 1204, 1216, 1229,
                        1241, 1229, 1216, 1204, 1191, 1179, 1166, 1154, 1142, 1129,
                        1117, 1104, 1092, 1080, 1067, 1055, 1042, 1030, 1017, 1005,
                        993, 980, 968, 955, 943, 931, 918, 906, 893, 881,
                        868, 856, 844, 831, 819, 806, 794, 781, 769, 757,
                        744, 732, 719, 707, 695, 682, 670, 657, 645, 632};
uint16_t sinewave_0p5_1v[100] = {620, 621, 622, 625, 630, 635, 642, 650, 658, 668,
                        679, 691, 704, 718, 733, 748, 764, 781, 798, 816,
                        835, 853, 872, 892, 911, 931, 950, 969, 989, 1008,
                        1026, 1045, 1063, 1080, 1097, 1113, 1128, 1143, 1157, 1170,
                        1182, 1193, 1203, 1211, 1219, 1226, 1231, 1236, 1239, 1240,
                        1241, 1240, 1239, 1236, 1231, 1226, 1219, 1211, 1203, 1193,
                        1182, 1170, 1157, 1143, 1128, 1113, 1097, 1080, 1063, 1045,
                        1026, 1008, 989, 969, 950, 931, 911, 892, 872, 853,
                        835, 816, 798, 781, 764, 748, 733, 718, 704, 691,
                        679, 668, 658, 650, 642, 635, 630, 625, 622, 621};

/* 1.0V – 2.0V  (DAC 1241–2482) */
uint16_t sawtooth_1v_2v[100] = {1241, 1254, 1266, 1279, 1291, 1304, 1316, 1329, 1341, 1354,
                        1366, 1379, 1391, 1404, 1416, 1429, 1442, 1454, 1467, 1479,
                        1492, 1504, 1517, 1529, 1542, 1554, 1567, 1579, 1592, 1605,
                        1617, 1630, 1642, 1655, 1667, 1680, 1692, 1705, 1717, 1730,
                        1742, 1755, 1767, 1780, 1793, 1805, 1818, 1830, 1843, 1855,
                        1868, 1880, 1893, 1905, 1918, 1930, 1943, 1956, 1968, 1981,
                        1993, 2006, 2018, 2031, 2043, 2056, 2068, 2081, 2093, 2106,
                        2118, 2131, 2144, 2156, 2169, 2181, 2194, 2206, 2219, 2231,
                        2244, 2256, 2269, 2281, 2294, 2307, 2319, 2332, 2344, 2357,
                        2369, 2382, 2394, 2407, 2419, 2432, 2444, 2457, 2469, 2482};
uint16_t triangle_1v_2v[100] = {1241, 1266, 1291, 1315, 1340, 1365, 1390, 1415, 1440, 1464,
                        1489, 1514, 1539, 1564, 1588, 1613, 1638, 1663, 1688, 1713,
                        1737, 1762, 1787, 1812, 1837, 1862, 1886, 1911, 1936, 1961,
                        1986, 2010, 2035, 2060, 2085, 2110, 2135, 2159, 2184, 2209,
                        2234, 2259, 2283, 2308, 2333, 2358, 2383, 2408, 2432, 2457,
                        2482, 2457, 2432, 2408, 2383, 2358, 2333, 2308, 2283, 2259,
                        2234, 2209, 2184, 2159, 2135, 2110, 2085, 2060, 2035, 2010,
                        1986, 1961, 1936, 1911, 1886, 1862, 1837, 1812, 1787, 1762,
                        1737, 1713, 1688, 1663, 1638, 1613, 1588, 1564, 1539, 1514,
                        1489, 1464, 1440, 1415, 1390, 1365, 1340, 1315, 1291, 1266};
uint16_t sinewave_1v_2v[100] = {1241, 1242, 1246, 1252, 1260, 1271, 1285, 1300, 1318, 1338,
                        1360, 1383, 1409, 1437, 1466, 1497, 1529, 1563, 1597, 1633,
                        1670, 1707, 1745, 1784, 1823, 1862, 1900, 1939, 1978, 2016,
                        2053, 2090, 2126, 2160, 2194, 2226, 2257, 2286, 2314, 2340,
                        2363, 2385, 2405, 2423, 2438, 2452, 2463, 2471, 2477, 2481,
                        2482, 2481, 2477, 2471, 2463, 2452, 2438, 2423, 2405, 2385,
                        2363, 2340, 2314, 2286, 2257, 2226, 2194, 2160, 2126, 2090,
                        2053, 2016, 1978, 1939, 1900, 1862, 1823, 1784, 1745, 1707,
                        1670, 1633, 1597, 1563, 1529, 1497, 1466, 1437, 1409, 1383,
                        1360, 1338, 1318, 1300, 1285, 1271, 1260, 1252, 1246, 1242};

/* 2.0V – 2.5V  (DAC 2482–3102) */
uint16_t sawtooth_2v_2p5v[100] = {2482, 2488, 2495, 2501, 2507, 2513, 2520, 2526, 2532, 2538,
                        2545, 2551, 2557, 2563, 2570, 2576, 2582, 2588, 2595, 2601,
                        2607, 2614, 2620, 2626, 2632, 2639, 2645, 2651, 2657, 2664,
                        2670, 2676, 2682, 2689, 2695, 2701, 2707, 2714, 2720, 2726,
                        2733, 2739, 2745, 2751, 2758, 2764, 2770, 2776, 2783, 2789,
                        2795, 2801, 2808, 2814, 2820, 2826, 2833, 2839, 2845, 2851,
                        2858, 2864, 2870, 2877, 2883, 2889, 2895, 2902, 2908, 2914,
                        2920, 2927, 2933, 2939, 2945, 2952, 2958, 2964, 2970, 2977,
                        2983, 2989, 2996, 3002, 3008, 3014, 3021, 3027, 3033, 3039,
                        3046, 3052, 3058, 3064, 3071, 3077, 3083, 3089, 3096, 3102};
uint16_t triangle_2v_2p5v[100] = {2482, 2494, 2507, 2519, 2532, 2544, 2556, 2569, 2581, 2594,
                        2606, 2618, 2631, 2643, 2656, 2668, 2680, 2693, 2705, 2718,
                        2730, 2742, 2755, 2767, 2780, 2792, 2804, 2817, 2829, 2842,
                        2854, 2866, 2879, 2891, 2904, 2916, 2928, 2941, 2953, 2966,
                        2978, 2990, 3003, 3015, 3028, 3040, 3052, 3065, 3077, 3090,
                        3102, 3090, 3077, 3065, 3052, 3040, 3028, 3015, 3003, 2990,
                        2978, 2966, 2953, 2941, 2928, 2916, 2904, 2891, 2879, 2866,
                        2854, 2842, 2829, 2817, 2804, 2792, 2780, 2767, 2755, 2742,
                        2730, 2718, 2705, 2693, 2680, 2668, 2656, 2643, 2631, 2618,
                        2606, 2594, 2581, 2569, 2556, 2544, 2532, 2519, 2507, 2494};
uint16_t sinewave_2v_2p5v[100] = {2482, 2483, 2484, 2487, 2492, 2497, 2504, 2512, 2520, 2530,
                        2541, 2553, 2566, 2580, 2594, 2610, 2626, 2643, 2660, 2678,
                        2696, 2715, 2734, 2753, 2773, 2792, 2811, 2831, 2850, 2869,
                        2888, 2906, 2924, 2941, 2958, 2974, 2990, 3004, 3018, 3031,
                        3043, 3054, 3064, 3072, 3080, 3087, 3092, 3097, 3100, 3101,
                        3102, 3101, 3100, 3097, 3092, 3087, 3080, 3072, 3064, 3054,
                        3043, 3031, 3018, 3004, 2990, 2974, 2958, 2941, 2924, 2906,
                        2888, 2869, 2850, 2831, 2811, 2792, 2773, 2753, 2734, 2715,
                        2696, 2678, 2660, 2643, 2626, 2610, 2594, 2580, 2566, 2553,
                        2541, 2530, 2520, 2512, 2504, 2497, 2492, 2487, 2484, 2483};

void microDelay (uint16_t delay)
{
  __HAL_TIM_SET_COUNTER(&htim2, 0);
  while (__HAL_TIM_GET_COUNTER(&htim2) < delay);
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DAC_Init();
  MX_TIM2_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
    HAL_DAC_Start(&hdac, DAC_CHANNEL_1);
    HAL_TIM_Base_Start(&htim2);
//    HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, 1*4096/3.3); //1241
//    HAL_Delay(3000);
//    HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, 2*4096/3.3); //2482
//    HAL_Delay(3000);
//    HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, 3*4096/3.3); //3723
//    HAL_Delay(3000);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
      /* 0.5V-1.0V for 5 s  (100pts x 1us = 100us/cycle, 50000 cycles = 5s) */
      for(uint32_t cycle = 0; cycle < 50000; cycle++)
      {
          for(uint8_t i = 0; i < 100; i++)
          {
              HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, triangle_0p5_1v[i]);
              microDelay(1);
          }
      }
      /* 1.0V-2.0V for 5 s */
      for(uint32_t cycle = 0; cycle < 50000; cycle++)
      {
          for(uint8_t i = 0; i < 100; i++)
          {
              HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, triangle_1v_2v[i]);
              microDelay(1);
          }
      }
      /* 2.0V-2.5V for 5 s */
      for(uint32_t cycle = 0; cycle < 50000; cycle++)
      {
          for(uint8_t i = 0; i < 100; i++)
          {
              HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, triangle_2v_2p5v[i]);
              microDelay(1);
          }
      }
    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 180;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Activate the Over-Drive mode
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief DAC Initialization Function
  * @param None
  * @retval None
  */
static void MX_DAC_Init(void)
{

  /* USER CODE BEGIN DAC_Init 0 */

  /* USER CODE END DAC_Init 0 */

  DAC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN DAC_Init 1 */

  /* USER CODE END DAC_Init 1 */

  /** DAC Initialization
  */
  hdac.Instance = DAC;
  if (HAL_DAC_Init(&hdac) != HAL_OK)
  {
    Error_Handler();
  }

  /** DAC channel OUT1 config
  */
  sConfig.DAC_Trigger = DAC_TRIGGER_NONE;
  sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
  if (HAL_DAC_ConfigChannel(&hdac, &sConfig, DAC_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN DAC_Init 2 */

  /* USER CODE END DAC_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 89;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 4294967295;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
