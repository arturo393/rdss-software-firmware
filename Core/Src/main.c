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
Tone_uhf_t *uhf_ptr;

void USART1_IRQHandler(void) {
	uart1_read_to_frame(uart1_ptr);
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

	loraTx->frequency = DOWNLINK_FREQ;
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

void setTxParameters(SX1278_t *module) {
	uint8_t cmd = module->packetLength;
	writeRegister(module->spi, LR_RegPayloadLength, &(cmd), 1);
	uint8_t addr = readRegister(module->spi, LR_RegFifoTxBaseAddr);
	addr = 0x80;
	writeRegister(module->spi, LR_RegFifoAddrPtr, &addr, 1);
	module->packetLength = readRegister(module->spi, LR_RegPayloadLength);
}

void sx1278Reset() {
	HAL_GPIO_WritePin(LORA_NSS_GPIO_Port, LORA_NSS_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LORA_NSS_GPIO_Port, LORA_NSS_Pin, GPIO_PIN_RESET);
	HAL_Delay(1);
	HAL_GPIO_WritePin(LORA_NSS_GPIO_Port, LORA_NSS_Pin, GPIO_PIN_SET);
	HAL_Delay(100);
}

int messageCounter = 0;
void transmit(SX1278_t *loraTx) {
	if (loraTx->status == UNKNOW) {
		uint8_t tmp[] = "Configuring Master LoRa module: Tx Mode\r\n";
		size_t len = strlen(tmp);
		HAL_UART_Transmit(&huart1, tmp, len, 100);
		setTxBaseParameters(loraTx);
		saveTx(loraTx);
		loraTx->status = TX_READY;
	}
	if (loraTx->status == TX_READY) {

		memset(loraTx->buffer, 0, SX1278_MAX_PACKET);
		loraTx->packetLength = sprintf((char*) loraTx->buffer, "Hello World"
				" %d", messageCounter);

		setTxParameters(loraTx);
		uint8_t tmp2[] = "Sending message: ";
		size_t len2 = strlen(tmp2);
		HAL_UART_Transmit(&huart1, tmp2, len2, 100);
		HAL_UART_Transmit(&huart1, loraTx->buffer, loraTx->packetLength, 100);

		for (int i = 0; i < loraTx->packetLength; i++) {
			char data = loraTx->buffer[i];
			writeRegister(loraTx->spi, 0x00, &data, 1);
		}
		updateLoraLowFreq(loraTx, TX);
		int timeStart = HAL_GetTick();
		while (1) {

			if (SX1278_hw_GetDIO0(loraTx->hw)) {
				int timeEnd = HAL_GetTick();
				int tiempoTransmision = timeEnd - timeStart;
				readRegister(loraTx->spi, LR_RegIrqFlags);
				clearIrqFlags(loraTx);
				uint8_t tmp3[100] = { 0 };
				uint8_t len3 = sprintf((char*) tmp3,
						" - Tx Ok: %d ms %d bytes\n", tiempoTransmision,
						loraTx->packetLength);

				HAL_UART_Transmit(&huart1, tmp3, len3, 100);
				messageCounter += 1;
				return;
			}

			if (HAL_GetTick() - timeStart > LORA_SEND_TIMEOUT) {
				sx1278Reset();
				uint8_t tmp4[] = "EntryTx failed, timeout reset!\r\n";
				size_t len4 = strlen(tmp4);
				HAL_UART_Transmit(&huart1, tmp4, len4, 100);

				return;
			}

			HAL_Delay(1);
		}
		loraTx->operatingMode = readMode(loraTx);
	}
}

void setRxBaseParameters(SX1278_t *loraRx) {

	uint8_t dio0 = DIO0_RX_DONE;
	uint8_t dio1 = DIO1_RX_TIMEOUT;
	uint8_t dio2 = DIO2_FHSS_CHANGE_CHANNEL;
	uint8_t dio3 = DIO3_VALID_HEADER;
	////////////////////////////////////////
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

	loraRx->flagsMode = 0xff;
	CLEAR_BIT(loraRx->flagsMode, RX_DONE_MASK);
	CLEAR_BIT(loraRx->flagsMode, PAYLOAD_CRC_ERROR_MASK);

	loraRx->fhssValue = HOPS_PERIOD;
	loraRx->packetLength = SX1278_MAX_PACKET;
}

void saveRx(SX1278_t *module) {
	updateLoraLowFreq(module, SLEEP); //Change modem mode Must in Sleep mode
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

void clearMemForRx(SX1278_t *loraRx) {
	if (loraRx->status == RX_READY) {
		memset(loraRx->buffer, 0, SX1278_MAX_PACKET);
	}
}

void waitForRxDone(SX1278_t *loraRx) {
	while (!SX1278_hw_GetDIO0(loraRx->hw)) {
		uint8_t flags = readRegister(loraRx->spi, LR_RegIrqFlags);
		if (READ_BIT(flags, PAYLOAD_CRC_ERROR_MASK)) {
			uint8_t cmd = flags | (1 << 7);
			writeRegister(loraRx->spi, LR_RegIrqFlags, &cmd, 1);
			flags = readRegister(loraRx->spi, LR_RegIrqFlags);
		}
	}
}

void configInit(UART_HandleTypeDef *huart1, SX1278_t *loraRx) {
	uint8_t tmp[] = "Configuring Master LoRa module: Rx Mode\r\n";
	size_t len = strlen(tmp);
	HAL_UART_Transmit(&*huart1, tmp, len, 100);
	setRxBaseParameters(&*loraRx);
	saveRx(loraRx);
	loraRx->status = RX_READY;
	setRxParameters(loraRx);
}

int crcErrorActivation(SX1278_t *loraRx) {
	uint8_t flags2 = readRegister(loraRx->spi, LR_RegIrqFlags);
	SET_BIT(flags2, RX_DONE_MASK);
	uint8_t cmd = flags2;
	writeRegister(loraRx->spi, LR_RegIrqFlags, &cmd, 1);
	uint8_t flags = readRegister(loraRx->spi, LR_RegIrqFlags);
	int errorActivation = READ_BIT(flags, PAYLOAD_CRC_ERROR_MASK);
	return errorActivation;
}

void getLoraPacket(SX1278_t *loraRx) {
	loraRx->packetLength = readRegister(loraRx->spi, LR_RegRxNbBytes); //Number for received bytes
	uint8_t addr = 0x00;
	HAL_GPIO_WritePin(GPIOB, LORA_NSS_Pin, GPIO_PIN_RESET); // pull the pin low
	HAL_Delay(1);
	HAL_SPI_Transmit(loraRx->spi, &addr, 1, 100); // send address
	HAL_SPI_Receive(loraRx->spi, loraRx->buffer, loraRx->packetLength, 100); // receive 6 bytes data
	HAL_Delay(1);
	HAL_GPIO_WritePin(GPIOB, LORA_NSS_Pin, GPIO_PIN_SET); // pull the pin high
}

void printParameters(int timeRx, UART_HandleTypeDef *huart1, SX1278_t *loraRx) {
	HAL_UART_Transmit(huart1, loraRx->buffer, loraRx->packetLength, 100);
	uint8_t largo = sprintf((char*) loraRx->buffer,
			" - Rx Ok: %d ms %d bytes\n", timeRx, loraRx->packetLength);
	HAL_UART_Transmit(huart1, loraRx->buffer, largo, 100);
}

void read(UART_HandleTypeDef *huart1, SX1278_t *loraRx) {
	if (loraRx->status == UNKNOW) {
		configInit(huart1, loraRx);
		updateLoraLowFreq(&*loraRx, RX_CONTINUOUS);
	}
	clearMemForRx(loraRx);
	int timeStart = HAL_GetTick();
	waitForRxDone(loraRx);
	int timeEnd = HAL_GetTick();
	int timeRx = timeEnd - timeStart;
	int errorActivation = crcErrorActivation(loraRx);
	if (errorActivation == 1) {
		return;
	}
	getLoraPacket(loraRx);
	printParameters(timeRx, huart1, loraRx);
	setRxParameters(loraRx);
	updateLoraLowFreq(&*loraRx, RX_CONTINUOUS);
	loraRx->operatingMode = readMode(loraRx);
}

bool TX_MODE;
bool RX_MODE;

void modeCmdUpdate(const UART1_t *uart1) {
	unsigned long receiveValue;
	receiveValue = 0;
	receiveValue = uart1->rxBuffer[4] << 24;
	receiveValue |= uart1->rxBuffer[5] << 16;
	receiveValue |= uart1->rxBuffer[6] << 8;
	receiveValue |= uart1->rxBuffer[7];
	if (receiveValue == 0) {
		 RX_MODE = true;
		 TX_MODE = false;
	} if (receiveValue == 1) {
		 TX_MODE = true;
		 RX_MODE = false;
	}
}

void modeRs485Update(const UART1_t *uart1, RS485_t *rs485) {
	switch (rs485->cmd) {
	case SET_PARAMETER_FREQOUT: //cmd = 31
		modeCmdUpdate(uart1);
		rs485->cmd = NONE;
		break;
	case SET_PARAMETERS: //cmd = 32
		//ParametersCmd(uart1);
		rs485->cmd = NONE;
		break;
	case SET_PARAMETER_FREQBASE: //cmd = 33
		//freqBaseCmdUpdate(uart1);
		rs485->cmd = NONE;
		break;
	case QUERY_PARAMETER_PdBm: //cmd = 34
		//powerOutCmdUpdate(uart1);
		rs485->cmd = NONE;
		break;
	case SET_MODE: //cmd = 35
		//setModeCmd(uart1,;
		rs485->cmd = NONE;
		break;
	default:
		rs485->cmd = NONE;
		break;
	}
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
	UART1_t uart1;
	Tone_uhf_t uhf;
 	uhf_ptr = &uhf;
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
	toneUhfInit(UHF_TONE, ID0, &uhf);
	ledInit(&led);
	rs485Init(&rs485);
	//uart1Init(HS16_CLK, BAUD_RATE, &uart1);

	SX1278_hw_t lora_hw;
	SX1278_t loraTx, loraRx;
	lora_ptr = &loraTx;
	lora_ptr = &loraRx;

	lora_hw.dio0.port = LORA_BUSSY_GPIO_Port;
	lora_hw.dio0.pin = LORA_BUSSY_Pin;
	lora_hw.nss.port = LORA_NSS_GPIO_Port;
	lora_hw.nss.pin = LORA_NSS_Pin;
	lora_hw.reset.port = LORA_RST_GPIO_Port;
	lora_hw.reset.pin = LORA_RST_Pin;
	loraTx.hw = &lora_hw;
	loraRx.hw = &lora_hw;
	loraTx.spi = &hspi1;
	loraRx.spi = &hspi1;
	HAL_GPIO_WritePin(LORA_NSS_GPIO_Port, LORA_NSS_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LORA_RST_GPIO_Port, LORA_RST_Pin, GPIO_PIN_SET);
	loraTx.operatingMode = readRegister(&hspi1, LR_RegOpMode);
	loraRx.operatingMode = readRegister(&hspi1, LR_RegOpMode);
	loraTx.status = UNKNOW;
	loraRx.status = UNKNOW;
	TX_MODE = true;
	RX_MODE = false;
	int counter = HAL_GetTick();
	int change = 0;
	int master;
	int valueTx = 0;
	int valueRx = 0;
	//initialize LoRa module

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1) {

		 master = HAL_GPIO_ReadPin(MODE_GPIO_Port, MODE_Pin);

		 if (master == 1) {
			 valueRx = 0;
				TX_MODE = true;
				RX_MODE = false;
				valueTx +=1;
				if(valueTx == 1){
				loraTx.status = UNKNOW;
				}
		 } if(master == 0) {
			 valueTx = 0;
			    TX_MODE = false;
				RX_MODE = true;
				valueRx +=1;
				if(valueRx ==1){
				loraRx.status = UNKNOW;
				}
		 }

		rs485Uart1Decode(&rs485, &uart1);
		modeRs485Update(&uart1, &rs485);
		if (TX_MODE) {
			RX_MODE_OFF_LED();
			TX_MODE_ON_LED();
			if (HAL_GetTick() - counter > 1) {
				counter = HAL_GetTick();
				transmit(&loraTx);
				change +=1;
				/*if(change == 13){
					TX_MODE = false;
					RX_MODE = true;
					change = 0;
					loraTx.status = UNKNOW;
				}*/
			}
		}
		if (RX_MODE) {
			TX_MODE_OFF_LED();
			RX_MODE_ON_LED();
			read(&huart1, &loraRx);
			change +=1;
			/*if(change == 13){
				TX_MODE = true;
				RX_MODE = false;
				change = 0;
				loraRx.status = UNKNOW;
			}*/

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
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

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
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
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

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
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
  if (HAL_CRC_Init(&hcrc) != HAL_OK)
  {
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
static void MX_I2C1_Init(void)
{

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
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
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
  hspi1.Init.CRCPolynomial = 10;
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
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
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
static void MX_USART2_UART_Init(void)
{

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
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
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
static void MX_USART3_UART_Init(void)
{

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
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
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
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LORA_NSS_Pin|LORA_RST_Pin|LORA_DIO3_Pin|LORA_DIO1_Pin
                          |LORA_BUSSY_Pin|LED2_Pin|LED1_Pin|RS485_DE_Pin
                          |BUZZER_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : LORA_NSS_Pin LORA_RST_Pin LORA_DIO3_Pin LORA_DIO1_Pin
                           LORA_BUSSY_Pin LED2_Pin LED1_Pin RS485_DE_Pin
                           BUZZER_Pin */
  GPIO_InitStruct.Pin = LORA_NSS_Pin|LORA_RST_Pin|LORA_DIO3_Pin|LORA_DIO1_Pin
                          |LORA_BUSSY_Pin|LED2_Pin|LED1_Pin|RS485_DE_Pin
                          |BUZZER_Pin;
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
