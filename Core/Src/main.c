/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2022 STMicroelectronics.
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
#include "SX1278.h"
#include "gpio.h"
#include "uart.h"
#include "spi.h"
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
ADC_HandleTypeDef hadc1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
//static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
//static void MX_SPI1_Init(void);
//static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

uint8_t lora_read_reg(uint8_t address) {
	uint8_t rec;
	HAL_GPIO_WritePin(GPIOB, LORA_NSS_Pin, GPIO_PIN_RESET);  // pull the pin low
	HAL_Delay(1);
	HAL_SPI_Transmit(&hspi1, &address, 1, 100);  // send address
	HAL_SPI_Receive(&hspi1, &rec, 1, 100);  // receive16 bytes data
	HAL_Delay(1);
	HAL_GPIO_WritePin(GPIOB, LORA_NSS_Pin, GPIO_PIN_SET);  // pull the pin high
	return rec;
}

SX1278_hw_t SX1278_hw;
SX1278_t SX1278;

uint8_t master;
uint8_t ret;

char buffer[256];
uint8_t response[] = { 0x7e, 0x05, 0x04, 0x11, 0x00, 0x0c, 0x89, 0x00, 0xf7,
		0xff0, 0xac, 0xff, 0xeb, 0x00, 0x1d, 0x00, 0x020, 0xec, 0xb3, 0xde,
		0x7f };
uint8_t str[300];
int message;
int message_length;
int word;

uint8_t VLAD_ID = 0x00;
/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
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
	MX_ADC1_Init();
	MX_SPI1_Init();
	MX_USART1_UART_Init();
	/* USER CODE BEGIN 2 */

	master = 0;
	word = 0;
	VLAD_ID = 0x04;
	uint8_t is_rx_mode = 0x00;
	//initialize LoRa module
	SX1278_hw.dio0.port = BUSSY_GPIO_Port;
	SX1278_hw.dio0.pin = BUSSY_Pin;
	SX1278_hw.nss.port = LORA_NSS_GPIO_Port;
	SX1278_hw.nss.pin = LORA_NSS_Pin;
	SX1278_hw.reset.port = LORA_RST_GPIO_Port;
	SX1278_hw.reset.pin = LORA_RST_Pin;
	SX1278_hw.spi = &hspi1;
	SX1278.hw = &SX1278_hw;

	uart_send("Configuring LoRa module\r\n");
	SX1278_init(&SX1278, 150000000, SX1278_POWER_17DBM, SX1278_LORA_SF_10,
	SX1278_LORA_BW_62_5KHZ, SX1278_LORA_CR_4_6, SX1278_LORA_CRC_DIS, 10);
	uart_send("Done configuring LoRaModule\r\n");

	if (master == 1) {
		ret = SX1278_LoRaEntryTx(&SX1278, 16, 2000);
		HAL_GPIO_WritePin(DIO1_GPIO_Port, DIO1_Pin, GPIO_PIN_SET);
	} else {
		ret = SX1278_LoRaEntryRx(&SX1278, 9, 2000);
		if (ret == 1)
			uart_send("Rx Mode\r\n");
		else
			uart_send("TimeOut!\r\n");
	}

	int rssi_lora = 0;
	int rssi = 0;
	int version;

	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		if (master == 1) {

			uart_send("Sending package...");

			SX1278_init(&SX1278, 170000000, SX1278_POWER_17DBM,
					SX1278_LORA_SF_10,
					SX1278_LORA_BW_62_5KHZ, SX1278_LORA_CR_4_6,
					SX1278_LORA_CRC_DIS,10);

			ret = SX1278_LoRaEntryTx(&SX1278, sizeof(response), 2000);
			if (ret == 1) {
				uart_send("EntryTx Ok!... \r\n");
			} else {
				uart_send("EntryTx failed, timeout reset!\r\n");
			}

			for (uint8_t i = 0; i < sizeof(response); i++) {
				sprintf(str, "%02x", response[i]);
				HAL_UART_Transmit(&huart1, str, 2, 50);
			}
			HAL_UART_Transmit(&huart1,"\r\n", 2, 50);


			ret = SX1278_LoRaTxPacket(&SX1278, (uint8_t*) response,
					sizeof(response), 1000);

			if (ret >= 1) {
				sprintf(str, "%d bytes sent... \r\n", ret);
				uart_send(str);
			} else {
				uart_send("Sent Fail, timeout reset!\r\n");
			}
			master = 0;

		} else {

			if (!is_rx_mode) {
				SX1278_init(&SX1278, 150000000, SX1278_POWER_17DBM,
						SX1278_LORA_SF_10,
						SX1278_LORA_BW_62_5KHZ, SX1278_LORA_CR_4_6,
						SX1278_LORA_CRC_DIS, 10);
				ret = SX1278_LoRaEntryRx(&SX1278, 9, 2000);
				if (ret == 1) {
					uart_send("Rx Mode\r\n");
					is_rx_mode = 0x01;
				} else
					uart_send("TimeOut!\r\n");
			}

			if (SX1278_hw_GetDIO0(SX1278.hw)) {
				uart_send("Slave Receiving package...");
				ret = SX1278_LoRaRxPacket(&SX1278);
				sprintf(str, "Received: %d bytes...", ret);
				uart_send(str);
				if (ret > 0) {
					SX1278_read(&SX1278, (uint8_t*) buffer, ret);
					sprintf(str, "Content: ", ret);
					uart_send(str);
					uart_send("\r\n");
					for (uint8_t i = 0; i < ret; i++) {
						sprintf(str, "%02x ", buffer[i]);
						HAL_UART_Transmit(&huart1, str, 2, 50);

					}

					if (buffer[0] == 0x7e && buffer[2] == VLAD_ID) {
						master = 1;
						is_rx_mode = 0x00;
					}
				}

				uart_send("\r\n");

			}

		}

		//	rssi_lora = SX1278_RSSI_LoRa(&SX1278);
		//	rssi = SX1278_RSSI(&SX1278);
		//	HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);
		//	version = SX1278_SPIRead(&SX1278, REG_LR_VERSION);
		//	sprintf(str,"LoRa Version %d...\r\n", version);
		//	uart_send(str);

		//	HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);

		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	/** Configure the main internal regulator output voltage
	 */
	HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK) {
		Error_Handler();
	}
}

