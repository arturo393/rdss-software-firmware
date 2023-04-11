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
RS485_t *rs485_ptr;
Vlad_t *vlad_ptr;
SX1278_t *lora_ptr;

/*void USART1_IRQHandler(void) {
 uart1_read_to_frame(uart1_ptr);
 }*/

/* In the interrupt handler, read the received data from the UART1 data register */
/* Enable UART1 interrupt */
uint8_t rxData;
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

Vlad_t decodeVLAD(uint8_t *buff) {
	Vlad_t vlad;
	vlad.vin = buff[6];
	vlad.vin2 = buff[7];
	vlad.current_real = buff[8];
	vlad.current_real2 = buff[9];
	vlad.tone_level = buff[10];
	vlad.tone_level2 = buff[11];
	vlad.current = buff[12];
	vlad.current2 = buff[13];
	vlad.agc150m = buff[14];
	vlad.level150m = buff[15];
	vlad.agc170m = buff[16];
	vlad.level170m = buff[17];
	return vlad;
}

void print_parameters(UART1_t *u, Vlad_t vlad) {
	char *buff = (char*) u->tx;
	u->txLen = sprintf(buff, "vin %d [V]\r\n", vlad.vin);
	writeTx(u);
	u->txLen = sprintf(buff, "vin2 %d[V]\r\n", vlad.vin2);
	writeTx(u);
	u->txLen = sprintf(buff, "current real %d [A]\r\n",
			(uint8_t) vlad.current_real);
	writeTx(u);
	u->txLen = sprintf(buff, "current real2  %d[A]\r\n", vlad.current_real2);
	writeTx(u);
	u->txLen = sprintf(buff, "tone level %d[dBm]\r\n", vlad.tone_level);
	writeTx(u);
	u->txLen = sprintf(buff, "tone level2 %d[dBm]\r\n", vlad.tone_level2);
	writeTx(u);
	u->txLen = sprintf(buff, "current %d[A]\r\n", vlad.current);
	writeTx(u);
	u->txLen = sprintf(buff, "current2 %d[A]\r\n", vlad.current2);
	writeTx(u);
	u->txLen = sprintf(buff, "agc150m %d[dBm]\r\n", vlad.agc150m);
	writeTx(u);
	u->txLen = sprintf(buff, "level150m %d[dBm]\r\n", vlad.level150m);
	writeTx(u);
	u->txLen = sprintf(buff, "agc170m %d[dBm]\r\n", vlad.agc170m);
	writeTx(u);
	u->txLen = sprintf(buff, "level170m %d[dBm]\r\n", vlad.level170m);
	writeTx(u);
	cleanTx(u);
}

void printStatus(UART1_t *u1, RS485_t *rs485) {

	char *str = (char*) u1->tx;
	int i = 0;
	u1->txLen = sprintf(str, "RDSS Status - ");
	writeTx(u1);
	switch (rs485->status) {
	case CRC_ERROR:
		isValidCrc2(rs485);
		u1->txLen = sprintf(str, "CRC mismatch:\r\n");
		writeTx(u1);
		u1->txLen = 0;

		break;
	case WRONG_MODULE_ID:
		u1->txLen = sprintf(str, "ID mismatch - ID %d and ID received %d \r\n",
				rs485->id, rs485->idReceived);
		writeTx(u1);
		u1->txLen = 0;
		break;
	case NOT_VALID_FRAME:
		u1->txLen = sprintf(str, "Not valid start byte: %02x \r\n",
				rs485->buffer[0]);
		writeTx(u1);
		u1->txLen = 0;
		break;
	case DATA_OK:
		u1->txLen = sprintf(str,
				"Validation ok: ID %02x Cmd %02x Bytes %d Data \r\n",
				rs485->buffer[2], rs485->buffer[3], rs485->buffer[5]);
		writeTx(u1);
		for (int i = DATA_START_INDEX; i < rs485->buffer[5]; i++) {
			if (i > 250)
				break;
			u1->txLen = sprintf(str, "%02X", rs485->buffer[i]);
			writeTx(u1);
		}
		writeTxReg('\n');
		break;
	case WAITING:
		u1->txLen = sprintf(str, "Waiting for new data\r\n");
		writeTx(u1);
		u1->txLen = 0;
		break;
	case LORA_SEND:
		u1->txLen = sprintf(str, "Send uart data to loRa ID: %d\r\n",
				rs485->idReceived);
		writeTx(u1);
		u1->txLen = 0;
		break;
	case LORA_RECEIVE:
		u1->txLen = sprintf(str, "Receive loRa ID: %d with data\r\n",
				rs485->idReceived);
		writeTx(u1);

		for (uint8_t i = DATA_START_INDEX; i < rs485->buffer[DATA_LENGHT2_INDEX];
				i++) {
			if (i > 250)
				break;
			u1->txLen = sprintf(str, "%02X", rs485->buffer[i]);
			writeTx(u1);
		}
		if (i > DATA_START_INDEX)
			writeTxReg('\n');
		u1->txLen = 0;
		break;
	default:
		break;

	}
	cleanTx(u1);

}

