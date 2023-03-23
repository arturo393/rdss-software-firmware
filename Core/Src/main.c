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
	if (uart1_ptr->len >= RX_BUFFLEN) {
		cleanRxBuffer(uart1_ptr);
		uart1_ptr->len = 0;
	}
	HAL_UART_Receive_IT(&huart1, &rxData, 1);
	uart1_ptr->rxBuffer[uart1_ptr->len++] = rxData;
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi) {
	printf("SPI TX Done .. Do Something ...");
}

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi) {
	printf("SPI RX Done .. Do Something ...");
}

int _write(int file, char *ptr, int len) {
	int i;
	HAL_UART_Transmit(&huart1, (uint8_t*) ptr, len, 50);
	for (i = 0; i < len; i++) {
		ITM_SendChar(*ptr++);
	}
	return len;
}

void setTxFifoAddr(SX1278_t *module) {
	uint8_t cmd = module->len;
	writeRegister(module->spi, LR_RegPayloadLength, &(cmd), 1);
	uint8_t addr = readRegister(module->spi, LR_RegFifoTxBaseAddr);
	addr = 0x80;
	writeRegister(module->spi, LR_RegFifoAddrPtr, &addr, 1);
	module->len = readRegister(module->spi, LR_RegPayloadLength);
}

void sx1278Reset() {
	HAL_GPIO_WritePin(LORA_NSS_GPIO_Port, LORA_NSS_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LORA_NSS_GPIO_Port, LORA_NSS_Pin, GPIO_PIN_RESET);
	HAL_Delay(1);
	HAL_GPIO_WritePin(LORA_NSS_GPIO_Port, LORA_NSS_Pin, GPIO_PIN_SET);
	HAL_Delay(100);
}

void setRxFifoAddr(SX1278_t *module) {
	updateLoraLowFreq(module, SLEEP); //Change modem mode Must in Sleep mode
	uint8_t cmd = module->len;
	writeRegister(module->spi, LR_RegPayloadLength, &(cmd), 1); //RegPayloadLength 21byte
	uint8_t addr = readRegister(module->spi, LR_RegFifoRxBaseAddr); //RegFiFoTxBaseAddr
	writeRegister(module->spi, LR_RegFifoAddrPtr, &addr, 1); //RegFifoAddrPtr
	module->len = readRegister(module->spi, LR_RegPayloadLength);
}

void clearMemForRx(SX1278_t *loRa) {
	if (loRa->status == RX_READY) {
		memset(loRa->buffer, 0, SX1278_MAX_PACKET);
	}
}

uint8_t waitForRxDone(SX1278_t *loRa) {
	uint32_t timeout = HAL_GetTick();
	while ((!SX1278_hw_GetDIO0(loRa->hw))) {
		uint8_t flags = readRegister(loRa->spi, LR_RegIrqFlags);
		if (READ_BIT(flags, PAYLOAD_CRC_ERROR_MASK)) {
			uint8_t cmd = flags | (1 << 7);
			writeRegister(loRa->spi, LR_RegIrqFlags, &cmd, 1);
			flags = readRegister(loRa->spi, LR_RegIrqFlags);
		}
		if ((HAL_GetTick() - timeout > 5000))
			return -1;
	}
	return 0;
}

int crcErrorActivation(SX1278_t *loRa) {
	uint8_t flags2 = readRegister(loRa->spi, LR_RegIrqFlags);
	SET_BIT(flags2, RX_DONE_MASK);
	uint8_t cmd = flags2;
	writeRegister(loRa->spi, LR_RegIrqFlags, &cmd, 1);
	uint8_t flags = readRegister(loRa->spi, LR_RegIrqFlags);
	int errorActivation = READ_BIT(flags, PAYLOAD_CRC_ERROR_MASK);
	return errorActivation;
}

void getRxFifoData(SX1278_t *loRa) {
	loRa->len = readRegister(loRa->spi, LR_RegRxNbBytes); //Number for received bytes
	uint8_t addr = 0x00;
	HAL_GPIO_WritePin(GPIOB, LORA_NSS_Pin, GPIO_PIN_RESET); // pull the pin low
	HAL_Delay(1);
	HAL_SPI_Transmit(loRa->spi, &addr, 1, 100); // send address
	HAL_SPI_Receive(loRa->spi, loRa->buffer, loRa->len, 100); // receive 6 bytes data
	HAL_Delay(1);
	HAL_GPIO_WritePin(GPIOB, LORA_NSS_Pin, GPIO_PIN_SET); // pull the pin high
}

