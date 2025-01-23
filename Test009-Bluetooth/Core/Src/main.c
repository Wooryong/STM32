/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include <stdio.h>
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
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_usart1_rx;
DMA_HandleTypeDef hdma_usart2_rx;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#define BUF_SIZE 100
char DUM1, DUM2; // Dummy Buffer for UART RX Interrupt (1-byte)
char BUF1[BUF_SIZE], BUF2[BUF_SIZE]; // DMA Buffer for UART1, UART2
int Head1 = 0, Head2 = 0;
int Tail1 = 0, Tail2 = 0;

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart == &huart1) // from Bluetooth Module
	{
			BUF1[Tail1++] = DUM1; // Store 1-byte Data to BUF1
			HAL_UART_Transmit(&huart2, &DUM1, 1, 10); // Print the Bluetooth RX Data at Putty
			// HAL_UART_Transmit(&huart2, BUF1 + Tail1 - 1, 1, 10); // DUM1 is stored at BUF[Tail1 - 1]

			if (DUM1 == '\r') // EOL - LF
			{
					CheckCMD(BUF1); Tail1 = 0;
			}
			HAL_UART_Receive_IT(&huart1, &DUM1, 1); // Due to one-time Interrupt > Recursive Expression
	} // if (huart == &huart1)

	else if (huart == &huart2) // from USB UART2 (Type Data on PC Putty)
	{
			BUF2[Tail2++] = DUM2; // Store 1-byte Data to BUF2
			HAL_UART_Transmit(&huart2, &DUM2, 1, 10); // Echo (Show what you typed)
			if (DUM2 == '\r') // Input Char == Enter (CR : 0X0D) >> Next Line
			{
				HAL_UART_Transmit(&huart2, "\n", 1, 10);
				// HAL_UART_Transmit(&huart1, "\n", 1, 10); // "\n" : Due to Pointer Type (uint8_t *pData)

				BUF2[Tail2++] = '\n'; // Append LF for EOL
				HAL_UART_Transmit(&huart1, BUF2, Tail2, 10);
				// Press Enter > TX BUF2 Data (Length : Tail2) through UART1 (Bluetooth)
				// UART1 RX Interrupt > Print BUF2 Data at Putty

				Tail2 = 0; // BUF2 Pointer Reset
			}
			HAL_UART_Receive_IT(&huart2, &DUM2, 1); // Due to one-time Interrupt > Recursive Expression
	} // else if (huart == &huart2)
} // void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)

void TrimEx(char *Dest, char *Source) // ex) source = "___xxx__\t\r\n "
{
	int Head = 0;
	int Tail = strlen(Source) - 1; // Exclude '\0' (Null)

	while(1) // Head Index
	{
			if ( *(Source + Head) == ' ' || *(Source + Head) == '\t' || *(Source + Head) == '\r'  || *(Source + Head) == '\n' ) // Whitespace, Tap, CR, LF
				Head++;
			else
				break;
	}
	while(1) // Tail Index
	{
			if ( *(Source + Tail) == ' ' || *(Source + Tail) == '\t' || *(Source + Tail) == '\r'  || *(Source + Tail) == '\n' ) // Whitespace, Tap, CR, LF
				Tail--;
			else
				break;
	}
	strncpy(Dest, Source + Head, (Tail - Head + 1) ); // strncpy // Consider Zero-base Index
}

char * Trim(char *Source) // ex) source = "___xxx__\t\r\n "
// Memory Dynamic Allocation Version
{
	int Head = 0;
	int Tail = strlen(Source) - 1; // Exclude '\0' (Null)
	/*
	while(1) { // Head Index
			if ( *(Source + Head) == ' ' || *(Source + Head) == '\t' || *(Source + Head) == '\r'  || *(Source + Head) == '\n' ) Head++; // Whitespace, Tap, CR, LF
			else break;
	}
	while(1) { // Tail Index
			if ( *(Source + Tail) == ' ' || *(Source + Tail) == '\t' || *(Source + Tail) == '\r'  || *(Source + Tail) == '\n' ) Tail--; // Whitespace, Tap, CR, LF
			else break;
	}
	*/
	while( *(Source + Head) == ' ' || *(Source + Head) == '\t' || *(Source + Head) == '\r'  || *(Source + Head) == '\n' ) Head++; // Head Index
	while( *(Source + Tail) == ' ' || *(Source + Tail) == '\t' || *(Source + Tail) == '\r'  || *(Source + Tail) == '\n' ) Tail--;

	char * Dest = (char *) malloc(Tail - Head + 1); // malloc(size)
	// void	*malloc(size_t) __malloc_like __result_use_check __alloc_size(1) _NOTHROW;
	strncpy(Dest, Source + Head, (Tail - Head + 1) ); // strncpy // Consider Zero-base Index
	return Dest;
}

void CheckCMD(char *bb) // ex) str = "   LED 1   " ==>  str = "LED 1" << How to process whitespace??
{
	// char str[100];
	// char arg[50];
	// char * bb = Trim(str);
	// TrimEx(str, bb); // Remove whitespace (bb > str) // void Trim(char *dest, char *source)
	char * str = Trim(bb); // char * Trim(char *Source) ~ return dest;
	ToUpper(str);
	// ToLower(str);
	if ( strncmp(str, "LED", 3) == 0 ) // strncmp function : First n-bit of String Compare
	{
		// TrimEx(arg, str + 3);
		char *arg = Trim(str + 3);
		// LED와 숫자 사이 공백 제거 - "LED" 이후부터 Trim
			// if ( str[4] == '1')
			if ( arg[0] == '1')
			{
				HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, 1);
				// HAL_GPIO_WritePin(GPIOx, GPIO_Pin, PinState)
			}
			// else if ( str[4] == '0')
			else if ( arg[0] == '0')
			{
				HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, 0);
				// HAL_GPIO_WritePin(GPIOx, GPIO_Pin, PinState)
			}
	}
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
  MX_DMA_Init();
  MX_USART2_UART_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  ProgramStart("Bluetooth");
  // HAL_UART_Receive_DMA(&huart1, BUF1, BUF_SIZE);
  // HAL_UART_Receive_DMA(&huart2, BUF2, BUF_SIZE);
  // HAL_StatusTypeDef HAL_UART_Receive_DMA(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size)
  // uint8_t *pData : Buffer
  // uint16_t Size : Buffer Size = MAX_BUF(100)
  HAL_UART_Receive_IT(&huart1, &DUM1, 1); // 1-byte UART RX > Interrupt
  HAL_UART_Receive_IT(&huart2, &DUM2, 1); // 1-byte UART RX > Interrupt
  // HAL_StatusTypeDef HAL_UART_Receive_IT(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size)

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
		/*
	  	  char str[100];
		printf( "Typed : ");
		scanf( "%s", str);
		printf( "You Typed : %s \r\n", str);
		printf( "Change Lower-case to Upper-case \r\n");
		ToUpper(str);
		printf( "Changed Version : %s \r\n", str);
		 */

    /* USER CODE END WHILE */

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
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 9600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

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
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Stream5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);
  /* DMA2_Stream2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream2_IRQn);

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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