void printLoRaStatus(UART1_t *u1, SX1278_t *loRa) {

	char *str = (char*) u1->tx;
	u1->txLen = sprintf(str, "LoRa Status - ");
	writeTx(u1);
	switch (loRa->status) {
	case TX_TIMEOUT:
		u1->txLen = sprintf(str, "Transmission Fail: %d seconds Timeout\r\n",
				LORA_SEND_TIMEOUT / 1000);
		writeTx(u1);
		break;
	case TX_DONE:
		u1->txLen = sprintf(str, "Transmission Done: %lu ms\r\n",
				loRa->lastTxTime);
		writeTx(u1);
		break;
	case TX_BUFFER_READY:
		u1->txLen = sprintf(str, "Transmission Buffer: %d bytes data \r\n",
				loRa->len);
		writeTx(u1);
		for (int i = 0; i < loRa->len; i++) {
			u1->txLen = sprintf(str, "%02X", loRa->buffer[i]);
			writeTx(u1);
		}
		writeTxReg('\n');
		break;
	case TX_MODE:
		if (loRa->mode == MASTER_SENDER)
			u1->txLen = sprintf(str, "Master Sender Mode\r\n");
		else if (loRa->mode == SLAVE_SENDER)
			u1->txLen = sprintf(str, "Slave Sender Mode\r\n");
		else
			u1->txLen = sprintf(str, "Unknow Mode\r\n");
		writeTx(u1);
		break;
	case RX_DONE:
		u1->txLen = sprintf(str, "Reception Done: %d bytes\r\n", loRa->len);
		writeTx(u1);
		int i = 0;
		for (i = 0; i < loRa->len; i++) {
			u1->txLen = sprintf(str, "%02X", loRa->buffer[i]);
			writeTx(u1);
		}
		if (i > 0)
			writeTxReg('\n');
		break;
	case RX_MODE:
		if (loRa->mode == MASTER_RECEIVER)
			u1->txLen = sprintf(str, "Master receiver Mode\r\n");
		else if (loRa->mode == SLAVE_RECEIVER)
			u1->txLen = sprintf(str, "Slave receiver Mode\r\n");
		else
			u1->txLen = sprintf(str, "Unknow Mode\r\n");
		writeTx(u1);
		break;
	case CRC_ERROR_ACTIVATION:
		u1->txLen = sprintf(str, "Reception Fail: Crc error activation\r\n");
		writeTx(u1);
		break;
	case RX_TIMEOUT:
		u1->txLen = sprintf(str, "Reception Tiemout: %d seconds \r\n",
		LORA_RECEIVE_TIMEOUT / 1000);
		writeTx(u1);
		break;
	default:
		break;
	}
	u1->txLen = 0;
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
	buff[1] = VLAD;
	buff[2] = ID1;
	buff[3] = QUERY_PARAMETERS_VLAD;
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

	loRa->len = len + 9;
	loRa->status = TX_BUFFER_READY;

}

