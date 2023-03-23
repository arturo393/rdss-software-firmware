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
	uart1_read_to_frame(uart1_ptr);
}

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

void setRxBaseParameters(SX1278_t *loRa) {

	uint8_t dio0 = DIO0_RX_DONE;
	uint8_t dio1 = DIO1_RX_TIMEOUT;
	uint8_t dio2 = DIO2_FHSS_CHANGE_CHANNEL;
	uint8_t dio3 = DIO3_VALID_HEADER;
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	loRa->frequency = DOWNLINK_FREQ;
	loRa->power = SX1278_POWER_17DBM;
	loRa->LoRa_SF = SF_10;
	loRa->LoRa_BW = LORABW_62_5KHZ;
	loRa->LoRa_CR = LORA_CR_4_6;
	loRa->LoRa_CRC_sum = CRC_ENABLE;
	loRa->syncWord = LORAWAN;
	loRa->ocp = OVERCURRENTPROTECT;
	loRa->lnaGain = LNAGAIN;
	loRa->AgcAutoOn = LNA_SET_BY_AGC;
	loRa->symbTimeoutLsb = RX_TIMEOUT_LSB;
	loRa->preambleLengthMsb = PREAMBLE_LENGTH_MSB;
	loRa->preambleLengthLsb = PREAMBLE_LENGTH_LSB;
	loRa->dioConfig = dio0 | dio1 | dio2 | dio3;

	loRa->flagsMode = 0xff;
	CLEAR_BIT(loRa->flagsMode, RX_DONE_MASK);
	CLEAR_BIT(loRa->flagsMode, PAYLOAD_CRC_ERROR_MASK);

	loRa->fhssValue = HOPS_PERIOD;
	loRa->len = SX1278_MAX_PACKET;
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
	if (loRa->currentStatus == RX_READY) {
		memset(loRa->buffer, 0, sizeof(loRa->buffer));
		loRa->len = 0;
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
		if (HAL_GetTick() - timeout > 2000)
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

void getRxFifoData(SX1278_t *lora) {
	GPIO_PinState bussy = HAL_GPIO_ReadPin(BUSSY_GPIO_Port, BUSSY_Pin);
	if (bussy == GPIO_PIN_RESET) {
		lora->currentStatus = RX_READY;
		return;
	}
	if (crcErrorActivation(lora) == 1) {
		lora->currentStatus = CRC_ERROR_ACTIVATION;
		return;
	}

	lora->len = readRegister(lora->spi, LR_RegRxNbBytes); //Number for received bytes
	uint8_t addr = 0x00;
	HAL_GPIO_WritePin(GPIOB, LORA_NSS_Pin, GPIO_PIN_RESET); // pull the pin low
	HAL_Delay(1);
	HAL_SPI_Transmit(lora->spi, &addr, 1, 100); // send address
	HAL_SPI_Receive(lora->spi, lora->buffer, lora->len, 100); // receive 6 bytes data
	HAL_Delay(1);
	HAL_GPIO_WritePin(GPIOB, LORA_NSS_Pin, GPIO_PIN_SET); // pull the pin high
	lora->currentStatus = RX_DONE;
}

void printParameters(int timeRx, const UART1_t *uart1, SX1278_t *loRa) {
	if (loRa->len == 0)
		return;
	for (int i = 0; i < loRa->len; i++) {
		uint8_t len = sprintf(uart1->txBuffer, "%02X", loRa->buffer[i]);
		uart1_send_frame(uart1->txBuffer, len);
	}
	sprintf(uart1->txBuffer, " - Rx Ok: %d ms %d bytes\n", timeRx, loRa->len);
	uart1_send_frame(uart1->txBuffer, TX_BUFFLEN);
}

bool TX_MODE;
bool RX_MODE;

void modeCmdUpdate(const UART1_t *uart1, SX1278_t *loRa) {
	unsigned long receiveValue;
	receiveValue = 0;
	receiveValue = uart1->rxBuffer[4] << 8;
	receiveValue |= uart1->rxBuffer[5];
	if (receiveValue == 0) {
		RX_MODE = true;
		TX_MODE = false;
		loRa->currentStatus = UNKNOW;
	}
	if (receiveValue == 1) {
		TX_MODE = true;
		RX_MODE = false;
		loRa->currentStatus = UNKNOW;
	}
}

void encodeVLAD(uint8_t *frame) {
	uint8_t crc_frame[2];
	uint16_t crc;
	uint8_t data_length = 12;

	frame[0] = LTEL_START_MARK;
	frame[1] = VLAD;
	frame[2] = ID1;
	frame[3] = QUERY_PARAMETERS_VLAD;
	frame[4] = 0x00;
	frame[5] = data_length;
	frame[6] = 0x00;
	frame[7] = 12;
	frame[8] = 0x00;
	frame[9] = 200;
	frame[10] = 0x00;
	frame[11] = 230;
	frame[12] = 0x00;
	frame[13] = 112;
	frame[14] = 10;
	frame[15] = 4;
	frame[16] = 5;
	frame[17] = 6;

	crc = crc_get(&(frame[1]), 17);
	memcpy(crc_frame, &crc, 2);
	frame[18] = crc_frame[0];
	frame[19] = crc_frame[1];
	frame[20] = LTEL_END_MARK;

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

void printStatus(UART1_t *uart1, Rs485_status_t status, RS485_t *rs485) {
	char rs485_msgs[11][30] = { "DATA OK", "START READING", "VALID FRAME",
			"NOT VALID FRAME", "WRONG MODULE FUNCTION", "WRONG MODULE ID",
			"CRC ERROR", "DONE", "WAITING", "VALID MODULE", "CHECK LORA DATA" };
	if (rs485->status == rs485->lastStatus)
		return;
	rs485->lastStatus = rs485->status;
	cleanByTimeout(uart1, rs485_msgs[status]);
}

void printLoRaStatus(UART1_t *uart1, SX1278_t *loRa) {
	SX1278_Status_t status = loRa->currentStatus;
	uint8_t len = 0;
	char *buff = uart1->txBuffer;

	if (loRa->currentStatus == loRa->lastStatus)
		return;
	loRa->lastStatus = loRa->currentStatus;
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
		if (HAL_GPIO_ReadPin(BUSSY_GPIO_Port, BUSSY_Pin)) {
			int timeEnd = HAL_GetTick();
			loRa->lastTxTime = timeEnd - timeStart;
			readRegister(loRa->spi, LR_RegIrqFlags);
			clearIrqFlags(loRa);
			loRa->currentStatus = TX_DONE;
			return;
		}
		if (HAL_GetTick() - timeStart > LORA_SEND_TIMEOUT) {
			sx1278Reset();
			loRa->currentStatus = TX_TIMEOUT;
			return;
		}
		HAL_Delay(1);
	}

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
	uart1_ptr = &uart1;
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
	//MX_USART1_UART_Init();
	MX_CRC_Init();
	/* USER CODE BEGIN 2 */
	vladInit(VLAD, ID1, &vlad);
	rs485Init(&rs485);
	uart1Init(HS16_CLK, BAUD_RATE, &uart1);
	ledInit(&led);

	SX1278_hw_t lora_hw;
	SX1278_t loRa;

	lora_ptr = &loRa;

	lora_hw.dio0.port = BUSSY_GPIO_Port;
	lora_hw.dio0.pin = BUSSY_Pin;
	lora_hw.nss.port = LORA_NSS_GPIO_Port;
	lora_hw.nss.pin = LORA_NSS_Pin;
	lora_hw.reset.port = LORA_RST_GPIO_Port;
	lora_hw.reset.pin = LORA_RST_Pin;
	loRa.hw = &lora_hw;
	loRa.spi = &hspi1;
	HAL_GPIO_WritePin(LORA_NSS_GPIO_Port, LORA_NSS_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LORA_RST_GPIO_Port, LORA_RST_Pin, GPIO_PIN_SET);
	loRa.operatingMode = readRegister(&hspi1, LR_RegOpMode);
	loRa.currentStatus = UNKNOW;
	loRa.len = 0;
	TX_MODE = false;
	RX_MODE = true;

	initLoRaParameters(&loRa, SLAVE);
	writeLoRaParameters(&loRa);

	int counter = HAL_GetTick();
	int change = 0;
	Rs485_status_t status;
	//initialize LoRa module

	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		if (uart1.len > 0) {
			rs485.len = uart1.len;
			memcpy(rs485.buffer, uart1.rxBuffer, uart1.len);

		} else if (loRa.len > 0) {
			rs485.len = loRa.len;
			memcpy(rs485.buffer, loRa.buffer, loRa.len);
		}

		if (rs485.len > 0) {
			status = checkBuffer(&rs485);
			if (rs485.status == DATA_OK) {
				rs485.cmd = rs485.buffer[3];
				rs485.status = WAITING;
				cleanRxBuffer(&uart1);
				memset(loRa.buffer, 0, sizeof(loRa.len));
				loRa.len = 0;
			}
		}
		if (status != rs485.status)
			printStatus(&uart1, rs485.status, &rs485);
		modeRs485Update(&uart1, &rs485, &loRa);
		if (rs485.cmd == QUERY_PARAMETERS_VLAD) {
			uint8_t frame[21] = { 0 };
			encodeVLAD(frame);
			memcpy(loRa.buffer, frame, 21);
			loRa.len = 21;
			updateMode(&loRa, MASTER);
			printLoRaStatus(&uart1, &loRa);
			setTxFifoData(&loRa);
			updateLoraLowFreq(&loRa, TX);
			waitForTxEnd(&loRa);

			memset(loRa.buffer, 0, sizeof(loRa.len));
			loRa.len = 0;
			loRa.currentStatus = UNKNOW;
			change = 0;
			rs485.cmd = NONE;
			memset(rs485.buffer, 0, sizeof(rs485.len));
			rs485.len = 0;
		}

		if (RX_MODE) {
			if (loRa.operatingMode != RX_CONTINUOUS) {
				updateMode(&loRa, SLAVE);
				setRxFifoAddr(&loRa);
				updateLoraLowFreq(&loRa, RX_CONTINUOUS);
			}
			clearMemForRx(&loRa);
			getRxFifoData(&loRa);
			if (loRa.currentStatus == RX_DONE) {
				setRxFifoAddr(&loRa);
				updateLoraLowFreq(&loRa, RX_CONTINUOUS);
				readOperatingMode(&loRa);
			}
			printLoRaStatus(&uart1, &loRa);
		}
		if (TX_MODE) {
			if (change == 255)
				change = 0;
			loRa.buffer[change] = change;
			loRa.len = change;

			if (HAL_GetTick() - counter > 10000) {
				counter = HAL_GetTick();

				change += 1;
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
	hadc1.Init.DiscontinuousConvMode = DISABLE;
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
	hcrc.Init.DefaultPolynomialUse = DEFAULT_POLYNOMIAL_ENABLE;
	hcrc.Init.DefaultInitValueUse = DEFAULT_INIT_VALUE_ENABLE;
	hcrc.Init.InputDataInversionMode = CRC_INPUTDATA_INVERSION_NONE;
	hcrc.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_DISABLE;
	hcrc.InputDataFormat = CRC_INPUTDATA_FORMAT_BYTES;
	if (HAL_CRC_Init(&hcrc) != HAL_OK) {
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

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOB, LORA_NSS_Pin | LORA_RST_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(DIO1_GPIO_Port, DIO1_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(BUSSY_GPIO_Port, BUSSY_Pin, GPIO_PIN_RESET);

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

	/*Configure GPIO pin : BUSSY_Pin */
	GPIO_InitStruct.Pin = BUSSY_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(BUSSY_GPIO_Port, &GPIO_InitStruct);

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
