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
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);

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

SX1278_t *lora_ptr;

void setBaseParameters(SX1278_t *loraRx) {

	uint8_t dio0 = DIO0_RX_DONE;
	uint8_t dio1 = DIO1_RX_TIMEOUT;
	uint8_t dio2 = DIO2_FHSS_CHANGE_CHANNEL;
	uint8_t dio3 = DIO3_VALID_HEADER;
	////////////////////////////////////////
	uint8_t rxTimeoutMask = 0x00 | (MASK_DISABLE << 7);
	uint8_t rxDoneMask = 0x00 | (MASK_ENABLE << 6);
	uint8_t payloadCrcErrorMask = 0x00 | (MASK_DISABLE << 5);
	uint8_t validHeaderMask = 0x00 | (MASK_DISABLE << 4);
	uint8_t txDoneMask = 0x00 | (MASK_DISABLE << 3);
	uint8_t cadDoneMask = 0x00 | (MASK_DISABLE << 2);
	uint8_t fhssChangeChannelMask = 0x00 | (MASK_DISABLE << 1);
	uint8_t cadDetectedMask = 0x00 | (MASK_DISABLE << 0);

	loraRx->frequency = DOWNLINK_FREQ;
	loraRx->power = SX1278_POWER_17DBM;
	loraRx->LoRa_SF = SF_10;
	loraRx->LoRa_BW = LORABW_62_5KHZ;
	loraRx->LoRa_CR = LORA_CR_4_6;
	loraRx->LoRa_CRC_sum = CRC_ENABLE;
	loraRx->syncWord = LORAWAN;
	loraRx->ocp = OVERCURRENTPROTECT;
	loraRx->lnaGain = LNAGAIN;
	loraRx->AgcAutoOn = LNA_SET_BY_AGC;
	loraRx->symbTimeoutLsb = RX_TIMEOUT_LSB;
	loraRx->PreambleLengthMsb = PREAMBLE_LENGTH_MSB;
	loraRx->PreambleLengthLsb = PREAMBLE_LENGTH_LSB;
	loraRx->dioConfig = dio0 | dio1 | dio2 | dio3;
	loraRx->flagsMode = rxTimeoutMask | rxDoneMask | payloadCrcErrorMask;
	loraRx->flagsMode |= validHeaderMask | txDoneMask | cadDoneMask;
	loraRx->flagsMode |= fhssChangeChannelMask | cadDetectedMask;
	loraRx->fhssValue = HOPS_PERIOD;
}

void save(SX1278_t *module) {
	updateLoraLowFreq(module, SLEEP); //Change modem mode Must in Sleep mode
	HAL_Delay(15);
	//updateLoraLowFreq(module, STANDBY);
	setRFFrequency(module);
	setLORAWAN(module);
	setOutputPower(module);
	setOvercurrentProtect(module);
	writeRegister(module->spi, LR_RegLna, &(module->lnaGain), 1);
	if (module->LoRa_SF == SF_6) {
		//SFactor=6
		module->headerMode = IMPLICIT;
		module->symbTimeoutMsb = 0x03;
		setDetectionParameters(module);
	} else {
		module->headerMode = EXPLICIT;
		module->symbTimeoutMsb = 0x00;
	}
	setReModemConfig(module);
	setPreambleParameters(module);
	writeRegister(module->spi, LR_RegHopPeriod, &(module->fhssValue), 1); //RegHopPeriod NO FHSS
	writeRegister(module->spi, LR_RegDioMapping1, &(module->dioConfig), 1); //DIO0=01, DIO1=00,DIO2=00, DIO3=01
	clearIrqFlags(module);
	writeRegister(module->spi, LR_RegIrqFlagsMask, &(module->flagsMode), 1); //Open TxDone interrupt
}

void setRxParameters(SX1278_t *module) {
//	writeRegister(module->spi, LR_RegFifoTxBaseAddr, 0x80, 1);///
	uint8_t cmd = module->packetLength;
	writeRegister(module->spi, LR_RegPayloadLength, &(cmd), 1); //RegPayloadLength 21byte
	uint8_t addr = readRegister(module->spi, LR_RegFifoRxBaseAddr); //RegFiFoTxBaseAddr
	writeRegister(module->spi, LR_RegFifoAddrPtr, &addr, 1); //RegFifoAddrPtr
	uint8_t DireccionBaseTx = readRegister(module->spi, LR_RegFifoRxBaseAddr);
	uint8_t DireccionPtr = readRegister(module->spi, LR_RegFifoAddrPtr);
	module->packetLength = readRegister(module->spi, LR_RegPayloadLength);
}