void parseLoRaMaster(RS485_t *rs485, SX1278_t *loRa) {
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

void parseUartMaster(UART1_t *u1, RS485_t *rs485) {
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

void sendQuery(RS485_t *rs485, SX1278_t *loRa, UART1_t *u1) {
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

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */

int main(void) {
	/* USER CODE BEGIN 1 */
	LED_t led;
	RS485_t rs485;
	UART1_t u1;
	Vlad_t vlad;
	vlad_ptr = &vlad;
	uart1_ptr = &u1;
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
	MX_I2C1_Init();
	MX_SPI1_Init();
	MX_USART1_UART_Init();
	MX_USART2_UART_Init();
	MX_USART3_UART_Init();
	MX_ADC1_Init();
	MX_CRC_Init();
	/* USER CODE BEGIN 2 */
	vladInit(VLAD, ID0, &vlad);
	ledInit(&led);
	rs485Init(&rs485);
	rs485.id = ID0;
//uart1Init(HS16_CLK, BAUD_RATE, &uart1);

	SX1278_t loRa;
	lora_ptr = &loRa;

	loRa.spi = &hspi1;
	HAL_GPIO_WritePin(LORA_NSS_GPIO_Port, LORA_NSS_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LORA_RST_GPIO_Port, LORA_RST_Pin, GPIO_PIN_SET);
	loRa.operatingMode = readRegister(&hspi1, LR_RegOpMode);
	memset(loRa.buffer, 0, SX1278_MAX_PACKET);
	loRa.len = 0;
	int counter = HAL_GetTick();
	int dataLen = 0;

	HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(USART1_IRQn);
	HAL_UART_Receive_IT(&huart1, &rxData, 1);
	loRa.mode = -1;
	initLoRaParameters(&loRa, MASTER_RECEIVER);
	writeLoRaParameters(&loRa);
	printStatus(&u1, &rs485);
	printLoRaStatus(&u1, &loRa);
	srand(time(NULL));
	uint32_t preambleTime = HAL_GetTick();
	uint16_t PREAMBLE_LENGTH = 0;
	uint16_t preambleBytes = 0;
	uint16_t symbTiemout = 0;
//initialize LoRa module

	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {


		if (rs485.status == LORA_RECEIVE) {
			printStatus(&u1, &rs485);
			if (rs485.cmd == QUERY_PARAMETERS_VLAD)
				if (rs485.len == 21) {
					vlad = decodeVLAD(rs485.buffer);
					print_parameters(&u1, vlad);
				}
			printLoRaStatus(&u1, &loRa);
			reinit(&rs485);
			//	updateMode(&loRa, MASTER_SENDER);

		} else if (rs485.status == LORA_SEND) {
			printStatus(&u1, &rs485);
			if (rs485.cmd == QUERY_PARAMETERS_VLAD)
				sendQuery(&rs485, &loRa, &u1);
			printLoRaStatus(&u1, &loRa);
			reinit(&rs485);
			updateMode(&loRa, MASTER_RECEIVER);
			printLoRaStatus(&u1, &loRa);
		} else if (rs485.status == CRC_ERROR) {
			printStatus(&u1, &rs485);
			reinit(&rs485);
		} else if (u1.isReady)
			parseUartMaster(&u1, &rs485);
		else if (loRa.len > 0)
			parseLoRaMaster(&rs485, &loRa);
		else if (loRa.mode == MASTER_RECEIVER) {
			RX_MODE_ON_LED();
			TX_MODE_OFF_LED();
			if (loRa.operatingMode != RX_CONTINUOUS) {
				updateMode(&loRa, MASTER_RECEIVER);
				setRxFifoAddr(&loRa);
				updateLoraLowFreq(&loRa, RX_CONTINUOUS);
				printLoRaStatus(&u1, &loRa);
			}
			clearMemForRx(&loRa);
			GPIO_PinState bussy = HAL_GPIO_ReadPin(LORA_BUSSY_GPIO_Port,
			LORA_BUSSY_Pin);
			if (bussy == GPIO_PIN_SET)
				if (crcErrorActivation(&loRa) != 1) {
					getRxFifoData(&loRa);
					printLoRaStatus(&u1, &loRa);
				}

			if (loRa.status == RX_DONE) {
				setRxFifoAddr(&loRa);
				updateLoraLowFreq(&loRa, RX_CONTINUOUS);
				loRa.status = RX_MODE;
				printLoRaStatus(&u1, &loRa);
			}
			//updateMode(&loRa, MASTER_SENDER);

		} else if (loRa.mode == MASTER_SENDER) {
			RX_MODE_OFF_LED();
			TX_MODE_ON_LED();
			/*			if (HAL_GetTick() - counter > 1000) {
			 counter = HAL_GetTick();
			 if (dataLen == 240)
			 dataLen = 0;
			 loRa.len = dataLen;
			 dataLen += 1;
			 dinamicFrame(&loRa);
			 printLoRaStatus(&u1, &loRa);
			 transmit(&loRa);
			 printLoRaStatus(&u1, &loRa);
			 }
			 */
		}
		led_enable_kalive(&led);
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
	huart1.Init.BaudRate = 115200;
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
