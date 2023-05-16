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

I2C_HandleTypeDef hi2c1;

SPI_HandleTypeDef hspi1;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_ADC1_Init(void);
static void MX_CRC_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
UART1_t *uart1_ptr;
RDSS_t *rs485_ptr;
Vlad_t *vlad_ptr;
SX1278_t *lora_ptr;

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi);
void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi);
uint8_t decodeVLAD(Vlad_t *vlad, uint8_t *frame);
void print_parameters(UART1_t *u1, Vlad_t vlad);
void printStatus(UART1_t *u1, RDSS_t *rdss);
void printLoRaStatus(UART1_t *u1, SX1278_t *loRa);
void parseLoRaMaster(RDSS_t *rs485, SX1278_t *loRa);
void parseUartMaster(UART1_t *u1, RDSS_t *rs485);
void sendQuery(RDSS_t *rs485, SX1278_t *loRa, UART1_t *u1);
uint8_t processReceivedLoraCommand(RDSS_t *rdss, SX1278_t *loRa, Vlad_t *vlad, UART1_t *u1);
void processReceivedUartCommand(Vlad_t *vlad, UART1_t *u1, RDSS_t *rdss, SX1278_t *loRa);
GPIO_PinState processLoRaSlaveReceiver(SX1278_t *loRa, UART1_t *u1);
void processLoraReceiveState(UART1_t *uartHandle, RDSS_t *rdssHandle,
		Vlad_t *vladHandle, SX1278_t *loRaHandle);
void processLoraSendState(UART1_t *uartHandle, RDSS_t *rdssHandle,
		SX1278_t *loRaHandle);
void processMasterReceiverMode(UART1_t *uartHandle, SX1278_t *loRaHandle);
/*void USART1_IRQHandler(void) {
 uart1_read_to_frame(uart1_ptr);
 }*/