void sx1278Reset() {
	HAL_GPIO_WritePin(LORA_NSS_GPIO_Port, LORA_NSS_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LORA_NSS_GPIO_Port, LORA_NSS_Pin, GPIO_PIN_RESET);
	HAL_Delay(1);
	HAL_GPIO_WritePin(LORA_NSS_GPIO_Port, LORA_NSS_Pin, GPIO_PIN_SET);
	HAL_Delay(100);
}

uint8_t DireccionBaseTx;
uint8_t DireccionPtr;
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
	//MX_I2C1_Init();
	MX_SPI1_Init();
	MX_USART1_UART_Init();
	//MX_USART2_UART_Init();
	//MX_USART3_UART_Init();
	MX_ADC1_Init();
	//MX_CRC_Init();
	/* USER CODE BEGIN 2 */
	/*
	 master = HAL_GPIO_ReadPin(MODE_GPIO_Port, MODE_Pin);
	 if (master == 1) {
	 printf("Mode: Master\r\n");
	 HAL_GPIO_WritePin(MODE_GPIO_Port, MODE_Pin, GPIO_PIN_RESET);
	 } else {
	 printf("Mode: Slave\r\n");
	 HAL_GPIO_WritePin(MODE_GPIO_Port, MODE_Pin, GPIO_PIN_SET);
	 }
	 */
	SX1278_hw_t lora_hw;
	SX1278_t loraTx, loraRx;
	lora_ptr = &loraRx;

	int messageCounter = 0;

	lora_hw.dio0.port = BUSSY_GPIO_Port;
	lora_hw.dio0.pin = BUSSY_Pin;
	lora_hw.nss.port = LORA_NSS_GPIO_Port;
	lora_hw.nss.pin = LORA_NSS_Pin;
	lora_hw.reset.port = LORA_RST_GPIO_Port;
	lora_hw.reset.pin = LORA_RST_Pin;
	loraRx.hw = &lora_hw;
	loraRx.spi = &hspi1;
	HAL_GPIO_WritePin(LORA_NSS_GPIO_Port, LORA_NSS_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LORA_RST_GPIO_Port, LORA_RST_Pin, GPIO_PIN_SET);
	loraRx.operatingMode = readRegister(&hspi1, LR_RegOpMode);
	loraRx.status = UNKNOW;
	//initialize LoRa module

	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {

		if (loraRx.status == UNKNOW) {
			uart_send("Configuring Master LoRa module\r\n");
			setBaseParameters(&loraRx);
			save(&loraRx);
			loraRx.status = RX_READY;
		}
		SX1278_t *module;
		module = &loraRx;
		if (loraRx.status == RX_READY) {

			memset(loraRx.rxBuffer, 0, SX1278_MAX_PACKET);
			setRxParameters(&loraRx);
		}
		updateLoraLowFreq(module, RX_SINGLE);
		if (SX1278_hw_GetDIO0(module->hw)) { //if(Get_NIRQ()) //Packet send over
			unsigned char addr;
			unsigned char packet_size;

			memset(loraRx.rxBuffer, 0, SX1278_MAX_PACKET);

			addr = readRegister(module->spi, LR_RegFifoRxCurrentaddr); //last packet addr
			writeRegister(module->spi, LR_RegFifoAddrPtr, &addr, 1); //RxBaseAddr -> FiFoAddrPtr

			if (module->LoRa_SF == SX1278_LORA_SF_6) { //When SpreadFactor is six,will used Implicit Header mode(Excluding internal packet length)
				packet_size = module->packetLength;
			} else {
				packet_size = SX1278_SPIRead(module, LR_RegRxNbBytes); //Number for received bytes
			}
			uint8_t buff[20] = { 0 };
			for (int i = 0; i < packet_size; i++) {

				buff[i] = readRegister(module->spi, LR_RegFifoRxBaseAddr);
			}
			uart_send(buff);
			module->readBytes = packet_size;
			SX1278_clearLoRaIrq(module);

			return module->readBytes;

			readRegister(module->spi, LR_RegIrqFlags);
			clearIrqFlags(module); //Clear irq
			uart_send("Transmission: OK\r\n");
			break;
		}
		//module->operatingMode = readMode(module);

		HAL_Delay(500);
		//}

		//	rssi_lora = SX1278_RSSI_LoRa(&SX1278);
		//	rssi = SX1278_RSSI(&SX1278);
		//	HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);
		//	version = SX1278_SPIRead(&SX1278, REG_LR_VERSION);
		//	sprintf(str,"LoRa Version %d...\r\n", version);
		//	uart_send(str);

		//	HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);
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
