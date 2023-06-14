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
#include "module.h"
#include <string.h>
#include "spi.h"
#include "max4003.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#define HS16_CLK 16000000
#define BAUD_RATE 115200
#define IWDG_ENABLE

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

IWDG_HandleTypeDef hiwdg;

SPI_HandleTypeDef hspi1;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
UART1_t *u1;
LED_t *led;
RDSS_t *rdss;
SX1278_t *loRa;
Vlad_t *vlad;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_I2C1_Init(void);
static void MX_IWDG_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void USART1_IRQHandler(void);
void print_parameters(UART1_t *u1, Vlad_t vlad);
void printStatus(UART1_t *u1, RDSS_t *rdss);
void printLoRaStatus(UART1_t *u1, SX1278_t *loRa);
void setQueryStatusBuffer(RDSS_t *rdss, Vlad_t *vlad);
void setQueryRdssBuffer(RDSS_t *rdss, Vlad_t *vlad);
void i2cSetQueryRdssBuffer(RDSS_t *rdss);
uint8_t executeCommand(uint8_t *buffer, RDSS_t *rdss, SX1278_t *loRa,
		Vlad_t *vlad);
void processUart1Rx(UART1_t *u1, RDSS_t *rdss, SX1278_t *loRa, Vlad_t *vlad);
void processCommand(SX1278_t *loRa, RDSS_t *rdss, Vlad_t *vlad);
void transmitStatus(SX1278_t *loRa, RDSS_t *rdss);
void slaveProcessRdss(RDSS_t *rdss, SX1278_t *loRa, Vlad_t *vlad);
void slaveProcessLoRaRx(SX1278_t *loRa, RDSS_t *rdss, Vlad_t *vlad);

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
	MX_GPIO_Init();
	MX_SPI1_Init();
	MX_USART1_UART_Init();
	MX_I2C1_Init();
#ifdef IWDG_ENABLE
  MX_IWDG_Init();
#endif
	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */

	/* USER CODE BEGIN 2 */

	i2c1MasterInit();
	u1 = uart1Init(HS16_CLK, BAUD_RATE);
	u1->isDebugModeEnabled = false;
	vlad = vladInit(0);
	rdss = rdssInit(vlad->id);
	loRa = loRaInit(&hspi1, SLAVE_RECEIVER);
	led = ledInit();

#ifdef IWDG_ENABLE
	HAL_IWDG_Refresh(&hiwdg);
#endif
	rdss->lastUpdateTicks = HAL_GetTick();
	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {

		/*
		 if (updateVladMeasurements(vlad))
		 setQueryRdssBuffer(rdss, vlad);

		 */
		updateStatus(rdss, 3000);
		processUart1Rx(u1, rdss, loRa, vlad);
		configureLoRaRx(loRa, SLAVE_RECEIVER);
		slaveProcessLoRaRx(loRa, rdss, vlad);
		blinkKALed(led);
#ifdef IWDG_ENABLE
		HAL_IWDG_Refresh(&hiwdg);
#endif
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

	/** Configure the main internal regulator output voltage
	 */
	HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI
			| RCC_OSCILLATORTYPE_LSI;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.LSIState = RCC_LSI_ON;
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
	hi2c1.Init.Timing = 0x00303D5B;
	hi2c1.Init.OwnAddress1 = 0;
	hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	hi2c1.Init.OwnAddress2 = 0;
	hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
	hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
	if (HAL_I2C_Init(&hi2c1) != HAL_OK) {
		Error_Handler();
	}

	/** Configure Analogue filter
	 */
	if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE)
			!= HAL_OK) {
		Error_Handler();
	}

	/** Configure Digital filter
	 */
	if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN I2C1_Init 2 */

	/* USER CODE END I2C1_Init 2 */

}

/**
 * @brief IWDG Initialization Function
 * @param None
 * @retval None
 */