/* In the interrupt handler, read the received data from the UART1 data register */
/* Enable UART1 interrupt */
uint8_t rxData;

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
	/* USER CODE BEGIN 1 */
	LED_t led;
	RDSS_t *rdss;
	UART1_t u1;
	Vlad_t *vlad;
	SX1278_t *loRa;

	uart1_ptr = &u1;
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
	MX_I2C1_Init();
	MX_SPI1_Init();
	MX_USART1_UART_Init();
	MX_USART2_UART_Init();
	MX_USART3_UART_Init();
	MX_ADC1_Init();
	MX_CRC_Init();
	/* USER CODE BEGIN 2 */
	vlad = vladInit(0);
	ledInit(&led);
	rdss = rdssInit(0);
	loRa = loRaInit(&hspi1, MASTER_RECEIVER);
	printStatus(&u1, rdss);
	printLoRaStatus(&u1, loRa);

	HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(USART1_IRQn);
	HAL_UART_Receive_IT(&huart1, &rxData, 1);

	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {

		if (rdss->status == LORA_RECEIVE) {
			processLoraReceiveState(&u1, rdss, vlad, loRa);
		} else if (rdss->status == LORA_SEND) {
			processLoraSendState(&u1, rdss, loRa);
		} else if (u1.isReady) {
			parseUartMaster(&u1, rdss);
		} else if (loRa->len > 0) {
			parseLoRaMaster(rdss, loRa);
		} else if (loRa->mode == MASTER_RECEIVER) {
			processMasterReceiverMode(&u1, loRa);
		} else if (loRa->mode == MASTER_SENDER) {
			RX_MODE_OFF_LED();
			TX_MODE_ON_LED();
		}
		enableLedKeepAlive(&led);
	}
	/* USER CODE END WHILE */

	/* USER CODE BEGIN 3 */

	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };
	RCC_PeriphCLKInitTypeDef PeriphClkInit = { 0 };

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL2;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK) {
		Error_Handler();
	}
	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
	PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV2;
	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) {
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

	/** Common config
	 */
	hadc1.Instance = ADC1;
	hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
	hadc1.Init.ContinuousConvMode = DISABLE;
	hadc1.Init.DiscontinuousConvMode = DISABLE;
	hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
	hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	hadc1.Init.NbrOfConversion = 1;
	if (HAL_ADC_Init(&hadc1) != HAL_OK) {
		Error_Handler();
	}

	/** Configure Regular Channel
	 */
	sConfig.Channel = ADC_CHANNEL_0;
	sConfig.Rank = ADC_REGULAR_RANK_1;
	sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
	if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
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
static void MX_CRC_Init(void) {

	/* USER CODE BEGIN CRC_Init 0 */

	/* USER CODE END CRC_Init 0 */

	/* USER CODE BEGIN CRC_Init 1 */

	/* USER CODE END CRC_Init 1 */
	hcrc.Instance = CRC;
	if (HAL_CRC_Init(&hcrc) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN CRC_Init 2 */

	/* USER CODE END CRC_Init 2 */

}

/**
 * @brief I2C1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_I2C1_Init(void) {

	/* USER CODE BEGIN I2C1_Init 0 */

	/* USER CODE END I2C1_Init 0 */

	/* USER CODE BEGIN I2C1_Init 1 */

	/* USER CODE END I2C1_Init 1 */
	hi2c1.Instance = I2C1;
	hi2c1.Init.ClockSpeed = 100000;
	hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
	hi2c1.Init.OwnAddress1 = 0;
	hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	hi2c1.Init.OwnAddress2 = 0;
	hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
	if (HAL_I2C_Init(&hi2c1) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN I2C1_Init 2 */

	/* USER CODE END I2C1_Init 2 */

}

/**
 * @brief SPI1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_SPI1_Init(void) {

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
	hspi1.Init.CRCPolynomial = 10;
	if (HAL_SPI_Init(&hspi1) != HAL_OK) {
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
static void MX_USART1_UART_Init(void) {

	/* USER CODE BEGIN USART1_Init 0 */

	/* USER CODE END USART1_Init 0 */

	/* USER CODE BEGIN USART1_Init 1 */

	/* USER CODE END USART1_Init 1 */
	huart1.Instance = USART1;
	huart1.Init.BaudRate = 19200;
	huart1.Init.WordLength = UART_WORDLENGTH_8B;
	huart1.Init.StopBits = UART_STOPBITS_1;
	huart1.Init.Parity = UART_PARITY_NONE;
	huart1.Init.Mode = UART_MODE_TX_RX;
	huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart1.Init.OverSampling = UART_OVERSAMPLING_16;
	if (HAL_UART_Init(&huart1) != HAL_OK) {
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
static void MX_USART2_UART_Init(void) {

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
	if (HAL_UART_Init(&huart2) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN USART2_Init 2 */

	/* USER CODE END USART2_Init 2 */

}

/**
 * @brief USART3 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART3_UART_Init(void) {

	/* USER CODE BEGIN USART3_Init 0 */

	/* USER CODE END USART3_Init 0 */

	/* USER CODE BEGIN USART3_Init 1 */

	/* USER CODE END USART3_Init 1 */
	huart3.Instance = USART3;
	huart3.Init.BaudRate = 115200;
	huart3.Init.WordLength = UART_WORDLENGTH_8B;
	huart3.Init.StopBits = UART_STOPBITS_1;
	huart3.Init.Parity = UART_PARITY_NONE;
	huart3.Init.Mode = UART_MODE_TX_RX;
	huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart3.Init.OverSampling = UART_OVERSAMPLING_16;
	if (HAL_UART_Init(&huart3) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN USART3_Init 2 */

	/* USER CODE END USART3_Init 2 */

}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void) {
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	/* USER CODE BEGIN MX_GPIO_Init_1 */
	/* USER CODE END MX_GPIO_Init_1 */

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOB,
			LORA_NSS_Pin | LORA_RST_Pin | LORA_DIO3_Pin | LORA_DIO1_Pin
					| LORA_BUSSY_Pin | LED2_Pin | LED1_Pin | RS485_DE_Pin
					| BUZZER_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pins : LORA_NSS_Pin LORA_RST_Pin LORA_DIO3_Pin LORA_DIO1_Pin
	 LORA_BUSSY_Pin LED2_Pin LED1_Pin RS485_DE_Pin
	 BUZZER_Pin */
	GPIO_InitStruct.Pin = LORA_NSS_Pin | LORA_RST_Pin | LORA_DIO3_Pin
			| LORA_DIO1_Pin | LORA_BUSSY_Pin | LED2_Pin | LED1_Pin
			| RS485_DE_Pin | BUZZER_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/*Configure GPIO pin : MODE_Pin */
	GPIO_InitStruct.Pin = MODE_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(MODE_GPIO_Port, &GPIO_InitStruct);

	/* USER CODE BEGIN MX_GPIO_Init_2 */
	/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	/* Read received data from UART1 */
	if (uart1_ptr->rxLen >= RX_BUFFLEN) {
		cleanRx(uart1_ptr);
		uart1_ptr->rxLen = 0;
	}
	HAL_UART_Receive_IT(&huart1, &rxData, 1);
	uart1_ptr->rx[uart1_ptr->rxLen++] = rxData;
	if (rxData == LTEL_END_MARK)
		uart1_ptr->isReady = true;
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi) {
	printf("SPI TX Done .. Do Something ...");
}

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi) {
	printf("SPI RX Done .. Do Something ...");
}

uint8_t decodeVLAD(Vlad_t *vlad, uint8_t *frame) {
	uint8_t index = 0;
	uint8_t vladRev23Id = frame[index++];
	vlad->state = frame[index++];
	vlad->lineVoltagereal = (((uint16_t) frame[index++])
			| ((uint16_t) (frame[index++] << 8))) / 10.0;
	vlad->lineCurrentReal = (((uint16_t) frame[index++])
			| ((uint16_t) (frame[index++] << 8))) / 1000.0;
	vlad->tone_level = (((uint16_t) frame[index++])
			| ((uint16_t) (frame[index++] << 8)));
	vlad->level152m_real = frame[index++] / 10;
	vlad->level152m_real = frame[index++];
	vlad->agc172m_real = frame[index++] / 10;
	vlad->level172m_real = frame[index++];

	return index;
}

void print_parameters(UART1_t *u1, Vlad_t vlad) {
	if (!u1->debug)
		return;

	char *buff = (char*) u1->tx;
	u1->tx_len = (uint8_t) sprintf(buff, "vin %d [V]\r\n", vlad.vin);
	writeTx(u1);
	u1->tx_len = (uint8_t) sprintf(buff, "current real %d [A]\r\n",
			(uint8_t) vlad.lineCurrentReal);
	writeTx(u1);
	u1->tx_len = (uint8_t) sprintf(buff, "tone level %d[dBm]\r\n",
			vlad.tone_level);
	writeTx(u1);
	u1->tx_len = (uint8_t) sprintf(buff, "current %d[A]\r\n", vlad.current);
	writeTx(u1);
	u1->tx_len = (uint8_t) sprintf(buff, "agc150m %d[dBm]\r\n", vlad.agc152m);
	writeTx(u1);
	u1->tx_len = (uint8_t) sprintf(buff, "level150m %d[dBm]\r\n",
			vlad.level152m);
	writeTx(u1);
	u1->tx_len = (uint8_t) sprintf(buff, "agc170m %d[dBm]\r\n", vlad.agc172m);
	writeTx(u1);
	u1->tx_len = (uint8_t) sprintf(buff, "level170m %d[dBm]\r\n",
			vlad.level172m);
	writeTx(u1);
	cleanTx(u1);
}

void printStatus(UART1_t *u1, RDSS_t *rdss) {

	if (!u1->debug)
		return;

	char *str = (char*) u1->tx;
	uint8_t i = 0;
	switch (rdss->status) {
	case CRC_ERROR:
		checkValidCrc(rdss->buffer, rdss->len);
		u1->tx_len = (uint8_t) sprintf(str, "CRC mismatch:\r\n");
		writeTx(u1);
		u1->tx_len = 0;
		break;
	case WRONG_MODULE_ID:
		u1->tx_len = (uint8_t) sprintf(str,
				"ID mismatch - ID %d and ID received %d \r\n", rdss->id,
				rdss->idReceived);
		writeTx(u1);
		u1->tx_len = 0;
		break;
	case NOT_VALID_FRAME:
		u1->tx_len = (uint8_t) sprintf(str, "Not valid start byte \r\n");
		writeTx(u1);
		u1->tx_len = 0;
		break;
	case DATA_OK:
		u1->tx_len = (uint8_t) sprintf(str,
				"Validation ok: ID %02x Cmd %02x Bytes %d Data \r\n",
				rdss->buffer[2], rdss->buffer[3], rdss->buffer[5]);
		writeTx(u1);
		for (int i = DATA_START_INDEX; i < rdss->buffer[5]; i++) {
			if (i > 250)
				break;
			u1->tx_len = (uint8_t) sprintf(str, "%02X", rdss->buffer[i]);
			writeTx(u1);
		}
		writeTxReg('\n');
		break;
	case WAITING:
		u1->tx_len = (uint8_t) sprintf(str, "Waiting for new data\r\n");
		writeTx(u1);
		u1->tx_len = 0;
		break;
	case LORA_SEND:
		u1->tx_len = (uint8_t) sprintf(str, "Send uart data to loRa ID: %d\r\n",
				rdss->idReceived);
		writeTx(u1);
		u1->tx_len = 0;
		break;
	case LORA_RECEIVE:
		u1->tx_len = (uint8_t) sprintf(str,
				"Validation ok: ID %02x Cmd %02x Bytes %d Data \r\n",
				rdss->buffer[2], rdss->buffer[3], rdss->buffer[5]);
		writeTx(u1);
		for (i = DATA_START_INDEX; i < rdss->buffer[5]; i++) {
			if (i > 250)
				break;
			u1->tx_len = (uint8_t) sprintf(str, "%02X", rdss->buffer[i]);
			writeTx(u1);
		}
		if (i > DATA_START_INDEX)
			writeTxReg('\n');
		u1->tx_len = 0;
		break;
	case UART_SEND:
		u1->tx_len = (uint8_t) sprintf(str, "Reply vlad data: %d\r\n",
				rdss->idReceived);
		writeTx(u1);
		for (i = 0; i < rdss->len; i++) {
			if (i > 250)
				break;
			u1->tx_len = (uint8_t) sprintf(str, "%02X", rdss->buffer[i]);
			writeTx(u1);
		}
		if (i > 0)
			writeTxReg('\n');
		u1->tx_len = 0;
		break;
	case UART_VALID:
		u1->tx_len = (uint8_t) sprintf(str,
				"Validation ok: ID %02x Cmd %02x Bytes %d Data \r\n",
				rdss->buffer[2], rdss->buffer[3], rdss->buffer[5]);
		writeTx(u1);
		for (int i = DATA_START_INDEX; i < rdss->buffer[5]; i++) {
			if (i > 250)
				break;
			u1->tx_len = (uint8_t) sprintf(str, "%02X", rdss->buffer[i]);
			writeTx(u1);
		}
		if (i > DATA_START_INDEX)
			writeTxReg('\n');
		u1->tx_len = 0;
		break;
	default:
		break;

	}
	cleanTx(u1);

}
void printLoRaStatus(UART1_t *u1, SX1278_t *loRa) {
	if (!u1->debug) {
		return;
	}

	char *str = (char*) u1->tx;

	switch (loRa->status) {
	case TX_TIMEOUT:
		u1->tx_len = (uint8_t) sprintf(str,
				"Transmission Fail: %d seconds Timeout\r\n", TX_TIMEOUT / 1000);
		writeTx(u1);
		break;
	case TX_DONE:
		u1->tx_len = (uint8_t) sprintf(str, "Transmission Done: %lu ms\r\n",
				loRa->lastTxTime);
		writeTx(u1);
		break;
	case TX_BUFFER_READY:
		u1->tx_len = (uint8_t) sprintf(str,
				"Transmission Buffer: %d bytes data \r\n", loRa->len);
		writeTx(u1);
		for (int i = 0; i < loRa->len; i++) {
			u1->tx_len = (uint8_t) sprintf(str, "%02X", loRa->buffer[i]);
			writeTx(u1);
		}
		writeTxReg('\n');
		break;
	case TX_MODE:
		u1->tx_len = (uint8_t) sprintf(str, "%s Mode\r\n",
				(loRa->mode == MASTER_SENDER) ? "Master Sender" :
				(loRa->mode == SLAVE_SENDER) ? "Slave Sender" : "Unknown");
		writeTx(u1);
		break;
	case RX_DONE:
		u1->tx_len = (uint8_t) sprintf(str, "Reception Done: %d bytes\r\n",
				loRa->len);
		writeTx(u1);
		for (int i = 0; i < loRa->len; i++) {
			u1->tx_len = (uint8_t) sprintf(str, "%02X", loRa->buffer[i]);
			writeTx(u1);
		}
		if (loRa->len > 0) {
			writeTxReg('\n');
		}
		break;
	case RX_MODE:
		u1->tx_len = (uint8_t) sprintf(str, "%s Mode\r\n",
				(loRa->mode == MASTER_RECEIVER) ? "Master Receiver" :
				(loRa->mode == SLAVE_RECEIVER) ? "Slave Receiver" : "Unknown");
		writeTx(u1);
		break;
	case CRC_ERROR_ACTIVATION:
		u1->tx_len = (uint8_t) sprintf(str,
				"Reception Fail: CRC error activation\r\n");
		writeTx(u1);
		break;
	default:
		break;
	}
	cleanTx(u1);
}

void parseLoRaMaster(RDSS_t *rs485, SX1278_t *loRa) {
	fillValidBuffer(&*rs485, loRa->buffer, loRa->len);
	memset(loRa->buffer, 0, sizeof(loRa->buffer));
	loRa->len = 0;
	if (rs485->status == DATA_OK) {
		rs485->cmd = rs485->buffer[CMD_INDEX];
		rs485->idReceived = rs485->buffer[MODULE_ID_INDEX];
		if (rs485->idReceived == rs485->idQuery)
			rs485->status = LORA_RECEIVE;
		else
			rs485->status = WRONG_MODULE_ID;

		rs485->idQuery = 0;
	}
}

void parseUartMaster(UART1_t *u1, RDSS_t *rs485) {
	u1->isReady = false;
	fillValidBuffer(&*rs485, u1->rx, u1->rxLen);
	memset(u1->rx, 0, sizeof(u1->rx));
	u1->rxLen = 0;
	if (rs485->status == DATA_OK) {
		rs485->cmd = rs485->buffer[CMD_INDEX];
		rs485->idReceived = rs485->buffer[MODULE_ID_INDEX];
		rs485->idQuery = rs485->idReceived;
		rs485->status = LORA_SEND;
	}
}

void sendQuery(RDSS_t *rs485, SX1278_t *loRa, UART1_t *u1) {
	if (rs485->len != 9)
		return;
	rs485->idQuery = rs485->buffer[MODULE_ID_INDEX];
	loRa->len = rs485->len;
	updateMode(loRa, MASTER_SENDER);
	memcpy(loRa->buffer, rs485->buffer, rs485->len);
	loRa->status = TX_BUFFER_READY;
	printLoRaStatus(u1, loRa);
	transmit(loRa);
}

GPIO_PinState processLoRaSlaveReceiver(SX1278_t *loRa, UART1_t *u1) {
	if (loRa->operatingMode != RX_CONTINUOUS) {
		updateMode(loRa, SLAVE_RECEIVER);
		setRxFifoAddr(loRa);
		setLoraLowFreqModeReg(loRa, RX_CONTINUOUS);
	}
	clearMemForRx(loRa);
	GPIO_PinState bussy = HAL_GPIO_ReadPin(LORA_BUSSY_GPIO_Port,
	LORA_BUSSY_Pin);
	if (bussy == GPIO_PIN_SET)
		if (crcErrorActivation(loRa) != 1) {
			getRxFifoData(loRa);
			printLoRaStatus(u1, loRa);
		}

	if (loRa->status == RX_DONE) {
		setRxFifoAddr(loRa);
		setLoraLowFreqModeReg(loRa, RX_CONTINUOUS);
		readOperatingMode(loRa);
		loRa->status = RX_MODE;
		printLoRaStatus(u1, loRa);
	}
	return bussy;
}

// Function to process LORA receive state
void processLoraReceiveState(UART1_t *uartHandle, RDSS_t *rdssHandle,
		Vlad_t *vladHandle, SX1278_t *loRaHandle) {
	printStatus(uartHandle, rdssHandle);
	if (rdssHandle->cmd == QUERY_STATUS && rdssHandle->len == 21) {
		decodeVLAD(vladHandle, rdssHandle->buffer);
		memcpy(uartHandle->tx, rdssHandle->buffer, rdssHandle->len);
		uartHandle->tx_len = rdssHandle->len;
		writeTx(uartHandle);
		uartHandle->tx_len = 0;
		print_parameters(uartHandle, *vladHandle);
	}
	printLoRaStatus(uartHandle, loRaHandle);
	reinit(rdssHandle);
}

// Function to process LORA send state
void processLoraSendState(UART1_t *uartHandle, RDSS_t *rdssHandle,
		SX1278_t *loRaHandle) {
	printStatus(uartHandle, rdssHandle);
	if (rdssHandle->cmd != 0) {
		sendQuery(rdssHandle, loRaHandle, uartHandle);
	}
	printLoRaStatus(uartHandle, loRaHandle);
	reinit(rdssHandle);
	updateMode(loRaHandle, MASTER_RECEIVER);
	printLoRaStatus(uartHandle, loRaHandle);
}

// Function to process LORA master receive mode
void processMasterReceiverMode(UART1_t *uartHandle, SX1278_t *loRaHandle) {
	RX_MODE_ON_LED();
	TX_MODE_OFF_LED();
	if (loRaHandle->operatingMode != RX_CONTINUOUS) {
		updateMode(loRaHandle, MASTER_RECEIVER);
		setRxFifoAddr(loRaHandle);
		setLoraLowFreqModeReg(loRaHandle, RX_CONTINUOUS);
		printLoRaStatus(uartHandle, loRaHandle);
	}
	clearMemForRx(loRaHandle);
	GPIO_PinState bussy = HAL_GPIO_ReadPin(LORA_BUSSY_GPIO_Port,
			LORA_BUSSY_Pin);
	if (bussy == GPIO_PIN_SET && crcErrorActivation(loRaHandle) != 1) {
		getRxFifoData(loRaHandle);
		printLoRaStatus(uartHandle, loRaHandle);
	}
	if (loRaHandle->status == RX_DONE) {
		setRxFifoAddr(loRaHandle);
		setLoraLowFreqModeReg(loRaHandle, RX_CONTINUOUS);
		loRaHandle->status = RX_MODE;
		printLoRaStatus(uartHandle, loRaHandle);
	}
}

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