void printParameters(int timeRx, UART1_t *uart1, SX1278_t *loRa) {
	if (loRa->len == 0)
		return;
	uint8_t tmp[50];
	for (int i = 0; i < loRa->len; i++) {
		uint8_t len = sprintf(uart1->txBuffer, "%02X", loRa->buffer[i]);
		uart1_send_frame(uart1->txBuffer, len);
	}
	uint8_t largo = sprintf(tmp, " - Rx Ok: %d ms %d bytes\n", timeRx,
			loRa->len);
	uart1_send_frame(tmp, largo);
}

bool TX_MODE;
bool RX_MODE;

Vlad_t decodeVLAD(SX1278_t *loRa) {
	Vlad_t vlad;
	vlad.vin = loRa->buffer[6];
	vlad.vin2 = loRa->buffer[7];
	vlad.current_real = loRa->buffer[8];
	vlad.current_real2 = loRa->buffer[9];
	vlad.tone_level = loRa->buffer[10];
	vlad.tone_level2 = loRa->buffer[11];
	vlad.current = loRa->buffer[12];
	vlad.current2 = loRa->buffer[13];
	vlad.agc150m = loRa->buffer[14];
	vlad.level150m = loRa->buffer[15];
	vlad.agc170m = loRa->buffer[16];
	vlad.level170m = loRa->buffer[17];
	return vlad;
}

void modeCmdUpdate(UART1_t *uart1) {
	unsigned long receiveValue;
	receiveValue = 0;
	receiveValue = uart1_ptr->rxBuffer[4] << 8;
	receiveValue |= uart1_ptr->rxBuffer[5];
	if (receiveValue == 0) {
		HAL_GPIO_WritePin(MODE_GPIO_Port, MODE_Pin, GPIO_PIN_RESET);
	}
	if (receiveValue == 1) {
		HAL_GPIO_WritePin(MODE_GPIO_Port, MODE_Pin, GPIO_PIN_SET);
	}
}

void modeRs485Update(UART1_t *uart1, RS485_t *rs485, SX1278_t *loRa) {
	Vlad_t vlad;
	switch (rs485->cmd) {
	case QUERY_PARAMETERS_VLAD: //cmd = 11
		break;
	case SET_VLAD_MODE: //cmd = 12
		modeCmdUpdate(uart1_ptr);
		rs485->cmd = NONE;
		memset(rs485->buffer, 0, sizeof(rs485->buffer));
		rs485->len = 0;
		break;
	default:
		break;
	}

}

void print_parameters(UART1_t *u, Vlad_t vlad) {
	uint8_t len = 0;

	len = sprintf((char*) u->txBuffer, "vin %d[V]\r\n", vlad.vin);
	uart1_send_frame((char*) u->txBuffer, len);
	len = sprintf((char*) u->txBuffer, "vin2 %d[V]\r\n", vlad.vin2);
	uart1_send_frame((char*) u->txBuffer, len);
	len = sprintf((char*) u->txBuffer, "current real %d[A]\r\n",
			vlad.current_real);
	uart1_send_frame((char*) u->txBuffer, len);
	len = sprintf((char*) u->txBuffer, "current real2  %d[A]\r\n",
			vlad.current_real2);
	uart1_send_frame((char*) u->txBuffer, len);
	len = sprintf((char*) u->txBuffer, "tone level %d[dBm]\r\n",
			vlad.tone_level);
	uart1_send_frame((char*) u->txBuffer, len);
	len = sprintf((char*) u->txBuffer, "tone level2 %d[dBm]\r\n",
			vlad.tone_level2);
	uart1_send_frame((char*) u->txBuffer, len);
	len = sprintf((char*) u->txBuffer, "current %d[A]\r\n", vlad.current);
	uart1_send_frame((char*) u->txBuffer, len);
	len = sprintf((char*) u->txBuffer, "current2 %d[A]\r\n", vlad.current2);
	uart1_send_frame((char*) u->txBuffer, len);
	len = sprintf((char*) u->txBuffer, "agc150m %d[dBm]\r\n", vlad.agc150m);
	uart1_send_frame((char*) u->txBuffer, len);
	len = sprintf((char*) u->txBuffer, "level150m %d[dBm]\r\n", vlad.level150m);
	uart1_send_frame((char*) u->txBuffer, len);
	len = sprintf((char*) u->txBuffer, "agc170m %d[dBm]\r\n", vlad.agc170m);
	uart1_send_frame((char*) u->txBuffer, len);
	len = sprintf((char*) u->txBuffer, "level170m %d[dBm]\r\n", vlad.level170m);
	uart1_send_frame((char*) u->txBuffer, len);
	cleanRxBuffer(u);
}