static void MX_IWDG_Init(void) {

	/* USER CODE BEGIN IWDG_Init 0 */

	/* USER CODE END IWDG_Init 0 */

	/* USER CODE BEGIN IWDG_Init 1 */

	/* USER CODE END IWDG_Init 1 */
	hiwdg.Instance = IWDG;
	hiwdg.Init.Prescaler = IWDG_PRESCALER_256;
	hiwdg.Init.Window = 4095;
	hiwdg.Init.Reload = 1000;
	if (HAL_IWDG_Init(&hiwdg) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN IWDG_Init 2 */

	/* USER CODE END IWDG_Init 2 */

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
	hspi1.Init.CRCPolynomial = 7;
	hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
	hspi1.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
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
	UART_HandleTypeDef huart1;
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
	if (HAL_UART_Init(&huart1) != HAL_OK) {
		Error_Handler();
	}
	if (HAL_UARTEx_SetTxFifoThreshold(&huart1, UART_TXFIFO_THRESHOLD_1_8)
			!= HAL_OK) {
		Error_Handler();
	}
	if (HAL_UARTEx_SetRxFifoThreshold(&huart1, UART_RXFIFO_THRESHOLD_1_8)
			!= HAL_OK) {
		Error_Handler();
	}
	if (HAL_UARTEx_DisableFifoMode(&huart1) != HAL_OK) {
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
static void MX_GPIO_Init(void) {
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	/* USER CODE BEGIN MX_GPIO_Init_1 */
	/* USER CODE END MX_GPIO_Init_1 */

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOA,
			KA_LED_Pin | LORA_TX_OK_Pin | LORA_RX_OK_Pin | DIO1_Pin,
			GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOB, LORA_NSS_Pin | LORA_RST_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(LORA_BUSSY_GPIO_Port, LORA_BUSSY_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pins : KA_LED_Pin LORA_TX_OK_Pin LORA_RX_OK_Pin DIO1_Pin */
	GPIO_InitStruct.Pin = KA_LED_Pin | LORA_TX_OK_Pin | LORA_RX_OK_Pin
			| DIO1_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*Configure GPIO pins : LORA_NSS_Pin LORA_RST_Pin */
	GPIO_InitStruct.Pin = LORA_NSS_Pin | LORA_RST_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/*Configure GPIO pin : DIO3_Pin */
	GPIO_InitStruct.Pin = DIO3_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(DIO3_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pin : LORA_BUSSY_Pin */
	GPIO_InitStruct.Pin = LORA_BUSSY_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(LORA_BUSSY_GPIO_Port, &GPIO_InitStruct);

	/* USER CODE BEGIN MX_GPIO_Init_2 */
	/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

void USART1_IRQHandler(void) {
	if (u1->rxSize >= RX_BUFFLEN) {
		memset(u1->rxData, 0, RX_BUFFLEN);
		u1->rxSize = 0;
		u1->isReceivedDataReady = false;
	}

	u1->rxData[u1->rxSize++] = readRxReg();
	if (u1->rxData[u1->rxSize - 1] == LTEL_END_MARK)
		u1->isReceivedDataReady = true;
	if (u1->rxData[0] != LTEL_START_MARK) {
		memset(u1->rxData, 0, RX_BUFFLEN);
		u1->rxSize = 0;
		u1->isReceivedDataReady = false;
	}

}

uint8_t executeCommand(uint8_t *buffer, RDSS_t *rdss, SX1278_t *loRa,
		Vlad_t *vlad) {
	uint8_t index = 0;
	uint8_t attenuationCommand[2];
	const uint8_t i2cSlaveAddress = 0x08;

	if (rdss->buffSize < LTEL_QUERY_LENGTH || rdss->buffSize > LTEL_SET_LENGTH)
		return 0;

	index = setRdssStartData(rdss, buffer);

	switch (rdss->cmd) {
	case QUERY_RX_FREQ:
		buffer[index++] = 4;
		freqEncode(buffer + index, loRa->dlFreq);
		index += sizeof(loRa->dlFreq);
		index++;
		break;
	case QUERY_TX_FREQ:
		buffer[index++] = 4;
		freqEncode(buffer + index, loRa->upFreq);
		index += sizeof(loRa->upFreq);
		index++;
		break;
	case QUERY_SPREAD_FACTOR:
		buffer[index++] = 1;
		buffer[index++] = loRa->spreadFactor - 6;
		break;
	case QUERY_CODING_RATE:
		buffer[index++] = 1;
		buffer[index++] = loRa->codingRate;
		break;
	case QUERY_BANDWIDTH:
		buffer[index++] = 1;
		buffer[index++] = loRa->bandwidth + 1;
		break;
	case QUERY_MODULE_ID:
		index = 0;
		buffer[index++] = LTEL_START_MARK;
		buffer[index++] = VLADR;
		buffer[index++] = rdss->id;
		buffer[index++] = QUERY_MODULE_ID;
		buffer[index++] = 0x00;
		buffer[index++] = 2;
		buffer[index++] = VLADR;
		buffer[index++] = rdss->id;
		break;
	case SET_MODULE_ID:
		vlad->function = rdss->buff[6];
		vlad->id = rdss->buff[7];
		rdss->id = vlad->id;
		index = setRdssStartData(rdss, buffer);
		buffer[index++] = VLADR;
		buffer[index++] = rdss->id;
		savePage(CAT24C02_PAGE0_START_ADDR, (uint8_t*) &(vlad->function), 3, 1);
		savePage(CAT24C02_PAGE0_START_ADDR, (uint8_t*) &(vlad->id), 4, 1);
		break;
	case SET_TX_FREQ:
		buffer[index++] = 4;
		loRa->upFreq = freqDecode(rdss->buff + index);
		index += sizeof(loRa->upFreq);
		savePage(
		CAT24C02_PAGE1_START_ADDR, (uint8_t*) &(loRa->upFreq), 0, 4);
		changeMode(loRa, loRa->mode);
		writeLoRaParametersReg(loRa);
		break;
	case SET_RX_FREQ:
		buffer[index++] = 4;
		loRa->dlFreq = freqDecode(rdss->buff + index);
		index += sizeof(loRa->dlFreq);
		savePage(
		CAT24C02_PAGE1_START_ADDR, (uint8_t*) &(loRa->dlFreq), 4, 4);
		changeMode(loRa, loRa->mode);
		writeLoRaParametersReg(loRa);
		break;
	case SET_BANDWIDTH:
		buffer[index++] = 1;
		loRa->bandwidth = rdss->buff[index++] - 1;
		savePage(
		CAT24C02_PAGE0_START_ADDR, &(loRa->bandwidth), 1, 1);
		changeMode(loRa, loRa->mode);
		writeLoRaParametersReg(loRa);
		break;
	case SET_SPREAD_FACTOR:
		buffer[index++] = 1;
		loRa->spreadFactor = rdss->buff[index++] + 6;
		savePage(
		CAT24C02_PAGE0_START_ADDR, &(loRa->spreadFactor), 0, 1);
		changeMode(loRa, loRa->mode);
		writeLoRaParametersReg(loRa);
		break;
	case SET_CODING_RATE:
		buffer[index++] = 1;
		loRa->codingRate = rdss->buff[index++];
		savePage(
		CAT24C02_PAGE0_START_ADDR, &(loRa->codingRate), 2, 1);
		changeMode(loRa, loRa->mode);
		writeLoRaParametersReg(loRa);
		break;
	case SET_VLAD_ATTENUATION:

		attenuationCommand[0] = SET_VLAD_ATTENUATION;
		attenuationCommand[1] = rdss->buff[ATTENUATION_VALUE_INDEX];
		vlad->is_attenuation_updated = i2c1MasterTransmit(i2cSlaveAddress,
				attenuationCommand, sizeof(attenuationCommand), 10);
		buffer[index++] = vlad->is_attenuation_updated;
		break;
	default:
		break;
	}

	index += setCrc(buffer, index);
	buffer[index++] = LTEL_END_MARK;
	return index;

}

void printLoRaStatus(UART1_t *u1, SX1278_t *loRa) {
	if (!u1->isDebugModeEnabled) {
		return;
	}

	char *buff;
	uint8_t size;
	buff = malloc(sizeof(char) * 20);

	switch (loRa->status) {
	case TX_TIMEOUT:
		size = (uint8_t) sprintf(buff,
				"Transmission Fail: %d seconds Timeout\r\n", TX_TIMEOUT / 1000);
		writeTx(u1);
		break;
	case TX_DONE:
		size = (uint8_t) sprintf(buff, "Transmission Done: %lu ms\r\n",
				loRa->lastTxTime);
		writeTx(u1);
		break;
	case TX_BUFFER_READY:
		size = (uint8_t) sprintf(buff,
				"Transmission Buffer: %d bytes data \r\n", loRa->len);
		writeTx(u1);
		for (int i = 0; i < loRa->len; i++) {
			size = (uint8_t) sprintf(buff, "%02X", loRa->txData[i]);
			writeTx(u1);
		}
		writeTxReg('\n');
		break;
	case TX_MODE:
		size = (uint8_t) sprintf(buff, "%s Mode\r\n",
				(loRa->mode == MASTER_SENDER) ? "Master Sender" :
				(loRa->mode == SLAVE_SENDER) ? "Slave Sender" : "Unknown");
		writeTx(u1);
		break;
	case RX_DONE:
		size = (uint8_t) sprintf(buff, "Reception Done: %d bytes\r\n",
				loRa->len);
		writeTx(u1);
		for (int i = 0; i < loRa->len; i++) {
			size = (uint8_t) sprintf(buff, "%02X", loRa->rxData[i]);
			writeTx(u1);
		}
		if (loRa->len > 0) {
			writeTxReg('\n');
		}
		break;
	case RX_MODE:
		size = (uint8_t) sprintf(buff, "%s Mode\r\n",
				(loRa->mode == MASTER_RECEIVER) ? "Master Receiver" :
				(loRa->mode == SLAVE_RECEIVER) ? "Slave Receiver" : "Unknown");
		writeTx(u1);
		break;
	case CRC_ERROR_ACTIVATION:
		size = (uint8_t) sprintf(buff,
				"Reception Fail: CRC error activation\r\n");
		writeTx(u1);
		break;
	default:
		break;
	}
	cleanTx(u1);
}

uint8_t i2cGetVladMeasurements(uint8_t *buffer, uint8_t querySize) {
	const uint8_t slaveAddress = 0x08;
	uint8_t vladMeasurementsCmd[2] = { 0x10, 0x00 };

	if (!i2c1MasterTransmit(slaveAddress, vladMeasurementsCmd,
			sizeof(vladMeasurementsCmd), 10)) {
		memset(buffer, 0, querySize);
		return querySize;
	}
	HAL_Delay(6);
	if (!i2c1MasterReceive(slaveAddress, buffer, querySize, 10)) {
		memset(buffer, 0, querySize);
		return querySize;
	}

	return querySize;
}

void processUart1Rx(UART1_t *u1, RDSS_t *rdss, SX1278_t *loRa, Vlad_t *vlad) {
	if (u1->isReceivedDataReady == false)
		return;
	u1->isReceivedDataReady = false;
	HAL_Delay(1);
	if (validate(u1->rxData, u1->rxSize) != DATA_OK) {
		// Clear UART buffer and length
		memset(u1->rxData, 0, sizeof(u1->rxData));
		u1->rxSize = 0;
		return;
	}
	updateRdss(rdss, u1->rxData, u1->rxSize);
	u1->txData = malloc(sizeof(uint8_t) * 25);
	u1->txSize = executeCommand(u1->txData, rdss, loRa, vlad);
	for (uint8_t i = 0; i < u1->txSize; i++)
		writeTxReg(u1->txData[i]);
	free(u1->txData);
	memset(u1->rxData, 0, sizeof(u1->rxData));
	u1->rxSize = 0;
	reinit(rdss);
}

void processCommand(SX1278_t *loRa, RDSS_t *rdss, Vlad_t *vlad) {
	loRa->txData = malloc(sizeof(uint8_t) * 25);
	loRa->txSize = executeCommand(loRa->txData, rdss, loRa, vlad);
	changeMode(loRa, SLAVE_SENDER); // Change LoRa operating mode to SLAVE_SENDER
	transmit(loRa); // Transmit data using LoRa
	changeMode(loRa, SLAVE_RECEIVER); // Change LoRa operating mode to SLAVE_RECEIVER
	free(loRa->txData);
}

void setQueryStatusBuffer(RDSS_t *rdss, Vlad_t *vlad) {
	uint8_t index = 0;
	rdss->queryBuffer[index++] = LTEL_START_MARK;
	rdss->queryBuffer[index++] = VLADR;
	rdss->queryBuffer[index++] = vlad->id;
	rdss->queryBuffer[index++] = QUERY_STATUS;
	rdss->queryBuffer[index++] = 0x00;
	index += encodeVladToLtel(rdss->queryBuffer + index, vlad);
	index += setCrc(rdss->queryBuffer, index);
	rdss->queryBuffer[index++] = LTEL_END_MARK;
}

void setQueryRdssBuffer(RDSS_t *rdss, Vlad_t *vlad) {
	uint8_t index = 0;
	rdss->queryBuffer[index++] = LTEL_START_MARK;
	rdss->queryBuffer[index++] = VLADR;
	rdss->queryBuffer[index++] = vlad->id;
	rdss->queryBuffer[index++] = QUERY_STATUS;
	rdss->queryBuffer[index++] = 0x00;
	index += encodeVladToRdss(rdss->queryBuffer + index, vlad);
	index += setCrc(rdss->queryBuffer, index);
	rdss->queryBuffer[index++] = LTEL_END_MARK;
}

void i2cSetQueryRdssBuffer(RDSS_t *rdss) {
	uint8_t index = 0;
	const uint8_t querySize = 25;
	memset(rdss->queryBuffer, 0, QUERY_STATUS_BUFFER_SIZE);
	rdss->queryBuffer[index++] = LTEL_START_MARK;
	rdss->queryBuffer[index++] = VLADR;
	rdss->queryBuffer[index++] = rdss->id;
	rdss->queryBuffer[index++] = QUERY_STATUS;
	rdss->queryBuffer[index++] = 0x00;
	rdss->queryBuffer[index++] = querySize;
	index += i2cGetVladMeasurements(rdss->queryBuffer + index, querySize);
	index += setCrc(rdss->queryBuffer, index);
	rdss->queryBuffer[index++] = LTEL_END_MARK;
}

void transmitStatus(SX1278_t *loRa, RDSS_t *rdss) {
	loRa->txData = rdss->queryBuffer;
	loRa->txSize = QUERY_STATUS_BUFFER_SIZE;
	changeMode(loRa, SLAVE_SENDER); // Change LoRa operating mode to SLAVE_SENDER
	transmit(loRa); // Transmit data using LoRa
	if (loRa->status == TX_DONE)
		HAL_GPIO_WritePin(LORA_TX_OK_GPIO_Port, LORA_TX_OK_Pin, GPIO_PIN_SET);
	changeMode(loRa, SLAVE_RECEIVER); // Change LoRa operating mode to SLAVE_RECEIVER
	HAL_GPIO_WritePin(LORA_TX_OK_GPIO_Port, LORA_TX_OK_Pin, GPIO_PIN_RESET);
}

void slaveProcessRdss(RDSS_t *rdss, SX1278_t *loRa, Vlad_t *vlad) {
	if (rdss->idReceived != rdss->id) {
		rdss->status = WRONG_MODULE_ID;
		return;
	}
	if (rdss->cmd == QUERY_STATUS)
		transmitStatus(loRa, rdss);
	else
		processCommand(loRa, rdss, vlad);
}

void slaveProcessLoRaRx(SX1278_t *loRa, RDSS_t *rdss, Vlad_t *vlad) {
	if (HAL_GPIO_ReadPin(LORA_BUSSY_GPIO_Port,
	LORA_BUSSY_Pin) == GPIO_PIN_RESET)
		return; // if (crcErrorActivation(loRa) != 1)

	clearIrqFlagsReg(loRa);
	loRa->rxData = getRxFifoData(loRa);
	if (loRa->rxSize <= 0) {
		changeMode(loRa, loRa->mode);
		return;

	}
	if (validate(loRa->rxData, loRa->rxSize) != DATA_OK) {
		changeMode(loRa, loRa->mode);
		return;
	}
	HAL_GPIO_WritePin(LORA_RX_OK_GPIO_Port, LORA_RX_OK_Pin, GPIO_PIN_SET);
	updateRdss(rdss, loRa->rxData, loRa->rxSize);
	slaveProcessRdss(rdss, loRa, vlad);
	rdssReinit(rdss);
	free(loRa->rxData);
	loRa->rxSize = 0;
	HAL_GPIO_WritePin(LORA_RX_OK_GPIO_Port, LORA_RX_OK_Pin, GPIO_PIN_RESET);
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
