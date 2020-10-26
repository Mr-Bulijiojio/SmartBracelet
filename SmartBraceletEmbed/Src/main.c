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
#include "adc.h"
#include "i2c.h"
#include "spi.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ADS1292.h"
#include "Systick_Delay.h"
#include "USART3.h"
#include "USART1.h"

#include "math.h"
#include "ADS1118.h"
#include "ESP8266.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
//#define ESP8266_STA 1
#define ESP8266_AP 1
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */


/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	uint8_t device_id = 0;
	uint32_t ADCValue = 0;
	float32_t temp = 0;
	uint32_t len = 0;
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
  MX_ADC1_Init();
  MX_I2C1_Init();
  MX_SPI1_Init();
  MX_SPI2_Init();
  /* USER CODE BEGIN 2 */
	USART3_Init(912600);
	delay_init(216);
	USART1_Init(912600);

#ifdef ESP8266_AP
	HAL_Delay(500);
	ESP8266_send_cmd((uint8_t*)"AT+CWMODE=2", (uint8_t*)"OK", 400);
	ESP8266_send_cmd((uint8_t*)"AT+CIPMUX=0", (uint8_t*)"OK", 400);
	ESP8266_send_cmd((uint8_t*)"AT+CWSAP=\"SmartBracelet\",\"123456789\",6,4", (uint8_t*)"OK", 400);
	HAL_GPIO_WritePin(GPIOB, LED0_Pin | LED1_Pin,GPIO_PIN_SET);
	HAL_Delay(20000);
	HAL_GPIO_WritePin(GPIOB, LED1_Pin , GPIO_PIN_RESET);
	HAL_Delay(10000);
	HAL_GPIO_WritePin(GPIOB, LED0_Pin, GPIO_PIN_RESET);
	HAL_Delay(10000);

	ESP8266_send_cmd((uint8_t*)"AT+CIPSTART=\"TCP\",\"192.168.4.2\",1334", (uint8_t*)"OK", 400);
	ESP8266_send_cmd((uint8_t*)"AT+CIPMODE=1", (uint8_t*)"OK", 400);
	ESP8266_send_cmd((uint8_t*)"AT+CIPSEND", (uint8_t*)"OK", 400);
#endif	
#ifdef ESP8266_STA

	while (!ESP8266_send_cmd((uint8_t *)"AT", (uint8_t *)"OK", 20))
	{
		HAL_Delay(50);
		HAL_GPIO_TogglePin(GPIOB, LED1_Pin);
	}
	ESP8266_send_cmd((uint8_t *)"AT+CWMODE=1", (uint8_t *)"OK", 400);
	HAL_GPIO_TogglePin(GPIOB, LED1_Pin);
	ESP8266_send_cmd((uint8_t *)"AT+CWJAP=\"KC114\",\"kc114kc114\"", (uint8_t *)"OK", 1500);
//	ESP8266_send_cmd((uint8_t *)"AT+CWJAP=\"yxybaba\",\"123456789\"", (uint8_t *)"OK", 1500);
//	ESP8266_send_cmd((uint8_t *)"AT+CWJAP=\"Yuxuyao\",\"88888888\"", (uint8_t *)"OK", 1500);
	HAL_GPIO_TogglePin(GPIOB, LED1_Pin);
	ESP8266_send_cmd((uint8_t *)"AT+CIFSR", (uint8_t *)"OK", 500);
	HAL_GPIO_TogglePin(GPIOB, LED1_Pin);
	
//	ESP8266_send_cmd((uint8_t *)"AT+CIPSTART=\"TCP\",\"192.168.43.215\",1334", (uint8_t *)"OK", 1000);
//	ESP8266_send_cmd((uint8_t *)"AT+CIPSTART=\"TCP\",\"192.168.137.1\",1334", (uint8_t *)"OK", 1000);
//	ESP8266_send_cmd((uint8_t *)"AT+CIPSTART=\"TCP\",\"192.168.50.31\",8080", (uint8_t *)"OK", 1000);
	ESP8266_send_cmd((uint8_t *)"AT+CIPSTART=\"TCP\",\"192.168.50.236\",1334", (uint8_t *)"OK", 1000);
	HAL_GPIO_TogglePin(GPIOB, LED1_Pin);
	ESP8266_send_cmd((uint8_t *)"AT+CIPMODE=1", (uint8_t *)"OK", 400);
	HAL_GPIO_TogglePin(GPIOB, LED1_Pin);
	ESP8266_send_cmd((uint8_t *)"AT+CIPSEND", (uint8_t *)"OK", 400);
	HAL_GPIO_TogglePin(GPIOB, LED1_Pin);
 
#endif // ESP8266_STA
	
	HAL_Delay(500);
	/*Initialize ADS1118*/
	ADS1118_Init();
	
	printf("waiting for ads1292r\n");
	HAL_Delay(500);
	HAL_Delay(500);
	HAL_Delay(500);
	/*Initialize ADS12929R IO*/
	ADS1292R_Init();
	HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);
	HAL_Delay(500);
	
	/*PowerOn ADS12929R If the initialization is not successful LED0 will blink*/
	device_id = ADS1292R_PowerOnInit();
	/*Enable interrupt data transfer*/
	ADS1292R_Work();
	
	
//---------------------
	
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		/*transmit MPU6050 data to ESP8266*/
	  	if (USART3_RX_STA & 0x8000)
	  	{
		  	HAL_NVIC_DisableIRQ(EXTI3_IRQn);
	  		len = USART3_RX_STA & 0x3fff;
	  		HAL_UART_Transmit(&UART1_Handler, (uint8_t*)USART3_RX_BUF, len, 1000);
		  	HAL_UART_Transmit(&UART1_Handler, (uint8_t*)"\r\n", 2, 100);
	  		while(__HAL_UART_GET_FLAG(&UART1_Handler, UART_FLAG_TC) != SET);
	  		USART3_RX_STA = 0;
		  	HAL_NVIC_SetPriority(EXTI3_IRQn, 0, 0);
		  	HAL_NVIC_EnableIRQ(EXTI3_IRQn);
	  	}
		/*ADS1118 Start a single conversion*/
		//--------------------------------
	  	HAL_Delay(1);
	  	ADS1118_Read(CONF);
		//--------------------------------	
		
		HAL_GPIO_TogglePin(GPIOB, LED0_Pin | LED1_Pin);
		HAL_Delay(100);

		/*Read temperature data from ADS1118*/
		//--------------------------------
		ADS1118_Read(0);
		//--------------------------------

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
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Configure the main internal regulator output voltage 
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 432;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
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
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_I2C1;
  PeriphClkInitStruct.I2c1ClockSelection = RCC_I2C1CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_Pin == GPIO_PIN_3)
	{
		ADS1292R_ReadData();
	}
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
