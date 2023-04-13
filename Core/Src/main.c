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
#include "eeprom.h"
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

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_CRC_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
UART1_t *uart1_ptr;
Vlad_t *vlad_ptr;
RDSS_t *rs485_ptr;
SX1278_t *lora_ptr;

void USART1_IRQHandler(void);
void print_parameters(UART1_t *u, Vlad_t vlad);
void printStatus(UART1_t *u1, RDSS_t *rs485);
void printLoRaStatus(UART1_t *u1, SX1278_t *loRa);
void dinamicFrame(SX1278_t *loRa);
void parseLoRaMaster(RDSS_t *rs485, SX1278_t *loRa);
void parseLoRaSlave(RDSS_t *rs485, SX1278_t *loRa);
void parseUartMaster(UART1_t *u1, RDSS_t *rs485);
void parseUartSlave(UART1_t *u1, RDSS_t *rs485);
void sendQuery(RDSS_t *rs485, SX1278_t *loRa, UART1_t *u1);

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
	/* USER CODE BEGIN 1 */
	LED_t led;
	RDSS_t rdss;
	UART1_t u1;
	//Vlad_t vlad;
	SX1278_t loRa;

	lora_ptr = &loRa;
	uart1_ptr = &u1;
	//vlad_ptr = &vlad;
	rs485_ptr = &rdss;
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
	// MX_USART1_UART_Init();
	MX_CRC_Init();
	// MX_I2C1_Init();
	/* USER CODE BEGIN 2 */

	uart1Init(HS16_CLK, BAUD_RATE, &u1);
	u1.debug = false;
	i2c1MasterInit();
	rdssInit(&rdss, 6);

	ledInit(&led);

	HAL_GPIO_WritePin(LORA_NSS_GPIO_Port, LORA_NSS_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LORA_RST_GPIO_Port, LORA_RST_Pin, GPIO_PIN_SET);
	loRa.spi = &hspi1;
	loRa.operatingMode = readRegister(loRa.spi, LR_RegOpMode);
	loRa.mode = -1;

	readPage(CAT24C02_PAGE0_START_ADDR, &(loRa.spreadFactor), 0, 1);
	readPage(CAT24C02_PAGE0_START_ADDR, &(loRa.bandwidth), 1, 1);
	readPage(CAT24C02_PAGE0_START_ADDR, &(loRa.codingRate), 2, 1);
	readPage(CAT24C02_PAGE1_START_ADDR, (uint8_t*) &(loRa.upFreq), 0, 4);
	readPage(CAT24C02_PAGE1_START_ADDR, (uint8_t*) &(loRa.dlFreq), 4, 4);
	if (loRa.spreadFactor < SF_9 || loRa.spreadFactor > SF_12)
		loRa.spreadFactor = SF_10;

	if (loRa.bandwidth < LORABW_7_8KHZ || loRa.bandwidth > LORABW_500KHZ)
		loRa.bandwidth = LORABW_62_5KHZ;

	if (loRa.codingRate < LORA_CR_4_5 || loRa.codingRate > LORA_CR_4_8)
		loRa.codingRate = LORA_CR_4_6;

	if (loRa.upFreq < UPLINK_FREQ_MIN || loRa.upFreq > UPLINK_FREQ_MAX)
		loRa.upFreq = UPLINK_FREQ;

	if (loRa.dlFreq < DOWNLINK_FREQ_MIN || loRa.dlFreq > DOWNLINK_FREQ_MAX)
		loRa.dlFreq = DOWNLINK_FREQ;

	/*
	 savePage(CAT24C02_PAGE0_START_ADDR, &(loRa.spreadFactor),0, 1);
	 savePage(CAT24C02_PAGE0_START_ADDR, &(loRa.bandwidth),1, 1);
	 savePage(CAT24C02_PAGE0_START_ADDR, &(loRa.codingRate),2, 1);
	 savePage(CAT24C02_PAGE1_START_ADDR, (uint8_t*)&(loRa.upFreq),0, 4);
	 savePage(CAT24C02_PAGE1_START_ADDR, (uint8_t*)&(loRa.dlFreq),4, 4);
	 */
	initLoRaParameters(&loRa, SLAVE_RECEIVER);
	writeLoRaParameters(&loRa);
	printLoRaStatus(&u1, &loRa);

	//initialize LoRa module

	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {

		if (rdss.status == LORA_RECEIVE) {
			uint8_t crc_frame[2];
			uint16_t crc;
			uint8_t data_length;
			loRa.buffer[0] = LTEL_START_MARK;
			loRa.buffer[1] = VLADR;
			loRa.buffer[2] = rdss.id;
			loRa.buffer[3] = rdss.cmd;
			loRa.buffer[4] = 0x00;
			if (rdss.cmd == QUERY_STATUS && rdss.len == 9) {
				data_length = 12;
				uint16_t lineVoltage = rand() % 610;
				uint16_t baseCurrent = rand() % 301;
				uint16_t tunnelCurrent = rand() % 1001;
				uint16_t unitCurrent = rand() % 301;
				uint8_t uplinkAgc = rand() % 43;
				uint8_t downlinkInputPower = rand() % 256;
				uint8_t downlinkAgc = rand() % 43;
				uint8_t uplinkOuputPower = rand() % 256;

				loRa.buffer[5] = data_length;
				loRa.buffer[7] = (lineVoltage >> 8) & 0xFF;
				loRa.buffer[6] = lineVoltage & 0xFF;
				loRa.buffer[9] = (baseCurrent >> 8) & 0xFF;
				loRa.buffer[8] = baseCurrent & 0xFF;
				loRa.buffer[11] = (tunnelCurrent >> 8) & 0xFF;
				loRa.buffer[10] = tunnelCurrent & 0xFF;
				loRa.buffer[13] = (unitCurrent >> 8) & 0xFF;
				loRa.buffer[12] = unitCurrent & 0xFF;
				loRa.buffer[14] = uplinkAgc;
				loRa.buffer[15] = downlinkInputPower;
				loRa.buffer[16] = downlinkAgc;
				loRa.buffer[17] = uplinkOuputPower;

				crc = crc_get(&(loRa.buffer[1]), 17);
				memcpy(crc_frame, &crc, 2);
				loRa.buffer[18] = crc_frame[0];
				loRa.buffer[19] = crc_frame[1];
				loRa.buffer[20] = LTEL_END_MARK;
				loRa.len = 21;
				loRa.status = TX_BUFFER_READY;
				printLoRaStatus(&u1, &loRa);
				updateMode(&loRa, SLAVE_SENDER);
				printStatus(&u1, &rdss);
				transmit(&loRa);
				printLoRaStatus(&u1, &loRa);
			}
			reinit(&rdss);
			updateMode(&loRa, SLAVE_RECEIVER);

			printLoRaStatus(&u1, &loRa);
		} else if (rdss.status == UART_VALID) {
			uint8_t crc_frame[2];
			uint16_t crc;
			uint8_t i = 0;
			u1.tx[i++] = LTEL_START_MARK;
			u1.tx[i++] = VLADR;
			u1.tx[i++] = rdss.id;
			u1.tx[i++] = rdss.cmd;
			u1.tx[i++] = 0x00;

			if (rdss.cmd == QUERY_STATUS && rdss.len == 9) {
				uint16_t lineVoltage = rand() % 610;
				uint16_t baseCurrent = rand() % 301;
				uint16_t tunnelCurrent = rand() % 1001;
				uint16_t unitCurrent = rand() % 301;
				uint8_t uplinkAgc = rand() % 43;
				uint8_t downlinkInputPower = rand() % 256;
				uint8_t downlinkAgc = rand() % 43;
				uint8_t uplinkOuputPower = rand() % 256;

				u1.tx[i++] = 12;
				u1.tx[i++] = (lineVoltage >> 8) & 0xFF;
				u1.tx[i++] = lineVoltage & 0xFF;
				u1.tx[i++] = (baseCurrent >> 8) & 0xFF;
				u1.tx[i++] = baseCurrent & 0xFF;
				u1.tx[i++] = (tunnelCurrent >> 8) & 0xFF;
				u1.tx[i++] = tunnelCurrent & 0xFF;
				u1.tx[i++] = (unitCurrent >> 8) & 0xFF;
				u1.tx[i++] = unitCurrent & 0xFF;
				u1.tx[i++] = uplinkAgc;
				u1.tx[i++] = downlinkInputPower;
				u1.tx[i++] = downlinkAgc;
				u1.tx[i++] = uplinkOuputPower;

			} else if (rdss.cmd == QUERY_RX_FREQ) {
				union floatConverter txFreq;
				txFreq.f = loRa.dlFreq / 1000000.0f;
				u1.tx[i++] = 4;
				memcpy(u1.tx + 6, &txFreq.i, sizeof(txFreq.i));
				i += sizeof(txFreq.i);
				i++;
			} else if (rdss.cmd == QUERY_TX_FREQ) {
				union floatConverter rxFreq;
				rxFreq.f = loRa.upFreq / 1000000.0f;
				u1.tx[i++] = 4;
				memcpy(u1.tx + 6, &rxFreq.i, sizeof(rxFreq.i));
				i += sizeof(rxFreq.i);
				i++;
			} else if (rdss.cmd == QUERY_SPREAD_FACTOR) {

			} else if (rdss.cmd == QUERY_CODING_RATE) {

			} else if (rdss.cmd == QUERY_BANDWIDTH) {

			} else if (rdss.cmd == QUERY_MODULE_ID) {

			} else if (rdss.cmd == SET_TX_FREQ) {

			} else if (rdss.cmd == SET_RX_FREQ) {

			} else if (rdss.cmd == SET_BANDWIDTH) {

			} else if (rdss.cmd == SET_SPREAD_FACTOR) {

			} else if (rdss.cmd == QUERY_CODING_RATE) {

			}
			crc = crc_get(&(u1.tx[1]), i - 1);
			memcpy(crc_frame, &crc, 2);
			u1.tx[i++] = crc_frame[0];
			u1.tx[i++] = crc_frame[1];
			u1.tx[i++] = LTEL_END_MARK;
			u1.txLen = i;
			rdss.status = UART_SEND;
			writeTx(&u1);
			printStatus(&u1, &rdss);
			reinit(&rdss);
			cleanTx(&u1);
			u1.txLen = 0;
		} else if (rdss.status == LORA_SEND) {
			printStatus(&u1, &rdss);
			if (rdss.cmd == QUERY_STATUS)
				sendQuery(&rdss, &loRa, &u1);
			printLoRaStatus(&u1, &loRa);
			reinit(&rdss);
			updateMode(&loRa, SLAVE_RECEIVER);
		} else if (u1.isReady) {
			parseUartSlave(&u1, &rdss);
			printStatus(&u1, &rdss);
		} else if (loRa.len > 0) {
			parseLoRaSlave(&rdss, &loRa);
			printStatus(&u1, &rdss);
			printLoRaStatus(&u1, &loRa);
		} else if (loRa.mode == SLAVE_RECEIVER) {
			if (loRa.operatingMode != RX_CONTINUOUS) {
				updateMode(&loRa, SLAVE_RECEIVER);
				setRxFifoAddr(&loRa);
				updateLoraLowFreq(&loRa, RX_CONTINUOUS);
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
				readOperatingMode(&loRa);
				loRa.status = RX_MODE;
				printLoRaStatus(&u1, &loRa);
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

}

/* USER CODE BEGIN 4 */
void USART1_IRQHandler(void) {
	if (uart1_ptr->rxLen >= RX_BUFFLEN) {
		cleanRx(uart1_ptr);
		uart1_ptr->rxLen = 0;
	}
	uart1_ptr->rx[uart1_ptr->rxLen++] = readRxReg();
	if (uart1_ptr->rx[uart1_ptr->rxLen - 1] == LTEL_END_MARK)
		uart1_ptr->isReady = true;
	if (uart1_ptr->rx[0] != LTEL_START_MARK) {
		cleanRx(uart1_ptr);
		uart1_ptr->rxLen = 0;
	}

}

void print_parameters(UART1_t *u, Vlad_t vlad) {
	char *buff = (char*) u->tx;
	u->txLen = sprintf(buff, "vin %d [V]\r\n", vlad.vin);
	writeTx(u);
	//u->txLen = sprintf(buff, "vin2 %d[V]\r\n", vlad.vin2);
	//writeTx(u);
	u->txLen = sprintf(buff, "current real %d [A]\r\n",
			(uint8_t) vlad.current_real);
	writeTx(u);
	//u->txLen = sprintf(buff, "current real2  %d[A]\r\n", vlad.current_real2);
	//writeTx(u);
	u->txLen = sprintf(buff, "tone level %d[dBm]\r\n", vlad.tone_level);
	writeTx(u);
	//u->txLen = sprintf(buff, "tone level2 %d[dBm]\r\n", vlad.tone_level2);
	//writeTx(u);
	u->txLen = sprintf(buff, "current %d[A]\r\n", vlad.current);
	writeTx(u);
	//u->txLen = sprintf(buff, "current2 %d[A]\r\n", vlad.current2);
	//writeTx(u);
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

void printStatus(UART1_t *u1, RDSS_t *rs485) {

	if (!u1->debug)
		return;
	char *str = (char*) u1->tx;
	uint8_t i = 0;
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
		u1->txLen = sprintf(str, "Not valid start byte \r\n");
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
		u1->txLen = sprintf(str,
				"Validation ok: ID %02x Cmd %02x Bytes %d Data \r\n",
				rs485->buffer[2], rs485->buffer[3], rs485->buffer[5]);
		writeTx(u1);
		for (i = DATA_START_INDEX; i < rs485->buffer[5]; i++) {
			if (i > 250)
				break;
			u1->txLen = sprintf(str, "%02X", rs485->buffer[i]);
			writeTx(u1);
		}
		if (i > DATA_START_INDEX)
			writeTxReg('\n');
		u1->txLen = 0;
		break;
	case UART_SEND:
		u1->txLen = sprintf(str, "Reply vlad data: %d\r\n", rs485->idReceived);
		writeTx(u1);
		for (i = 0; i < rs485->len; i++) {
			if (i > 250)
				break;
			u1->txLen = sprintf(str, "%02X", rs485->buffer[i]);
			writeTx(u1);
		}
		if (i > 0)
			writeTxReg('\n');
		u1->txLen = 0;
		break;
	case UART_VALID:
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

	if (!u1->debug)
		return;
	char *str = (char*) u1->tx;
	switch (loRa->status) {
	case TX_TIMEOUT:
		u1->txLen = sprintf(str, "Transmission Fail: %d seconds Timeout\r\n",
				TX_TIMEOUT / 1000);
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
		if (loRa->len > 0)
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

	loRa->len = len + 9;
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

void parseLoRaSlave(RDSS_t *rs485, SX1278_t *loRa) {
	fillValidBuffer(&*rs485, loRa->buffer, loRa->len);
	memset(loRa->buffer, 0, sizeof(loRa->buffer));
	loRa->len = 0;
	if (rs485->status == DATA_OK) {
		rs485->cmd = rs485->buffer[CMD_INDEX];
		rs485->idReceived = rs485->buffer[MODULE_ID_INDEX];
		if (rs485->idReceived == rs485->id)
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

void parseUartSlave(UART1_t *u1, RDSS_t *rs485) {
	u1->isReady = false;
	fillValidBuffer(&*rs485, u1->rx, u1->rxLen);
	memset(u1->rx, 0, sizeof(u1->rx));
	u1->rxLen = 0;
	if (rs485->status == DATA_OK) {
		rs485->cmd = rs485->buffer[CMD_INDEX];
		rs485->idReceived = rs485->buffer[MODULE_ID_INDEX];
		if (rs485->idReceived == rs485->id)
			rs485->status = UART_VALID;
		else
			rs485->status = WRONG_MODULE_ID;
//	rs485->status = LORA_SEND;
	}
}

void sendQuery(RDSS_t *rs485, SX1278_t *loRa, UART1_t *u1) {
	if (rs485->len != 9)
		return;
	rs485->idQuery = rs485->buffer[MODULE_ID_INDEX];
	loRa->len = rs485->len;
	updateMode(&*loRa, MASTER_SENDER);
	memcpy(loRa->buffer, rs485->buffer, rs485->len);
	printLoRaStatus(&*u1, &*loRa);
	transmit(&*loRa);
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
