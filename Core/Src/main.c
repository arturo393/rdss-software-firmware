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
#include "led.h"
#include "uart1.h"
#include "rs485.h"
#include "module.h"
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#define HS16_CLK 16000000
#define BAUD_RATE 115200

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

CRC_HandleTypeDef hcrc;

SPI_HandleTypeDef hspi1;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_CRC_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
UART1_t *uart1_ptr;
Vlad_t *vlad_ptr;
RS485_t *rs485_ptr;
SX1278_t *lora_ptr;

void USART1_IRQHandler(void) {
	if (uart1_ptr->len >= RX_BUFFLEN) {
		cleanRx(uart1_ptr);
		uart1_ptr->len = 0;
	}
	uart1_ptr->rx[uart1_ptr->len++] = readRxReg();
	if(uart1_ptr->rx[uart1_ptr->len] == LTEL_END_MARK)
		uart1_ptr->isReady = true;
}

bool TX_MODE;
bool RX_MODE;

void modeCmdUpdate(const UART1_t *uart1, SX1278_t *loRa) {
	unsigned long receiveValue;
	receiveValue = 0;
	receiveValue = uart1->rx[4] << 8;
	receiveValue |= uart1->rx[5];
	if (receiveValue == 0) {
		RX_MODE = true;
		TX_MODE = false;
		loRa->status = UNKNOW;
	}
	if (receiveValue == 1) {
		TX_MODE = true;
		RX_MODE = false;
		loRa->status = UNKNOW;
	}
}

void modeRs485Update(const UART1_t *uart1, RS485_t *rs485, SX1278_t *loRa) {
	switch (rs485->cmd) {
	case QUERY_PARAMETERS_VLAD: //cmd = 11
		break;
	case SET_VLAD_MODE: //cmd = 12
		modeCmdUpdate(uart1, loRa);
		rs485->cmd = NONE;
		break;
	default:
		rs485->cmd = NONE;
		break;
	}
}

void printStatus(UART1_t *u1, RS485_t *rs485) {

	char *str = (char*) u1->tx;
	switch (rs485->status) {
	case CRC_ERROR:
		u1->len = sprintf(str,
				"CRC missmatch: 0x%04x received , 0x%04x calculated\r\n",
				rs485->crcReceived, rs485->crcCalculated);
		writeTx(u1);
		u1->len = 0;

		break;
	case WRONG_MODULE_ID:
		u1->len = sprintf(str, "ID mismatch - ID %d and ID received %d \r\n",
				rs485->id, rs485->idReceived);
		writeTx(u1);
		u1->len = 0;
		break;
	case NOT_VALID_FRAME:
		u1->len = sprintf(str, "Not valid start byte: %02x \r\n",
				rs485->buffer[0]);
		writeTx(u1);
		u1->len = 0;
		break;
	case DATA_OK:

		u1->len = sprintf(str,
				"Validation ok: ID %02x Cmd %02x Bytes %d Data \r\n",
				rs485->buffer[2], rs485->buffer[3], rs485->buffer[5]);
		writeTx(u1);
		for (int i = DATA_START_INDEX; i < rs485->buffer[5]; i++) {
			if (i > 250)
				break;
			u1->len = sprintf(str, "%02X", rs485->buffer[i]);
			writeTx(u1);

		}
		writeTxReg('\n');
		break;
	case WAITING:
		u1->len = sprintf(str, "Waiting for new data\r\n");
		writeTx(u1);
		u1->len = 0;
		break;
	case BROADCAST:
		u1->len = sprintf(str, "Send uart data to loRa ID: %d\r\n",rs485->idReceived);
		writeTx(u1);
		u1->len = 0;
		break;
	default:
		break;

	}

}

