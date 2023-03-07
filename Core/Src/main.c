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
void setTxBaseParameters(SX1278_t *loraTx) {

	uint8_t dio0 = DIO0_TX_DONE;
	uint8_t dio1 = DIO1_RX_TIMEOUT;
	uint8_t dio2 = DIO2_FHSS_CHANGE_CHANNEL;
	uint8_t dio3 = DIO3_VALID_HEADER;

	uint8_t rxTimeoutMask = 0x00 | (MASK_DISABLE << 7);
	uint8_t rxDoneMask = 0x00 | (MASK_DISABLE << 6);
	uint8_t payloadCrcErrorMask = 0x00 | (MASK_DISABLE << 5);
	uint8_t validHeaderMask = 0x00 | (MASK_DISABLE << 4);
	uint8_t txDoneMask = 0x00 | (MASK_ENABLE << 3);
	uint8_t cadDoneMask = 0x00 | (MASK_DISABLE << 2);
	uint8_t fhssChangeChannelMask = 0x00 | (MASK_DISABLE << 1);
	uint8_t cadDetectedMask = 0x00 | (MASK_DISABLE << 0);

	loraTx->frequency = UPLINK_FREQ;
	loraTx->power = SX1278_POWER_17DBM;
	loraTx->LoRa_SF = SF_10;
	loraTx->LoRa_BW = LORABW_62_5KHZ;
	loraTx->LoRa_CR = LORA_CR_4_6;
	loraTx->LoRa_CRC_sum = CRC_ENABLE;
	loraTx->syncWord = LORAWAN;
	loraTx->ocp = OVERCURRENTPROTECT;
	loraTx->lnaGain = LNAGAIN;
	loraTx->AgcAutoOn = LNA_SET_BY_AGC;
	loraTx->symbTimeoutLsb = RX_TIMEOUT_LSB;
	loraTx->preambleLengthMsb = PREAMBLE_LENGTH_MSB;
	loraTx->preambleLengthLsb = PREAMBLE_LENGTH_LSB;
	loraTx->dioConfig = dio0 | dio1 | dio2 | dio3;
	loraTx->flagsMode = rxTimeoutMask | rxDoneMask | payloadCrcErrorMask;
	loraTx->flagsMode |= validHeaderMask | txDoneMask | cadDoneMask;
	loraTx->flagsMode |= fhssChangeChannelMask | cadDetectedMask;
	loraTx->fhssValue = HOPS_PERIOD;
}

void setRxBaseParameters(SX1278_t *loraRx) {

	uint8_t dio0 = DIO0_RX_DONE;
	uint8_t dio1 = DIO1_RX_TIMEOUT;
	uint8_t dio2 = DIO2_FHSS_CHANGE_CHANNEL;
	uint8_t dio3 = DIO3_VALID_HEADER;
	////////////////////////////////////////
	uint8_t rxTimeoutMask = 0x00 | (MASK_ENABLE << 7);
	uint8_t rxDoneMask = 0x00 | (MASK_ENABLE << 6);
	uint8_t payloadCrcErrorMask = 0x00 | (MASK_DISABLE << 5);
	uint8_t validHeaderMask = 0x00 | (MASK_DISABLE << 4);
	uint8_t txDoneMask = 0x00 | (MASK_DISABLE << 3);
	uint8_t cadDoneMask = 0x00 | (MASK_DISABLE << 2);
	uint8_t fhssChangeChannelMask = 0x00 | (MASK_DISABLE << 1);
	uint8_t cadDetectedMask = 0x00 | (MASK_DISABLE << 0);

	loraRx->frequency = UPLINK_FREQ;
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
	loraRx->preambleLengthMsb = PREAMBLE_LENGTH_MSB;
	loraRx->preambleLengthLsb = PREAMBLE_LENGTH_LSB;
	loraRx->dioConfig = dio0 | dio1 | dio2 | dio3;
	loraRx->flagsMode = rxTimeoutMask | rxDoneMask | payloadCrcErrorMask;
	loraRx->flagsMode |= validHeaderMask | txDoneMask | cadDoneMask;
	loraRx->flagsMode |= fhssChangeChannelMask | cadDetectedMask;
	loraRx->fhssValue = HOPS_PERIOD;
	loraRx->packetLength = SX1278_MAX_PACKET;
}

void saveRx(SX1278_t *module) {
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
	updateLoraLowFreq(module, SLEEP); //Change modem mode Must in Sleep mode
	uint8_t cmd = module->packetLength;
	writeRegister(module->spi, LR_RegPayloadLength, &(cmd), 1); //RegPayloadLength 21byte
	uint8_t addr = readRegister(module->spi, LR_RegFifoRxBaseAddr); //RegFiFoTxBaseAddr
	writeRegister(module->spi, LR_RegFifoAddrPtr, &addr, 1); //RegFifoAddrPtr
	module->packetLength = readRegister(module->spi, LR_RegPayloadLength);
}

