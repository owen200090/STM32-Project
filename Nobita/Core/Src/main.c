/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
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
TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM4_Init(void);
static void MX_TIM3_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint8_t RX2_Char[2];
uint8_t Char_Buffer[20];
uint8_t Char_Buffer_length = 0;
uint8_t Char_Buffer_isRecieving = 0;

uint8_t MSG1[50];
uint8_t MSG1_length;

uint8_t MSG2[50];
uint8_t MSG2_length;

uint32_t encoder_value_l = 0;
uint32_t encoder_value_r = 0;
float rpm_value_l = 0.00;
float rpm_value_r = 0.00;

float linear_vel_x = 0.0;
float angular_vel_z = 0.0;
float wheel_l = 0.0;
float wheel_r = 0.0;
int direction_l = 0;
int direction_r = 0;
float encoder_l = 0.0;
float encoder_r = 0.0;

//PID Variables
float kp = 1;
float ki = 0.01;
float kd = 1;
//Set point is wheel_l and wheel_r
unsigned long currentTime, previousTime, elapsedTime;
float error, error_p;
float intVal_L, intVal_R, outVal, setPoint;
float cumError = 0;
float rateError = 0;
float PIDOut_L = 0;
float PIDOut_R = 0;
//PID Struct
struct PIDValues{
	float input = 0;
	float setPoint = 0;
	float error = 0;
	float error_p = 0;
	float cumError = 0;
	float rateErorr = 0;
	float output = 0;
};
PIDValues left_wheel;
PIDValues right_wheel;
PIDValues left_steer;
PIDValues right_steer;
float computePID(struct PIDValues wheel);

char buff1[50];
int out1;
char buff2[50];
int out2;

void set_speed(float Vx, float W){
	uint8_t direction_l = 1;
	uint8_t direction_r = 1;
	float _Wl = (Vx - (2.22169002*W))/1.08 ;
	float _Wr = ((2.22169002*W) + Vx)/1.08;
	if(_Wl < 0){
	  direction_l = 0;
	  _Wl = -_Wl;
	 }
	if(_Wr < 0){
		direction_r = 0;
		_Wr = -_Wr;
	}
	wheel_l = _Wl;
	wheel_r = _Wr;
//	HAL_GPIO_WritePin(MOTOR_PWM_GPIO_Port,MOTOR_PWM_Pin,direction_l);
	HAL_GPIO_WritePin(MOTOR_DIRECTION1_GPIO_Port,MOTOR_DIRECTION1_Pin,direction_l);
	HAL_GPIO_WritePin(MOTOR_DIRECTION2_GPIO_Port,MOTOR_DIRECTION2_Pin,!direction_l);
	 __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, _Wl);
	//HAL_GPIO_WritePin(MOTOR_PWM_GPIO_Port,MOTOR2_PWM_Pin,direction_r);
	 HAL_GPIO_WritePin(MOTOR2_DIRECTION1_GPIO_Port,MOTOR2_DIRECTION1_Pin,direction_r);
	 HAL_GPIO_WritePin(MOTOR2_DIRECTION2_GPIO_Port,MOTOR2_DIRECTION2_Pin,!direction_r);
	 __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, _Wr);
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */

void getTime(){
	currentTime = millis();
	elapsedTime = (float)(currentTime-elapsedTime);
	previousTime = currentTime;
}
float computePID(struct PIDValues wheel){
	wheel.error = wheel.setPoint - wheel.input;
	wheel.cumError += wheel.error * elapsedTime;
	wheel.rateErorr += (wheel.error - wheel.error_p)/elapsedTime;
	wheel.output = kp*wheel.error + ki*wheel.cumError + kd*wheel.rateErorr;
	wheel.error_p = wheel.error;
	return wheel.output
}

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
  MX_TIM4_Init();
  MX_TIM3_Init();
  MX_USART2_UART_Init();
  MX_TIM1_Init();
  /* USER CODE BEGIN 2 */

//  MSG1_length = sprintf(MSG1,"HELLO\n");
  HAL_UART_Transmit_IT(&huart2, MSG1, MSG1_length);

  HAL_UART_Receive_IT(&huart2, RX2_Char, 1);
  HAL_TIM_Base_Start_IT(&htim1);
  HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL);

  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);
//  set_speed(100);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	encoder_l = encoder_value_l;
	encoder_r = encoder_value_r;
	//encoder_r =
	  getTime();
	  //Left wheel
	  left_wheel.input = encoder_l;
	  left_wheel.setPoint = wheel_l;
	  //RIGHT WHEEL
	  right_wheel.input = encoder_r;
	  right_wheel.setPoint = wheel_r;
	  PIDOut_L = computePID(left_wheel);
	  PIDOut_R = computePID(right_wheel);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