void printLoRaStatus(UART1_t *u1, SX1278_t *loRa) {

	char *str = (char*) u1->tx;
	switch (loRa->status) {
	case TX_TIMEOUT:
		u1->len = sprintf(str, "Transmission Fail: %d seconds Timeout\r\n",
				TX_TIMEOUT / 1000);
		writeTx(u1);
		break;
	case TX_DONE:
		u1->len = sprintf(str, "Transmission Done: %lu ms %d bytes\r\n",
				loRa->lastTxTime, loRa->len);
		writeTx(u1);
		for (int i = 0; i < loRa->len; i++) {
			u1->len = sprintf(str, "%02X", loRa->buffer[i]);
			writeTx(u1);
		}
		writeTxReg('\n');
		break;
	case TX_READY:
		u1->len = sprintf(str, "Master Mode\r\n");
		writeTx(u1);
		break;
	case RX_DONE:
		u1->len = sprintf(str, "Reception Done: %d bytes\r\n", loRa->len);
		writeTx(u1);
		for (int i = 0; i < loRa->len; i++) {
			u1->len = sprintf(str, "%02X", loRa->buffer[i]);
			writeTx(u1);
		}
		writeTxReg('\n');
		break;
	case RX_READY:
		u1->len = sprintf(str, "Slave Mode\r\n");
		writeTx(u1);
		break;
	case CRC_ERROR_ACTIVATION:
		u1->len = sprintf(str, "Reception Fail: Crc error activation\r\n");
		writeTx(u1);
		break;
	default:
		break;
	}
	u1->len = 0;
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	LED_t led;
	RS485_t rs485;
	UART1_t u1;
	Vlad_t vlad;
	SX1278_t loRa;

	lora_ptr = &loRa;
	uart1_ptr = &u1;
	vlad_ptr = &vlad;
	rs485_ptr = &rs485;
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
//  MX_USART1_UART_Init();
  MX_CRC_Init();
  /* USER CODE BEGIN 2 */
	vladInit(VLAD, ID2, &vlad);
	rs485Init(&rs485);
	rs485.id = ID2;
	uart1Init(HS16_CLK, BAUD_RATE, &u1);
	ledInit(&led);

	loRa.spi = &hspi1;
	HAL_GPIO_WritePin(LORA_NSS_GPIO_Port, LORA_NSS_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LORA_RST_GPIO_Port, LORA_RST_Pin, GPIO_PIN_SET);
	loRa.operatingMode = readRegister(&hspi1, LR_RegOpMode);
	loRa.status = UNKNOW;
	loRa.len = 0;
	TX_MODE = false;
	RX_MODE = true;
	initLoRaParameters(&loRa, SLAVE_RECEIVER);
	writeLoRaParameters(&loRa);

	u1.len = sprintf((char*) u1.tx, "\nSlave init completed\n");
	writeTx(&u1);

	//initialize LoRa module

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1) {

		if (u1.isReady) {
			rs485.len = u1.len;
			memcpy(rs485.buffer, u1.rx, u1.len);
			cleanRx(&u1);
		}

		if (loRa.len > 0) {
			rs485.len = loRa.len;
			memcpy(rs485.buffer, loRa.buffer, loRa.len);
			memset(loRa.buffer, 0, sizeof(loRa.buffer));
			loRa.len = 0;
		}

		if (rs485.len > 0) {
			checkBuffer(&rs485);
			printStatus(&u1, &rs485);

			switch (rs485.status) {
			case DATA_OK:
				rs485.cmd = rs485.buffer[3];
				rs485.status = WAITING;
				break;
			case WAITING:
				rs485.status = WAITING;
			default:
				memset(rs485.buffer, 0, sizeof(rs485.buffer));
				rs485.len = 0;
				rs485.cmd = NONE;
				rs485.status = WAITING;
				break;
			}

			switch (rs485.cmd) {
			case QUERY_PARAMETERS_VLAD:

				if (rs485.buffer[DATA_LENGHT2_INDEX] != 0)
					break;

				loRa.len = 21;
				encodeVLAD(loRa.buffer);
				updateMode(&loRa, SLAVE_SENDER);
				printLoRaStatus(&u1, &loRa);
				setTxFifoData(&loRa);
				updateLoraLowFreq(&loRa, TX);
				waitForTxEnd(&loRa);
				printLoRaStatus(&u1, &loRa);
				memset(loRa.buffer, 0, sizeof(loRa.buffer));
				loRa.len = 0;
				loRa.status = UNKNOW;
				rs485.cmd = NONE;
				memset(rs485.buffer, 0, sizeof(rs485.buffer));
				rs485.len = 0;
				break;
			default:
				printStatus(&u1, &rs485);
				break;
			}

			memset(rs485.buffer, 0, sizeof(rs485.buffer));
			rs485.len = 0;
			rs485.cmd = NONE;
			rs485.status = WAITING;

		}

		if (RX_MODE) {
			if (loRa.operatingMode != RX_CONTINUOUS) {
				updateMode(&loRa, SLAVE_RECEIVER);
				setRxFifoAddr(&loRa);
				updateLoraLowFreq(&loRa, RX_CONTINUOUS);
			}
			clearMemForRx(&loRa);
			GPIO_PinState bussy = HAL_GPIO_ReadPin(LORA_BUSSY_GPIO_Port,
			LORA_BUSSY_Pin);
			if (bussy == GPIO_PIN_SET)
				if (crcErrorActivation(&loRa) != 1)
					getRxFifoData(&loRa);

			if (loRa.status == RX_DONE) {
				setRxFifoAddr(&loRa);
				updateLoraLowFreq(&loRa, RX_CONTINUOUS);
				readOperatingMode(&loRa);
				printLoRaStatus(&u1, &loRa);
				loRa.status = RX_READY;
			}

		}
	}
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

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
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

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
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc1.Init.SamplingTimeCommon1 = ADC_SAMPLETIME_1CYCLE_5;
  hadc1.Init.SamplingTimeCommon2 = ADC_SAMPLETIME_1CYCLE_5;
  hadc1.Init.OversamplingMode = DISABLE;
  hadc1.Init.TriggerFrequencyMode = ADC_TRIGGER_FREQ_HIGH;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLINGTIME_COMMON_1;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief CRC Initialization Function
  * @param None
  * @retval None
  */