void sx1278Reset() {
	HAL_GPIO_WritePin(LORA_NSS_GPIO_Port, LORA_NSS_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LORA_NSS_GPIO_Port, LORA_NSS_Pin, GPIO_PIN_RESET);
	HAL_Delay(1);
	HAL_GPIO_WritePin(LORA_NSS_GPIO_Port, LORA_NSS_Pin, GPIO_PIN_SET);
	HAL_Delay(100);
}

void read(UART_HandleTypeDef *huart1, SX1278_t *loraRx) {
	if (loraRx->status == UNKNOW) {
		uint8_t tmp[] = "Configuring Slave LoRa module: Rx Mode\r\n";
		size_t len = strlen(tmp);
		HAL_UART_Transmit(&*huart1, tmp, len, 100);
		setRxBaseParameters(&*loraRx);
		saveRx(loraRx);
		loraRx->status = RX_READY;
	}
	if (loraRx->status == RX_READY) {
		memset(loraRx->rxBuffer, 0, SX1278_MAX_PACKET);
		setRxParameters(loraRx);
	}
	updateLoraLowFreq(&*loraRx, RX_SINGLE);
	while (!SX1278_hw_GetDIO0(loraRx->hw)) {
		uint8_t flags = readRegister(loraRx->spi, LR_RegIrqFlags);
		if (flags && (MASK_DISABLE << 7)) {
			uint8_t cmd = flags | (1 << 7);
			writeRegister(loraRx->spi, LR_RegIrqFlags, &cmd, 1);
			flags = readRegister(loraRx->spi, LR_RegIrqFlags);
			updateLoraLowFreq(&*loraRx, RX_SINGLE);
		}
	}; //if(Get_NIRQ()) //Packet send over
	loraRx->operatingMode = readMode(loraRx);
	loraRx->packetLength = readRegister(loraRx->spi, LR_RegRxNbBytes); //Number for received bytes
	uint8_t addr = 0x00;
	HAL_GPIO_WritePin(GPIOB, LORA_NSS_Pin, GPIO_PIN_RESET); // pull the pin low
	HAL_Delay(1);
	HAL_SPI_Transmit(loraRx->spi, &addr, 1, 100);  // send address
	HAL_SPI_Receive(loraRx->spi, loraRx->rxBuffer, loraRx->packetLength, 100); // receive 6 bytes data
	HAL_Delay(1);
	HAL_GPIO_WritePin(GPIOB, LORA_NSS_Pin, GPIO_PIN_SET); // pull the pin high

	HAL_UART_Transmit(huart1, loraRx->rxBuffer, loraRx->packetLength, 100);
	clearIrqFlags(loraRx); //Clear irq
	uint8_t tmp[] = " -> Reception OK\r\n";
	uint16_t len = strlen(tmp);
	HAL_UART_Transmit(huart1, tmp, len, 100);
	loraRx->operatingMode = readMode(loraRx);
}
int messageCounter = 0;

