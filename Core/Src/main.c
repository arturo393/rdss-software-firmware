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
LED_t led;
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
void dinamicFrame(SX1278_t *loRa);
void parseLoRaMaster(RDSS_t *rs485, SX1278_t *loRa);
uint8_t parseLoRaSlave(RDSS_t *remoteDiagnoticServer, SX1278_t *loRa);
void parseUart(UART1_t *u1, RDSS_t *rdss);
void sendLoRaMasterQuery(RDSS_t *rs485, SX1278_t *loRa);
uint8_t setBufferWithLtelCmd(uint8_t *buffer, RDSS_t *rdss, SX1278_t *loRa,
		Vlad_t *vlad);
uint8_t processReceivedLoraCommand(RDSS_t *rdss, SX1278_t *loRa, Vlad_t *vlad,
		UART1_t *u1);
void processReceivedUartCommand(Vlad_t *vlad, UART1_t *u1, RDSS_t *rdss,
		SX1278_t *loRa);
uint8_t processLoRaSlaveReceiver(SX1278_t *loRa, UART1_t *u1);
void handleCommunication(RDSS_t *rdss, SX1278_t *loRa, Vlad_t *vlad,
		UART1_t *u1);
void handleDefaultCase(RDSS_t *rdss, SX1278_t *loRa, UART1_t *u1);

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
	MX_SPI1_Init();
//  MX_USART1_UART_Init();
//  MX_I2C1_Init();
//  MX_IWDG_Init();
	/* USER CODE BEGIN 2 */