void printStatus(UART1_t *uart1, Rs485_status_t status, RS485_t *rs485) {

	char rs485_msgs[11][30] = { "DATA OK", "START READING", "VALID FRAME",
			"NOT VALID FRAME", "WRONG MODULE FUNCTION", "WRONG MODULE ID",
			"CRC ERROR", "DONE", "WAITING", "VALID MODULE", "CHECK LORA DATA" };
	if(rs485->status == rs485->lastStatus)
		return;
	rs485->lastStatus = rs485->status;
	cleanByTimeout(uart1, rs485_msgs[status]);

}

void printLoRaStatus(UART1_t *uart1, SX1278_t *loRa) {
	SX1278_Status_t status = loRa->status;
	uint8_t len = 0;
	char *buff = uart1->txBuffer;
	/*if (loRa->status == loRa->lastStatus)
		return;*/
	loRa->lastStatus = loRa->status;
	if (status == TX_TIMEOUT) {
		len = sprintf(buff, "Transmission Fail: %d seconds Timeout\r\n",
				TX_TIMEOUT / 1000);
		uart1_send_frame(buff, len);
		return;
	}
	if (status == TX_DONE) {
		uint8_t bytesLen = loRa->len;
		uint32_t time = loRa->lastTxTime;
		len = sprintf(buff, "Transmission Done: %d ms %d bytes\r\n", time,
				bytesLen);
		uart1_send_frame(buff, len);
		return;
	}
	if (status == TX_READY) {
		len = sprintf(buff, "Master Mode\r\n");
		uart1_send_frame(buff, len);
		return;
	}
	if (status == RX_DONE) {
		uint8_t bytesLen = loRa->len;
		len = sprintf(buff, "Reception Done:  %d bytes\r\n", bytesLen);
		uart1_send_frame(buff, len);
		return;
	}
	if (status == RX_READY) {
		len = sprintf(buff, "Slave Mode\r\n");
		uart1_send_frame(buff, len);
		return;
	}
	if (status == CRC_ERROR_ACTIVATION) {
		len = sprintf(buff, "Reception Fail: Crc error activation\r\n");
		uart1_send_frame(buff, len);
		return;
	}
}

void setTxFifoData(SX1278_t *module) {
	setTxFifoAddr(module);
	for (int i = 0; i < module->len; i++) {
		uint8_t data = module->buffer[i];
		writeRegister(module->spi, 0x00, &data, 1);
	}
}

void waitForTxEnd(SX1278_t *loRa) {
	int timeStart = HAL_GetTick();
	while (1) {
		if (HAL_GPIO_ReadPin(LORA_BUSSY_GPIO_Port, LORA_BUSSY_Pin)) {
			int timeEnd = HAL_GetTick();
			loRa->lastTxTime = timeEnd - timeStart;
			readRegister(loRa->spi, LR_RegIrqFlags);
			clearIrqFlags(loRa);
			loRa->status = TX_DONE;
			return;
		}
		if (HAL_GetTick() - timeStart > LORA_SEND_TIMEOUT) {
			sx1278Reset();
			loRa->status = TX_TIMEOUT;
			return;
		}
		HAL_Delay(1);
	}
}