static void MX_CRC_Init(void)
{

  /* USER CODE BEGIN CRC_Init 0 */

  /* USER CODE END CRC_Init 0 */

  /* USER CODE BEGIN CRC_Init 1 */

  /* USER CODE END CRC_Init 1 */
  hcrc.Instance = CRC;
  hcrc.Init.DefaultPolynomialUse = DEFAULT_POLYNOMIAL_ENABLE;
  hcrc.Init.DefaultInitValueUse = DEFAULT_INIT_VALUE_ENABLE;
  hcrc.Init.InputDataInversionMode = CRC_INPUTDATA_INVERSION_NONE;
  hcrc.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_DISABLE;
  hcrc.InputDataFormat = CRC_INPUTDATA_FORMAT_BYTES;
  if (HAL_CRC_Init(&hcrc) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CRC_Init 2 */

  /* USER CODE END CRC_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

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
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

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
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LORA_NSS_Pin|LORA_RST_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LORA_DIO1_GPIO_Port, LORA_DIO1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LORA_BUSSY_GPIO_Port, LORA_BUSSY_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : LORA_NSS_Pin LORA_RST_Pin */
  GPIO_InitStruct.Pin = LORA_NSS_Pin|LORA_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : LORA_DIO3_Pin */
  GPIO_InitStruct.Pin = LORA_DIO3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(LORA_DIO3_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LORA_DIO1_Pin */
  GPIO_InitStruct.Pin = LORA_DIO1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LORA_DIO1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LORA_BUSSY_Pin */
  GPIO_InitStruct.Pin = LORA_BUSSY_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LORA_BUSSY_GPIO_Port, &GPIO_InitStruct);

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