// Initialize the I2C peripheral (hi2c1) here
	i2c1MasterInit();
	u1 = uart1Init(HS16_CLK, BAUD_RATE);
	u1->isDebugModeEnabled = false;
	vlad = vladInit(0);
	rdss = rdssInit(vlad->id);
	ledInit(&led);
	loRa = loRaInit(&hspi1, SLAVE_RECEIVER);
	printLoRaStatus(u1, loRa);

	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		if (updateVladMeasurements(vlad))
			setQueryStatusBuffer(rdss, vlad);
		//handleCommunication(rdss, loRa, vlad, u1);
		if (u1->isReceivedDataReady) {
			parseUart(u1, rdss);
			processReceivedUartCommand(vlad, u1, rdss, loRa);
		}
		if (loRa->mode == SLAVE_RECEIVER) {
			if (processLoRaSlaveReceiver(loRa, u1) > 0)
				if (parseLoRaSlave(rdss, loRa))
					processReceivedLoraCommand(rdss, loRa, vlad, u1);

			rdssReinit(rdss);

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
	hiwdg.Init.Prescaler = IWDG_PRESCALER_4;
	hiwdg.Init.Window = 4095;
	hiwdg.Init.Reload = 4095;
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
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOB, LORA_NSS_Pin | LORA_RST_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(DIO1_GPIO_Port, DIO1_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(LORA_BUSSY_GPIO_Port, LORA_BUSSY_Pin, GPIO_PIN_RESET);

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

	/*Configure GPIO pin : DIO1_Pin */
	GPIO_InitStruct.Pin = DIO1_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(DIO1_GPIO_Port, &GPIO_InitStruct);

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
	if (u1->receivedDataLength >= RX_BUFFLEN) {
		cleanRx(u1);
		u1->receivedDataLength = 0;
	}
	u1->receiveBuffer[u1->receivedDataLength++] = readRxReg();
	if (u1->receiveBuffer[u1->receivedDataLength - 1] == LTEL_END_MARK)
		u1->isReceivedDataReady = true;
	if (u1->receiveBuffer[0] != LTEL_START_MARK) {
		cleanRx(u1);
		u1->receivedDataLength = 0;
	}

}

void print_parameters(UART1_t *u1, Vlad_t vlad) {
	if (!u1->isDebugModeEnabled)
		return;

	char *buff = (char*) u1->transmitBuffer;
	u1->transmittedDataLength = (uint8_t) sprintf(buff, "vin %d [V]\r\n",
			vlad.vin);
	writeTx(u1);
	u1->transmittedDataLength = (uint8_t) sprintf(buff,
			"current real %d [A]\r\n", (uint8_t) vlad.lineCurrentReal);
	writeTx(u1);
	u1->transmittedDataLength = (uint8_t) sprintf(buff,
			"tone level %d[dBm]\r\n", vlad.tone_level);
	writeTx(u1);
	u1->transmittedDataLength = (uint8_t) sprintf(buff, "current %d[A]\r\n",
			vlad.current);
	writeTx(u1);
	u1->transmittedDataLength = (uint8_t) sprintf(buff, "agc150m %d[dBm]\r\n",
			vlad.agc152m);
	writeTx(u1);
	u1->transmittedDataLength = (uint8_t) sprintf(buff, "level150m %d[dBm]\r\n",
			vlad.level152m);
	writeTx(u1);
	u1->transmittedDataLength = (uint8_t) sprintf(buff, "agc170m %d[dBm]\r\n",
			vlad.agc172m);
	writeTx(u1);
	u1->transmittedDataLength = (uint8_t) sprintf(buff, "level170m %d[dBm]\r\n",
			vlad.level172m);
	writeTx(u1);
	cleanTx(u1);
}

void printStatus(UART1_t *u1, RDSS_t *rdss) {

	if (!u1->isDebugModeEnabled)
		return;

	char *str = (char*) u1->transmitBuffer;
	uint8_t i = 0;
	switch (rdss->status) {
	case CRC_ERROR:
		checkCRCValidity(rdss->buffer, rdss->len);
		u1->transmittedDataLength = (uint8_t) sprintf(str, "CRC mismatch:\r\n");
		writeTx(u1);
		u1->transmittedDataLength = 0;
		break;
	case WRONG_MODULE_ID:
		u1->transmittedDataLength = (uint8_t) sprintf(str,
				"ID mismatch - ID %d and ID received %d \r\n", rdss->id,
				rdss->idReceived);
		writeTx(u1);
		u1->transmittedDataLength = 0;
		break;
	case NOT_VALID_FRAME:
		u1->transmittedDataLength = (uint8_t) sprintf(str,
				"Not valid start byte \r\n");
		writeTx(u1);
		u1->transmittedDataLength = 0;
		break;
	case DATA_OK:
		u1->transmittedDataLength = (uint8_t) sprintf(str,
				"Validation ok: ID %02x Cmd %02x Bytes %d Data \r\n",
				rdss->buffer[2], rdss->buffer[3], rdss->buffer[5]);
		writeTx(u1);
		for (int i = DATA_START_INDEX; i < rdss->buffer[5]; i++) {
			if (i > 250)
				break;
			u1->transmittedDataLength = (uint8_t) sprintf(str, "%02X",
					rdss->buffer[i]);
			writeTx(u1);
		}
		writeTxReg('\n');
		break;
	case WAITING:
		u1->transmittedDataLength = (uint8_t) sprintf(str,
				"Waiting for new data\r\n");
		writeTx(u1);
		u1->transmittedDataLength = 0;
		break;
	case LORA_SEND:
		u1->transmittedDataLength = (uint8_t) sprintf(str,
				"Send uart data to loRa ID: %d\r\n", rdss->idReceived);
		writeTx(u1);
		u1->transmittedDataLength = 0;
		break;
	case LORA_RECEIVE:
		u1->transmittedDataLength = (uint8_t) sprintf(str,
				"Validation ok: ID %02x Cmd %02x Bytes %d Data \r\n",
				rdss->buffer[2], rdss->buffer[3], rdss->buffer[5]);
		writeTx(u1);
		for (i = DATA_START_INDEX; i < rdss->buffer[5]; i++) {
			if (i > 250)
				break;
			u1->transmittedDataLength = (uint8_t) sprintf(str, "%02X",
					rdss->buffer[i]);
			writeTx(u1);
		}
		if (i > DATA_START_INDEX)
			writeTxReg('\n');
		u1->transmittedDataLength = 0;
		break;
	case UART_SEND:
		u1->transmittedDataLength = (uint8_t) sprintf(str,
				"Reply vlad data: %d\r\n", rdss->idReceived);
		writeTx(u1);
		for (i = 0; i < rdss->len; i++) {
			if (i > 250)
				break;
			u1->transmittedDataLength = (uint8_t) sprintf(str, "%02X",
					rdss->buffer[i]);
			writeTx(u1);
		}
		if (i > 0)
			writeTxReg('\n');
		u1->transmittedDataLength = 0;
		break;
	case UART_VALID:
		u1->transmittedDataLength = (uint8_t) sprintf(str,
				"Validation ok: ID %02x Cmd %02x Bytes %d Data \r\n",
				rdss->buffer[2], rdss->buffer[3], rdss->buffer[5]);
		writeTx(u1);
		for (int i = DATA_START_INDEX; i < rdss->buffer[5]; i++) {
			if (i > 250)
				break;
			u1->transmittedDataLength = (uint8_t) sprintf(str, "%02X",
					rdss->buffer[i]);
			writeTx(u1);
		}
		if (i > DATA_START_INDEX)
			writeTxReg('\n');
		u1->transmittedDataLength = 0;
		break;
	default:
		break;

	}
	cleanTx(u1);

}

void printLoRaStatus(UART1_t *u1, SX1278_t *loRa) {
	if (!u1->isDebugModeEnabled) {
		return;
	}

	char *str = (char*) u1->transmitBuffer;

	switch (loRa->status) {
	case TX_TIMEOUT:
		u1->transmittedDataLength = (uint8_t) sprintf(str,
				"Transmission Fail: %d seconds Timeout\r\n", TX_TIMEOUT / 1000);
		writeTx(u1);
		break;
	case TX_DONE:
		u1->transmittedDataLength = (uint8_t) sprintf(str,
				"Transmission Done: %lu ms\r\n", loRa->lastTxTime);
		writeTx(u1);
		break;
	case TX_BUFFER_READY:
		u1->transmittedDataLength = (uint8_t) sprintf(str,
				"Transmission Buffer: %d bytes data \r\n", loRa->len);
		writeTx(u1);
		for (int i = 0; i < loRa->len; i++) {
			u1->transmittedDataLength = (uint8_t) sprintf(str, "%02X",
					loRa->buffer[i]);
			writeTx(u1);
		}
		writeTxReg('\n');
		break;
	case TX_MODE:
		u1->transmittedDataLength = (uint8_t) sprintf(str, "%s Mode\r\n",
				(loRa->mode == MASTER_SENDER) ? "Master Sender" :
				(loRa->mode == SLAVE_SENDER) ? "Slave Sender" : "Unknown");
		writeTx(u1);
		break;
	case RX_DONE:
		u1->transmittedDataLength = (uint8_t) sprintf(str,
				"Reception Done: %d bytes\r\n", loRa->len);
		writeTx(u1);
		for (int i = 0; i < loRa->len; i++) {
			u1->transmittedDataLength = (uint8_t) sprintf(str, "%02X",
					loRa->buffer[i]);
			writeTx(u1);
		}
		if (loRa->len > 0) {
			writeTxReg('\n');
		}
		break;
	case RX_MODE:
		u1->transmittedDataLength = (uint8_t) sprintf(str, "%s Mode\r\n",
				(loRa->mode == MASTER_RECEIVER) ? "Master Receiver" :
				(loRa->mode == SLAVE_RECEIVER) ? "Slave Receiver" : "Unknown");
		writeTx(u1);
		break;
	case CRC_ERROR_ACTIVATION:
		u1->transmittedDataLength = (uint8_t) sprintf(str,
				"Reception Fail: CRC error activation\r\n");
		writeTx(u1);
		break;
	default:
		break;
	}
	cleanTx(u1);
}

void dinamicFrame(SX1278_t *loRa) {
	uint8_t crc_frame[2];
	uint16_t crc;
	uint8_t i;
	uint8_t len = loRa->len;
	uint8_t *buff = loRa->buffer;

	if (len > 240)
		len = 240;

	buff[0] = LTEL_START_MARK;
	buff[1] = VLADR;
	buff[2] = ID1;
	buff[3] = QUERY_STATUS;
	buff[4] = 0x00;
	buff[5] = len;
	for (i = 6; i < len + 6; i++) {
		buff[i] = i - 6;
	}
	crc = crc_get(&(buff[1]), (len + 5));
	memcpy(crc_frame, &crc, 2);
	buff[len + 6] = crc_frame[0];
	buff[len + 7] = crc_frame[1];
	buff[len + 8] = LTEL_END_MARK;

	loRa->len = len + LTEL_QUERY_LENGTH;
	loRa->status = TX_BUFFER_READY;

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

/**
 * Parses the received LoRa data for the remote diagnostic server.
 * This function fills the valid buffer, resets the LoRa buffer, and updates the server status based on the received data.
 *
 * @param remoteDiagnosticServer Pointer to the RDSS_t structure representing the remote diagnostic server.
 * @param loRa Pointer to the SX1278_t structure representing the LoRa module.
 * @return 1 if the data is valid and received for the correct module, 0 otherwise.
 */
uint8_t parseLoRaSlave(RDSS_t *remoteDiagnosticServer, SX1278_t *loRa) {
	if (loRa->len <= 0)
		return 0;

	fillValidBuffer(remoteDiagnosticServer, loRa->buffer, loRa->len);
	memset(loRa->buffer, 0, sizeof(loRa->buffer));
	loRa->len = 0;

	if (remoteDiagnosticServer->status == DATA_OK) {
		remoteDiagnosticServer->cmd = remoteDiagnosticServer->buffer[CMD_INDEX];
		remoteDiagnosticServer->idReceived =
				remoteDiagnosticServer->buffer[MODULE_ID_INDEX];

		if (remoteDiagnosticServer->idReceived == remoteDiagnosticServer->id) {
			remoteDiagnosticServer->status = LORA_RECEIVE;
			return 1; // Data is valid and received for the correct module
		} else {
			remoteDiagnosticServer->status = WRONG_MODULE_ID;
			return 0; // Data received for the wrong module
		}
	}

	return 0; // Invalid data or status not OK
}

void parseUart(UART1_t *uart1, RDSS_t *rdss) {
	uart1->isReceivedDataReady = false;

	rdss->status = validateBuffer(uart1->receiveBuffer,
			uart1->receivedDataLength);

	if (rdss->status != DATA_OK) {
		// Clear UART buffer and length
		memset(uart1->receiveBuffer, 0, sizeof(uart1->receiveBuffer));
		uart1->receivedDataLength = 0;
		return;
	}
	// Copy from UART buffer to RDSS buffer
	rdss->cmd = uart1->receiveBuffer[CMD_INDEX];
	rdss->idReceived = uart1->receiveBuffer[MODULE_ID_INDEX];
	rdss->len = uart1->receivedDataLength;
	memcpy(rdss->buffer, uart1->receiveBuffer, uart1->receivedDataLength);

	// Clear UART buffer and length
	memset(uart1->receiveBuffer, 0, sizeof(uart1->receiveBuffer));
	uart1->receivedDataLength = 0;

	rdss->status = evaluateRdssStatus(rdss);
	if (isModuleCommand(rdss->cmd)) {
		rdss->status = UART_VALID;
	}
}

void sendLoRaMasterQuery(RDSS_t *rs485, SX1278_t *loRa) {
	if (rs485->len != LTEL_QUERY_LENGTH)
		return;
	rs485->idQuery = rs485->buffer[MODULE_ID_INDEX];
	loRa->len = rs485->len;
	changeLoRaOperatingMode(&*loRa, MASTER_SENDER);
	memcpy(loRa->buffer, rs485->buffer, rs485->len);
	transmitDataUsingLoRa(&*loRa);
}

uint8_t setBufferWithLtelCmd(uint8_t *buffer, RDSS_t *rdss, SX1278_t *loRa,
		Vlad_t *vlad) {
	uint8_t index = 0;
	uint8_t eepromResponse = 0;

	if (rdss->len < LTEL_QUERY_LENGTH || rdss->len > LTEL_SET_LENGTH)
		return 0;

	if (rdss->cmd == QUERY_STATUS) {
		memcpy(buffer, rdss->queryBuffer, QUERY_STATUS_BUFFER_SIZE);
		return QUERY_STATUS_BUFFER_SIZE;
	}

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
		vlad->function = rdss->buffer[6];
		vlad->id = rdss->buffer[7];
		rdss->id = vlad->id;
		index = setRdssStartData(rdss, buffer);
		buffer[index++] = VLADR;
		buffer[index++] = rdss->id;
		savePage(CAT24C02_PAGE0_START_ADDR, (uint8_t*) &(vlad->function), 3, 1);
		savePage(CAT24C02_PAGE0_START_ADDR, (uint8_t*) &(vlad->id), 4, 1);
		break;
	case SET_TX_FREQ:
		buffer[index++] = 4;
		loRa->upFreq = freqDecode(rdss->buffer + index);
		index += sizeof(loRa->upFreq);
		eepromResponse = savePage(CAT24C02_PAGE1_START_ADDR,
				(uint8_t*) &(loRa->upFreq), 0, 4);
		break;
	case SET_RX_FREQ:
		buffer[index++] = 4;
		loRa->dlFreq = freqDecode(rdss->buffer + index);
		index += sizeof(loRa->dlFreq);
		eepromResponse = savePage(CAT24C02_PAGE1_START_ADDR,
				(uint8_t*) &(loRa->dlFreq), 4, 4);
		break;
	case SET_BANDWIDTH:
		buffer[index++] = 1;
		loRa->bandwidth = rdss->buffer[index++] - 1;
		eepromResponse = savePage(CAT24C02_PAGE0_START_ADDR, &(loRa->bandwidth),
				1, 1);
		break;
	case SET_SPREAD_FACTOR:
		buffer[index++] = 1;
		loRa->spreadFactor = rdss->buffer[index++] + 6;
		eepromResponse = savePage(CAT24C02_PAGE0_START_ADDR,
				&(loRa->spreadFactor), 0, 1);
		break;
	case SET_CODING_RATE:
		buffer[index++] = 1;
		loRa->codingRate = rdss->buffer[index++];
		eepromResponse = savePage(CAT24C02_PAGE0_START_ADDR,
				&(loRa->codingRate), 2, 1);
		break;
	default:
		break;
	}

	index += setCrc(buffer, index);
	buffer[index++] = LTEL_END_MARK;
	rdss->status = UART_SEND;
	if (eepromResponse > 0)
		changeLoRaOperatingMode(loRa, MASTER_RECEIVER);

	return index;
}

/**
 Processes the received LoRa command, performs corresponding actions, and manages LoRa communication.
 This function updates the LoRa buffer with LTel command, transmits an attenuation command via I2C,
 changes the LoRa operating mode, prints status messages, transmits data using LoRa, and reinitializes
 the RDSS structure.
 @param rdss Pointer to the RDSS_t structure containing the LoRa command information.
 @param loRa Pointer to the SX1278_t structure representing the LoRa module.
 @param vlad Pointer to the Vlad_t structure containing additional data.
 @param u1 Pointer to the UART1_t structure for UART communication.
 @return The index value indicating the progress of the function execution.
 */
uint8_t processReceivedLoraCommand(RDSS_t *rdss, SX1278_t *loRa, Vlad_t *vlad,
		UART1_t *u1) {
	uint8_t index = 0;

	// Update LoRa buffer with LTel command
	loRa->len = setBufferWithLtelCmd(loRa->buffer, rdss, loRa, vlad);

	if (rdss->cmd == SET_VLAD_ATTENUATION) { // If the received command is to set attenuation
		uint8_t attenuationCommand[2];
		uint8_t i2cSlaveAddress = 0x08;

		// Set RDSS start data
		uint8_t index = setRdssStartData(&*rdss, loRa->buffer);

		attenuationCommand[0] = SET_VLAD_ATTENUATION;
		attenuationCommand[1] = rdss->buffer[ATTENUATION_VALUE_INDEX];

		// Transmit attenuation command via I2C
		vlad->is_attenuation_updated = i2c1MasterTransmit(i2cSlaveAddress,
				attenuationCommand, sizeof(attenuationCommand), 10);

		loRa->buffer[index++] = vlad->is_attenuation_updated;
		index += setCrc(loRa->buffer, index); // Set CRC in the LoRa buffer
		loRa->buffer[index++] = LTEL_END_MARK;
		loRa->len = index;
		loRa->status = TX_BUFFER_READY;
	}

	printLoRaStatus(u1, loRa); // Print LoRa status
	changeLoRaOperatingMode(loRa, SLAVE_SENDER); // Change LoRa operating mode to SLAVE_SENDER
	printStatus(u1, &*rdss); // Print status
	transmitDataUsingLoRa(loRa); // Transmit data using LoRa
	printLoRaStatus(u1, loRa); // Print LoRa status
	reinit(&*rdss); // Reinitialize the RDSS structure
	changeLoRaOperatingMode(loRa, SLAVE_RECEIVER); // Change LoRa operating mode to SLAVE_RECEIVER
	printLoRaStatus(u1, loRa); // Print LoRa status

	return index; // Return the index value indicating the progress of the function execution
}

void processReceivedUartCommand(Vlad_t *vlad, UART1_t *u1, RDSS_t *rdss,
		SX1278_t *loRa) {
	u1->transmittedDataLength = setBufferWithLtelCmd(u1->transmitBuffer, rdss,
			loRa, vlad);
	if (rdss->cmd == SET_VLAD_ATTENUATION) {
		uint8_t attenuationCommand[2];
		uint8_t i2cSlaveAddress = 0x08;
		uint8_t index = setRdssStartData(rdss, u1->transmitBuffer);
		attenuationCommand[0] = SET_VLAD_ATTENUATION;
		attenuationCommand[1] = rdss->buffer[5];
		vlad->is_attenuation_updated = i2c1MasterTransmit(i2cSlaveAddress,
				attenuationCommand, sizeof(attenuationCommand), 10);

		u1->transmitBuffer[index++] = vlad->is_attenuation_updated;
		index += setCrc(u1->transmitBuffer, index);
		u1->transmitBuffer[index++] = LTEL_END_MARK;
		u1->transmittedDataLength = index;
	}
	writeTx(u1);
	printStatus(u1, rdss);
	reinit(rdss);
	cleanTx(u1);
	u1->transmittedDataLength = 0;
}

/**
 * Processes the receive operation of the LoRa slave receiver.
 * This function handles the LoRa operating mode, receives data from the FIFO,
 * checks for CRC errors, updates the LoRa status, and prints the LoRa status.
 *
 * @param loRa Pointer to the SX1278_t structure representing the LoRa module.
 * @param u1 Pointer to the UART1_t structure for UART communication.
 * @return The length of the received data.
 */
uint8_t processLoRaSlaveReceiver(SX1278_t *loRa, UART1_t *u1) {
	uint8_t receivedLength = 0;

	if (loRa->operatingMode != RX_CONTINUOUS) {
		changeLoRaOperatingMode(loRa, SLAVE_RECEIVER);
		setRxFifoAddr(loRa);
		setLoRaLowFreqModeReg(loRa, RX_CONTINUOUS);
	}
	clearRxMemory(loRa);

	// Check BUSY pin status
	GPIO_PinState busy = HAL_GPIO_ReadPin(LORA_BUSSY_GPIO_Port, LORA_BUSSY_Pin);
	if (busy == GPIO_PIN_SET) {
		if (crcErrorActivation(loRa) != 1) {
			receivedLength = getRxFifoData(loRa);
			printLoRaStatus(u1, loRa);
		}
	}

	if (loRa->status == RX_DONE) {
		setRxFifoAddr(loRa);
		setLoRaLowFreqModeReg(loRa, RX_CONTINUOUS);
		readOperatingMode(loRa);
		loRa->status = RX_MODE;
		printLoRaStatus(u1, loRa);
	}

	return receivedLength;
}

/**
 Manages the communication flow based on the status of the RDSS structure.
 This function handles different communication scenarios such as receiving LoRa commands,
 sending LoRa queries, processing UART commands, and handling default cases.
 @param rdss Pointer to the RDSS_t structure containing communication status and data.
 @param loRa Pointer to the SX1278_t structure representing the LoRa module.
 @param vlad Pointer to the Vlad_t structure containing additional data.
 @param u1 Pointer to the UART1_t structure for UART communication.
 */
void handleCommunication(RDSS_t *rdss, SX1278_t *loRa, Vlad_t *vlad,
		UART1_t *u1) {
	switch (rdss->status) {
	case LORA_RECEIVE:
		// Process received LoRa command
		processReceivedLoraCommand(rdss, loRa, vlad, u1);
		break;
	case LORA_SEND:
		// Print status and send LoRa query if the command is QUERY_STATUS
		printStatus(u1, rdss);
		if (rdss->cmd == QUERY_STATUS) {
			sendLoRaMasterQuery(rdss, loRa);
		}
		printLoRaStatus(u1, loRa);
		reinit(rdss); // Reinitialize the RDSS structure
		break;
	case UART_VALID:
		// Process received UART command
		processReceivedUartCommand(vlad, u1, rdss, loRa);
		break;
	default:
		// Handle default case
		handleDefaultCase(rdss, loRa, u1);
		break;
	}
}

void handleDefaultCase(RDSS_t *rdss, SX1278_t *loRa, UART1_t *u1) {
	if (u1->isReceivedDataReady) {
		parseUart(u1, rdss);
		processReceivedUartCommand(vlad, u1, rdss, loRa);
		printStatus(u1, rdss);
	} else if (loRa->len > 0) {
		parseLoRaSlave(rdss, loRa);
		printStatus(u1, rdss);
		printLoRaStatus(u1, loRa);
	} else if (loRa->mode == SLAVE_RECEIVER) {
		processLoRaSlaveReceiver(loRa, u1);
		if (loRa->len > 0) {
			parseLoRaSlave(rdss, loRa);
			printStatus(u1, rdss);
			printLoRaStatus(u1, loRa);
			processReceivedLoraCommand(rdss, loRa, vlad, u1);
		}
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