int changeModeBySwitch(int master, int valueRx, _Bool TX_MODE, _Bool RX_MODE,
		int *valueTx, SX1278_t *loRa) {
	if (master == 1) {
		valueRx = 0;
		TX_MODE = true;
		RX_MODE = false;
		*valueTx += 1;
		if (*valueTx == 1) {
			loRa->status = UNKNOW;
		}
	}
	if (master == 0) {
		*valueTx = 0;
		TX_MODE = false;
		RX_MODE = true;
		valueRx += 1;
		if (valueRx == 1) {
			loRa->status = UNKNOW;
		}
	}
	return valueRx;
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
	UART1_t uart1;
	Vlad_t vlad;
	vlad_ptr = &vlad;
	uart1_ptr = &uart1;
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
	vladInit(VLAD, ID1, &vlad);
	ledInit(&led);
	rs485Init(&rs485);
	//uart1Init(HS16_CLK, BAUD_RATE, &uart1);

	SX1278_hw_t lora_hw;
	SX1278_t loRa;
	lora_ptr = &loRa;

	lora_hw.dio0.port = LORA_BUSSY_GPIO_Port;
	lora_hw.dio0.pin = LORA_BUSSY_Pin;
	lora_hw.nss.port = LORA_NSS_GPIO_Port;
	lora_hw.nss.pin = LORA_NSS_Pin;
	lora_hw.reset.port = LORA_RST_GPIO_Port;
	lora_hw.reset.pin = LORA_RST_Pin;
	loRa.hw = &lora_hw;
	loRa.spi = &hspi1;
	HAL_GPIO_WritePin(LORA_NSS_GPIO_Port, LORA_NSS_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LORA_RST_GPIO_Port, LORA_RST_Pin, GPIO_PIN_SET);
	loRa.operatingMode = readRegister(&hspi1, LR_RegOpMode);
	loRa.status = UNKNOW;
	TX_MODE = true;
	RX_MODE = false;
	memset(loRa.buffer, 0, SX1278_MAX_PACKET);
	loRa.len = 0;
	int counter = HAL_GetTick();
	int change = 0;
	int master;
	int valueTx = 0;
	int valueRx = 0;
	HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(USART1_IRQn);
	HAL_UART_Receive_IT(&huart1, &rxData, 1);

	initLoRaParameters(&loRa, MASTER);
	writeLoRaParameters(&loRa);
	//initialize LoRa module

	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {

		master = HAL_GPIO_ReadPin(MODE_GPIO_Port, MODE_Pin);
		valueRx = changeModeBySwitch(master, valueRx, TX_MODE, RX_MODE,
				&valueTx, &loRa);

		if (uart1.len > 0) {
			rs485.len = uart1.len;
			memcpy(rs485.buffer, uart1.rxBuffer, uart1.len);

		} else if (loRa.len > 0) {
			rs485.len = loRa.len;
			memcpy(rs485.buffer, loRa.buffer, loRa.len);
		}

		if (rs485.len > 0) {
			checkBuffer(&rs485);
			if (rs485.status == DATA_OK) {
				rs485.cmd = rs485.buffer[3];
				rs485.status = WAITING;
				cleanRxBuffer(&uart1);
				memset(loRa.buffer, 0, sizeof(loRa.len));
				loRa.len = 0;
			}
		}

		printStatus(&uart1, rs485.status, &rs485);
		modeRs485Update(&uart1, &rs485, &loRa);

		if (rs485.cmd == QUERY_PARAMETERS_VLAD && rs485.len == 9) { //cmd = 11
			loRa.len = rs485.len;
			memcpy(loRa.buffer, rs485.buffer, rs485.len);

			updateMode(&loRa, MASTER);
			setTxFifoData(&loRa);
			updateLoraLowFreq(&loRa, TX);
			waitForTxEnd(&loRa);
			printLoRaStatus(&uart1, &loRa);

			memset(loRa.buffer, 0, sizeof(loRa.len));
			loRa.len = 0;
			rs485.cmd = NONE;
			memset(rs485.buffer, 0, sizeof(rs485.buffer));
			rs485.len = 0;
			change = 0;
			loRa.status = UNKNOW;

			updateMode(&loRa, SLAVE);
			printLoRaStatus(&uart1, &loRa);
			setRxFifoAddr(&loRa);
			updateLoraLowFreq(&loRa, RX_CONTINUOUS);
			clearMemForRx(&loRa);
			waitForRxDone(&loRa);
			getRxFifoData(&loRa);
			printLoRaStatus(&uart1, &loRa);
			vlad = decodeVLAD(&loRa);
			print_parameters(&uart1, vlad);
			memset(loRa.buffer, 0, sizeof(loRa.len));
			loRa.len = 0;
		}

		if (TX_MODE) {
			RX_MODE_OFF_LED();
			TX_MODE_ON_LED();
			if (change == 255)
				change = 0;
			loRa.buffer[change] = change;
			loRa.len = change;
			if (HAL_GetTick() - counter > 10000) {
				counter = HAL_GetTick();

				updateMode(&loRa, MASTER);
				setTxFifoData(&loRa);
				updateLoraLowFreq(&loRa, TX);
				waitForTxEnd(&loRa);
				printLoRaStatus(&uart1, &loRa);
				memset(loRa.buffer, 0, sizeof(loRa.len));
				loRa.len = 0;
				loRa.status = UNKNOW;
				rs485.cmd = NONE;
				memset(rs485.buffer, 0, sizeof(rs485.len));
				rs485.len = 0;

				change += 1;
			}
		}
		if (RX_MODE) {
			TX_MODE_OFF_LED();
			RX_MODE_ON_LED();
			read(&uart1, &loRa);
			rs485.len = loRa.len;
			memcpy(rs485.buffer, loRa.buffer, loRa.len);
			rs485.status = CHECK_LORA_DATA;
			change += 1;
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