/**
 * @brief ADC1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_ADC1_Init(void) {

	/* USER CODE BEGIN ADC1_Init 0 */

	/* USER CODE END ADC1_Init 0 */

	ADC_ChannelConfTypeDef sConfig = { 0 };

	/* USER CODE BEGIN ADC1_Init 1 */

	/* USER CODE END ADC1_Init 1 */

	/** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
	 */
	hadc1.Instance = ADC1;
	hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
	hadc1.Init.Resolution = ADC_RESOLUTION_12B;
	hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
	hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
	hadc1.Init.LowPowerAutoWait = DISABLE;
	hadc1.Init.LowPowerAutoPowerOff = DISABLE;
	hadc1.Init.ContinuousConvMode = DISABLE;
	hadc1.Init.NbrOfConversion = 1;
	hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
	hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
	hadc1.Init.DMAContinuousRequests = DISABLE;
	hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
	hadc1.Init.SamplingTimeCommon1 = ADC_SAMPLETIME_1CYCLE_5;
	hadc1.Init.SamplingTimeCommon2 = ADC_SAMPLETIME_1CYCLE_5;
	hadc1.Init.OversamplingMode = DISABLE;
	hadc1.Init.TriggerFrequencyMode = ADC_TRIGGER_FREQ_HIGH;
	if (HAL_ADC_Init(&hadc1) != HAL_OK) {
		Error_Handler();
	}

	/** Configure Regular Channel
	 */
	sConfig.Channel = ADC_CHANNEL_0;
	sConfig.Rank = ADC_REGULAR_RANK_1;
	sConfig.SamplingTime = ADC_SAMPLINGTIME_COMMON_1;
	if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN ADC1_Init 2 */

	/* USER CODE END ADC1_Init 2 */

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
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