void transmit(SX1278_t *loraTx){
		if (loraTx->status == UNKNOW) {
			//printf("Configuring Slave LoRa module: Tx Mode\r\n");
			uint8_t tmp[] = "Configuring Slave LoRa module: Rx Mode\r\n";
			size_t len = strlen(tmp);
			HAL_UART_Transmit(&huart1, tmp, len, 100);
			setTxBaseParameters(loraTx);
			saveTx(loraTx);
			loraTx->status = TX_READY;
		}
		if (loraTx->status == TX_READY) {
			memset(loraTx->txBuffer, 0, SX1278_MAX_PACKET);
			loraTx->packetLength = sprintf((char*) loraTx->txBuffer, "Adios tia Paty, Adios tia Lela, "
					"saludos al loco Rene, al compadre moncho y "
					"al vendedor de leña %d",
					messageCounter);

			setTxParameters(loraTx);
			if (!(loraTx->packetLength > 0))
				return;
			//printf("EntryTx OK!\r\n");
			uint8_t tmp1[]= "EntryTx OK!\r\n";
			size_t len1 = strlen(tmp1);
			HAL_UART_Transmit(&huart1, tmp1, len1, 100);
			//printf("Sending message: %s\r\n", loraTx->txBuffer);
			uint8_t tmp2[]= "Sending message: \r\n";
			size_t len2 = strlen(tmp2);
			HAL_UART_Transmit(&huart1, tmp2, len2, 100);
			HAL_UART_Transmit(&huart1, loraTx->txBuffer, loraTx->packetLength, 100);

			for(int i = 0;i<loraTx->packetLength;i++){
				char data = loraTx->txBuffer[i];
				writeRegister(loraTx->spi, 0x00, &data,1);
		}
			updateLoraLowFreq(loraTx, TX);
			uint32_t timeStart = HAL_GetTick();
			while (1) {
				if (SX1278_hw_GetDIO0(loraTx->hw)) {
					readRegister(loraTx->spi, LR_RegIrqFlags);
					clearIrqFlags(loraTx);
					//printf("Transmission: OK\r\n");
					uint8_t tmp3[]= "\nTransmission: OK\r\n";
					size_t len3 = strlen(tmp3);
					HAL_UART_Transmit(&huart1, tmp3, len3, 100);
					messageCounter += 1;
					return;
				}

				if (HAL_GetTick() - timeStart > LORA_SEND_TIMEOUT) {
					sx1278Reset();
					//printf("EntryTx failed, timeout reset!\r\n");
					uint8_t tmp4[]= "EntryTx failed, timeout reset!\r\n";
					size_t len4 = strlen(tmp4);
					HAL_UART_Transmit(&huart1, tmp4, len4, 100);
					return;
				}

				HAL_Delay(1);
			}
			loraTx->operatingMode = readMode(loraTx);
		}
}

void setTxParameters(SX1278_t *module) {
	uint8_t cmd = module->packetLength;
	writeRegister(module->spi, LR_RegPayloadLength, &(cmd), 1);
	uint8_t addr = readRegister(module->spi, LR_RegFifoTxBaseAddr);
	writeRegister(module->spi, LR_RegFifoAddrPtr, &addr, 1);
	module->packetLength = readRegister(module->spi, LR_RegPayloadLength);
}

void saveTx(SX1278_t *module) {
	updateLoraLowFreq(module, SLEEP);
	HAL_Delay(15);
	setRFFrequency(module);
	setLORAWAN(module);
	setOutputPower(module);
	setOvercurrentProtect(module);
	writeRegister(module->spi, LR_RegLna, &(module->lnaGain), 1);
	if (module->LoRa_SF == SF_6) {
		module->headerMode = IMPLICIT;
		module->symbTimeoutMsb = 0x03;
		setDetectionParameters(module);
	} else {
		module->headerMode = EXPLICIT;
		module->symbTimeoutMsb = 0x00;
	}
	setReModemConfig(module);
	setPreambleParameters(module);
	writeRegister(module->spi, LR_RegHopPeriod, &(module->fhssValue), 1);
	writeRegister(module->spi, LR_RegDioMapping1, &(module->dioConfig), 1);
	clearIrqFlags(module);
	writeRegister(module->spi, LR_RegIrqFlagsMask, &(module->flagsMode), 1);
}

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
	/* USER CODE BEGIN 1 */
	LED_t led;
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
	MX_CRC_Init();
	/* USER CODE BEGIN 2 */
	ledInit(&led);

	SX1278_hw_t lora_hw;
	SX1278_t loraTx, loraRx;
	lora_ptr = &loraTx;
	lora_ptr = &loraRx;

	//int messageCounter = 0;

	lora_hw.dio0.port = BUSSY_GPIO_Port;
	lora_hw.dio0.pin = BUSSY_Pin;
	lora_hw.nss.port = LORA_NSS_GPIO_Port;
	lora_hw.nss.pin = LORA_NSS_Pin;
	lora_hw.reset.port = LORA_RST_GPIO_Port;
	lora_hw.reset.pin = LORA_RST_Pin;
	loraRx.hw = &lora_hw;
	loraTx.hw = &lora_hw;
	loraRx.spi = &hspi1;
	loraTx.spi = &hspi1;
	HAL_GPIO_WritePin(LORA_NSS_GPIO_Port, LORA_NSS_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LORA_RST_GPIO_Port, LORA_RST_Pin, GPIO_PIN_SET);
	loraRx.operatingMode = readMode(&loraRx);
	loraTx.operatingMode = readMode(&loraTx);
	loraRx.status = UNKNOW;
	loraTx.status = UNKNOW;

	bool RX_MODE = true;
	bool TX_MODE = false;
	//int counter = HAL_GetTick();
	//initialize LoRa module

	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {

		if (RX_MODE){
			read(&huart1, &loraRx);
		}
		if(TX_MODE){
        	//if (HAL_GetTick() - counter > 700){
        		//counter = HAL_GetTick();
	        transmit(&loraTx);
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