//	  encoder_value = htim3.Instance->CNT;
//	  MSG1_length = sprintf(MSG1, "Encoder value = %ld\r\n", encoder_value);
// 	  HAL_UART_Transmit_IT(&huart2, MSG1, MSG1_length);
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 8400;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 9999;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_Encoder_InitTypeDef sConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 0;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 65535;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  sConfig.EncoderMode = TIM_ENCODERMODE_TI1;
  sConfig.IC1Polarity = TIM_ICPOLARITY_RISING;
  sConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
  sConfig.IC1Prescaler = TIM_ICPSC_DIV1;
  sConfig.IC1Filter = 0;
  sConfig.IC2Polarity = TIM_ICPOLARITY_RISING;
  sConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
  sConfig.IC2Prescaler = TIM_ICPSC_DIV1;
  sConfig.IC2Filter = 0;
  if (HAL_TIM_Encoder_Init(&htim3, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 84;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 1000;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */
  HAL_TIM_MspPostInit(&htim4);

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

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5|MOTOR2_PWM_Pin|MOTOR_PWM_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, MOTOR_DIRECTION1_Pin|MOTOR_DIRECTION2_Pin|MOTOR2_DIRECTION1_Pin|MOTOR2_DIRECTION2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PA5 MOTOR2_PWM_Pin MOTOR_PWM_Pin */
  GPIO_InitStruct.Pin = GPIO_PIN_5|MOTOR2_PWM_Pin|MOTOR_PWM_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : MOTOR_DIRECTION1_Pin MOTOR_DIRECTION2_Pin MOTOR2_DIRECTION1_Pin MOTOR2_DIRECTION2_Pin */
  GPIO_InitStruct.Pin = MOTOR_DIRECTION1_Pin|MOTOR_DIRECTION2_Pin|MOTOR2_DIRECTION1_Pin|MOTOR2_DIRECTION2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

}

/* USER CODE BEGIN 4 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
//	HAL_GPIO_TogglePin(LD2_GPIO_Port,LD2_Pin);
}
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart == &huart2){
		HAL_UART_Receive_IT(&huart2, RX2_Char, 1);
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
//	HAL_GPIO_TogglePin(LD2_GPIO_Port,LD2_Pin);
//	HAL_UART_Transmit_IT(&huart2, MSG1, 4);
	if(huart == &huart2){
		if(RX2_Char[0] == '<'){
			Char_Buffer_isRecieving = 1;
		}else if(RX2_Char[0] == '>'){
//			if(Char_Buffer[2] == 'n') 		HAL_GPIO_WritePin(LD2_GPIO_Port,LD2_Pin,1);
//			else if(Char_Buffer[2] == 'f')	HAL_GPIO_WritePin(LD2_GPIO_Port,LD2_Pin,0);
			int speed = 0;
			uint8_t direction = 0;
 			if(Char_Buffer[1] == '-'){
 				direction = 1;
 			}
			if(Char_Buffer_length-1 == 1){
				speed = Char_Buffer[direction+1]-48;
			}
			speed *= 100;
//			set_speed(speed * (direction ? -1:1));

			MSG1_length = sprintf(MSG1,"Set Speed = %d\n", speed);
			HAL_UART_Transmit_IT(&huart2, MSG1, MSG1_length);

//			uint8_t
			Char_Buffer[Char_Buffer_length++] = '>';
			Char_Buffer[Char_Buffer_length++] = '\n';
			HAL_UART_Transmit_IT(&huart2, Char_Buffer, Char_Buffer_length);
			Char_Buffer_isRecieving = 0;
			Char_Buffer_length = 0;
		}

		if(Char_Buffer_isRecieving){
			Char_Buffer[Char_Buffer_length++] = RX2_Char[0];
		}
	}
	HAL_UART_Receive_IT(&huart2, RX2_Char, 1);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
//	HAL_GPIO_TogglePin(Ld2_GPIO_Port,Ld2_Pin);

	encoder_value_l = htim3.Instance->CNT - 30000;
	encoder_value_r = htim3.Instance->CNT - 30000;
	htim3.Instance->CNT = 30000;
	rpm_value_l = encoder_value_l*60.0/46000.0;
	rpm_value_r = encoder_value_r*60.0/46000.0;
	MSG1_length = sprintf(MSG1,"Encoder = %d RPM = %.2f\n", encoder_value_l, rpm_value_l);
	MSG2_length = sprintf(MSG2,"Encoder = %d RPM = %.2f\n", encoder_value_r, rpm_value_r);
	HAL_UART_Transmit_IT(&huart2, MSG1, MSG1_length);
	HAL_UART_Transmit_IT(&huart2, MSG2, MSG2_length);
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
