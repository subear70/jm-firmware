STM32CubeIDE
Prerequisites
This project assumes you have already installed STM32CubeIDE. You need to have previously done a basic blink sketch with Nucleo STM32F446RE using STM32CubeIDE. I have made a complete video from installing STM32CubeIDE to LED blink program. You can watch it by clicking this link. https://www.youtube.com/watch?v=oAwZ0cjlmN8

STM32CubeIDE Settings
Click Clock Configuration tab → HSE Input Frequency 8

Select HSE & Select PLLCLK → HCLK (MHz) to 180

Chips & Processors

Click Pinout and Configuration tab

Click Timer → Click TIM2 →

Clock Source set to Internal Clock

Configuration → Parameter Settings →

Prescaler set to 90-1 (APB1Timer-1)

Click Analog → Click DAC → Tick OUT1 Configuration

Additional code on top of STM32CubeIDE generated code
/* USER CODE BEGIN 0 */
uint16_t sawtooth[100] = {0, 41, 82, 123, 164, 205, 246, 287, 328, 369,
                        410, 451, 492, 533, 574, 615, 656, 697, 738, 779,
                        820, 861, 902, 943, 984, 1025, 1066, 1107, 1148, 1189,
                        1230, 1271, 1312, 1353, 1394, 1435, 1476, 1517, 1558, 1599,
                        1640, 1681, 1722, 1763, 1804, 1845, 1886, 1927, 1968, 2009,
                        2050, 2091, 2132, 2173, 2214, 2255, 2296, 2337, 2378, 2419,
                        2460, 2501, 2542, 2583, 2624, 2665, 2706, 2747, 2788, 2829,
                        2870, 2911, 2952, 2993, 3034, 3075, 3116, 3157, 3198, 3239,
                        3280, 3321, 3362, 3403, 3444, 3485, 3526, 3567, 3608, 3649,
                        3690, 3731, 3772, 3813, 3854, 3895, 3936, 3977, 4018, 4059};
uint16_t triangle[100] = {0, 81, 162, 243, 324, 405, 486, 567, 648, 729,
                        810, 891, 972, 1053, 1134, 1215, 1296, 1377, 1458, 1539,
                        1620, 1701, 1782, 1863, 1944, 2025, 2106, 2187, 2268, 2349,
                        2430, 2511, 2592, 2673, 2754, 2835, 2916, 2997, 3078, 3159,
                        3240, 3321, 3402, 3483, 3564, 3645, 3726, 3807, 3888, 3969,
                        4050, 3969, 3888, 3807, 3726, 3645, 3564, 3483, 3402, 3321,
                        3240, 3159, 3078, 2997, 2916, 2835, 2754, 2673, 2592, 2511,
                        2430, 2349, 2268, 2187, 2106, 2025, 1944, 1863, 1782, 1701,
                        1620, 1539, 1458, 1377, 1296, 1215, 1134, 1053, 972, 891,
                        810, 729, 648, 567, 486, 405, 324, 243, 162, 81};
uint16_t sinewave[100] = {0, 4, 16, 36, 64, 100, 144, 195, 253, 319,
                        391, 470, 555, 646, 742, 844, 950, 1061, 1176, 1294,
                        1415, 1538, 1664, 1791, 1919, 2048, 2176, 2304, 2431, 2557,
                        2680, 2801, 2919, 3034, 3145, 3251, 3353, 3449, 3540, 3625,
                        3704, 3776, 3842, 3900, 3951, 3995, 4031, 4059, 4079, 4091,
                        4095, 4091, 4079, 4059, 4031, 3995, 3951, 3900, 3842, 3776,
                        3704, 3625, 3540, 3449, 3353, 3251, 3145, 3034, 2919, 2801,
                        2680, 2557, 2431, 2304, 2176, 2048, 1919, 1791, 1664, 1538,
                        1415, 1294, 1176, 1061, 950, 844, 742, 646, 555, 470,
                        391, 319, 253, 195, 144, 100, 64, 36, 16, 4};

void microDelay (uint16_t delay)
{
  __HAL_TIM_SET_COUNTER(&htim2, 0);
  while (__HAL_TIM_GET_COUNTER(&htim2) < delay);
}
/* USER CODE END 0 */

  /* USER CODE BEGIN 2 */
  HAL_DAC_Start(&hdac, DAC_CHANNEL_1);
  HAL_TIM_Base_Start(&htim2);
  HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, 1*4096/3.3); //1241
  HAL_Delay(3000);
  HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, 2*4096/3.3); //2482
  HAL_Delay(3000);
  HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, 3*4096/3.3); //3723
  HAL_Delay(3000);
  /* USER CODE END 2 */

  /* USER CODE BEGIN WHILE */
  while (1)
  {
      for(uint8_t i=0; i<100; i++)
      {
          HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, sawtooth[i]);
          microDelay(1);
      }
    /* USER CODE END WHILE */