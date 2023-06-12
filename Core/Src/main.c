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
#include "led.h"
#include "rs485.h"
#include "lm75.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#define HS16_CLK 16000000
#define BAUD_RATE 115200
#define KEEP_ALIVE_INTERVAL 1000
#define KEEP_ALIVE_THRESHOLD 50
#define MCP3421_ADDRESS      0x68  // MCP3421 device address
#define MCP3421_GAIN_1    (0x00 << 0)
#define MCP3421_GAIN_2    (0x01 << 0)
#define MCP3421_GAIN_4    (0x02 << 0)
#define MCP3421_GAIN_8    (0x03 << 0)
#define MCP3421_240SPS    (0x00 << 2)
#define MCP3421_60SPS     (0x01 << 2)
#define MCP3421_15SPS     (0x02 << 2)
#define MCP3421_CONT_CONV (0x00 << 4)
#define ADC_CHANNELS_NUM 2
#define ADC_EXTRA_DATA 1
#define IWDG_DEBUG

volatile uint16_t adcValues[ADC_CHANNELS_NUM + ADC_EXTRA_DATA];
//#define IWDG_DEBUG
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

IWDG_HandleTypeDef hiwdg;

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
static void MX_IWDG_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
UART1_t *uart1_ptr;
LED_t led;
RDSS_t *rdss;
UART1_t u1;
Vlad_t *vlad;
SX1278_t *loRa;
Server_t *server;
uint8_t rxData;

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi);
void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi);
void print_parameters(UART1_t *u1, Vlad_t vlad);
void printStatus(UART1_t *u1, RDSS_t *rdss);
void printLoRaStatus(UART1_t *u1, SX1278_t *loRa);
uint8_t executeServerCmd(uint8_t *buffer, RDSS_t *rdss, SX1278_t *loRa,
		Server_t *server);
uint8_t executeCommand(uint8_t *buffer, RDSS_t *rdss, SX1278_t *loRa,
		Server_t *server);
void clearRx(UART1_t *u1);
void transmitRdssQuery(RDSS_t *rdss, SX1278_t *loRa);
void processServerCmd(UART1_t *u1, RDSS_t *rdss, SX1278_t *loRa,
		Server_t *server);
void processUart1Rx(UART1_t *u1, RDSS_t *rdss, Server_t *server, SX1278_t *loRa);
void masterProcessRdss(RDSS_t *rdss);
void masterProcessLoRaRx(SX1278_t *loRa, RDSS_t *rdss, Vlad_t *vlad);
uint32_t enableKeepAliveLed(uint32_t keepAliveStartTicks);
void configureADC();
void calibrateADC();
void configureGPIO();
void startADCConversion(uint8_t channel);
uint16_t readADCChannel(uint8_t channel);
void updateMasterStatus(RDSS_t *rdss, volatile uint16_t *adcValues, uint32_t timeout);

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

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
//  MX_IWDG_Init();
  /* USER CODE BEGIN 2 */
	vlad = vladInit(SERVER);
	server = serverInit(SERVER);
	ledInit(&led);
	rdss = rdssInit(0);
	loRa = loRaInit(&hspi1, MASTER_RECEIVER);
	printStatus(&u1, rdss);
	printLoRaStatus(&u1, loRa);
	lm75_init();
#ifdef IWDG_DEBUG
	HAL_IWDG_Refresh(&hiwdg);
#endif
	//adc_init();
	HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(USART1_IRQn);
	HAL_UART_Receive_IT(&huart1, &rxData, 1);

	configureGPIO();
	configureADC();
	calibrateADC();
//HAL_TIM_Base_Start_IT(&htim2);
	uint32_t keepAliveStartTicks = HAL_GetTick();
	rdss->lastUpdateTicks = HAL_GetTick();
//	HAL_IWDG_Refresh(&hiwdg);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1) {

		processUart1Rx(&u1, rdss, server, loRa);
		configureLoRaRx(loRa, MASTER_RECEIVER);
		masterProcessLoRaRx(loRa, rdss, vlad);
		adcValues[0] = readADCChannel(0);
		adcValues[1] = readADCChannel(1);
		adcValues[2] = lm75_read();
		updateMasterStatus(rdss, adcValues, 1000);

#ifdef IWDG_DEBUG
		HAL_IWDG_Refresh(&hiwdg);
#endif
		keepAliveStartTicks = enableKeepAliveLed(keepAliveStartTicks);
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
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
  hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 4;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = ADC_REGULAR_RANK_2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_VREFINT;
  sConfig.Rank = ADC_REGULAR_RANK_3;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = ADC_REGULAR_RANK_4;
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
  * @brief IWDG Initialization Function
  * @param None
  * @retval None
  */
static void MX_IWDG_Init(void)
{

  /* USER CODE BEGIN IWDG_Init 0 */

  /* USER CODE END IWDG_Init 0 */

  /* USER CODE BEGIN IWDG_Init 1 */

  /* USER CODE END IWDG_Init 1 */
  hiwdg.Instance = IWDG;
  hiwdg.Init.Prescaler = IWDG_PRESCALER_32;
  hiwdg.Init.Reload = 1875;
  if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
  {
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
  huart1.Init.BaudRate = 19200;
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
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LORA_NSS_Pin|LORA_RST_Pin|LORA_DIO3_Pin|LORA_DIO1_Pin
                          |LORA_BUSSY_Pin|LORA_TX_OK_Pin|LORA_RX_OK_Pin|KEEP_ALIVE_Pin
                          |RS485_DE_Pin|BUZZER_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : LORA_NSS_Pin LORA_RST_Pin LORA_DIO3_Pin LORA_DIO1_Pin
                           LORA_BUSSY_Pin LORA_TX_OK_Pin LORA_RX_OK_Pin KEEP_ALIVE_Pin
                           RS485_DE_Pin BUZZER_Pin */
  GPIO_InitStruct.Pin = LORA_NSS_Pin|LORA_RST_Pin|LORA_DIO3_Pin|LORA_DIO1_Pin
                          |LORA_BUSSY_Pin|LORA_TX_OK_Pin|LORA_RX_OK_Pin|KEEP_ALIVE_Pin
                          |RS485_DE_Pin|BUZZER_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	/* Read received data from UART1 */
	if (uart1_ptr->rxSize >= RX_BUFFLEN) {
		cleanRx(uart1_ptr);
		uart1_ptr->rxSize = 0;
	}
	HAL_UART_Receive_IT(&huart1, &rxData, 1);
	uart1_ptr->rxData[uart1_ptr->rxSize++] = rxData;
	if (rxData == LTEL_END_MARK)
		uart1_ptr->isReceivedDataReady = true;
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi) {
	printf("SPI TX Done .. Do Something ...");
}

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi) {
	printf("SPI RX Done .. Do Something ...");
}


uint8_t executeServerCmd(uint8_t *buffer, RDSS_t *rdss, SX1278_t *loRa,
		Server_t *server) {
	uint8_t index = 0;
	HAL_StatusTypeDef res;

	if (rdss->buffSize < LTEL_QUERY_LENGTH || rdss->buffSize > LTEL_SET_LENGTH)
		return 0;

	index = setRdssStartData(rdss, buffer, server->function);

	switch (rdss->cmd) {
	case QUERY_STATUS:
//		index += encodeVladToLtel(buffer + index, vlad);
		index = 0;
		break;
	case QUERY_RX_FREQ:
		buffer[index++] = 4;
		freqEncode(buffer + index, loRa->upFreq);
		index += sizeof(loRa->upFreq);
		index++;
		break;
	case QUERY_TX_FREQ:
		buffer[index++] = 4;
		freqEncode(buffer + index, loRa->dlFreq);
		index += sizeof(loRa->dlFreq);
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
		buffer[index++] = server->function;
		buffer[index++] = rdss->id;
		buffer[index++] = QUERY_MODULE_ID;
		buffer[index++] = 0x00;
		buffer[index++] = 2;
		buffer[index++] = server->function;
		buffer[index++] = rdss->id;
		break;
	case SET_MODULE_ID:
		server->function = rdss->buff[6];
		server->id = rdss->buff[7];
		rdss->id = server->id;
		index = setRdssStartData(rdss, buffer, server->function);
		buffer[index++] = SERVER;
		buffer[index++] = rdss->id;
		HAL_savePage(M24C64_PAGE0, (uint8_t*) &(vlad->function), 3, 1);
		HAL_savePage(M24C64_PAGE0, (uint8_t*) &(vlad->id), 4, 1);
		break;
	case SET_TX_FREQ:
		buffer[index++] = 4;
		loRa->dlFreq = freqDecode(rdss->buff + index);
		index += sizeof(loRa->dlFreq);
		res = HAL_savePage(M24C64_PAGE1, (uint8_t*) &(loRa->dlFreq), 0, 4);
		changeMode(loRa, loRa->mode);
		writeLoRaParametersReg(loRa);
		break;
	case SET_RX_FREQ:
		buffer[index++] = 4;
		loRa->upFreq = freqDecode(rdss->buff + index);
		index += sizeof(loRa->upFreq);
		res = HAL_savePage(M24C64_PAGE1, (uint8_t*) &(loRa->upFreq), 4, 4);
		changeMode(loRa, loRa->mode);
		writeLoRaParametersReg(loRa);
		break;
	case SET_BANDWIDTH:
		buffer[index++] = 1;
		loRa->bandwidth = rdss->buff[index++] - 1;
		res = HAL_savePage(M24C64_PAGE0, &(loRa->bandwidth), 1, 1);
		changeMode(loRa, loRa->mode);
		writeLoRaParametersReg(loRa);
		break;
	case SET_SPREAD_FACTOR:
		buffer[index++] = 1;
		loRa->spreadFactor = rdss->buff[index++] + 6;
		res = HAL_savePage(M24C64_PAGE0, &(loRa->spreadFactor), 0, 1);
		changeMode(loRa, loRa->mode);
		writeLoRaParametersReg(loRa);
		break;
	case SET_CODING_RATE:
		buffer[index++] = 1;
		loRa->codingRate = rdss->buff[index++];
		res = HAL_savePage(M24C64_PAGE0, &(loRa->codingRate), 2, 1);
		changeMode(loRa, loRa->mode);
		writeLoRaParametersReg(loRa);
		break;

	default:
		break;
	}

	index += setCrc(buffer, index);
	buffer[index++] = LTEL_END_MARK;
	rdss->status = UART_SEND;
	return index;
}

void print_parameters(UART1_t *u1, Vlad_t vlad) {
	if (!u1->isDebugModeEnabled)
		return;

	char *buff = (char*) u1->txData;
	u1->txSize = (uint8_t) sprintf(buff, "vin %d [V]\r\n", vlad.vin);
	writeTx(u1);
	u1->txSize = (uint8_t) sprintf(buff, "current real %d [A]\r\n",
			(uint8_t) vlad.currentReal);
	writeTx(u1);
	u1->txSize = (uint8_t) sprintf(buff, "tone level %d[dBm]\r\n",
			vlad.tone_level);
	writeTx(u1);
	u1->txSize = (uint8_t) sprintf(buff, "current %d[A]\r\n", vlad.current);
	writeTx(u1);
	u1->txSize = (uint8_t) sprintf(buff, "agc150m %d[dBm]\r\n", vlad.agc152m);
	writeTx(u1);
	u1->txSize = (uint8_t) sprintf(buff, "level150m %d[dBm]\r\n",
			vlad.level152m);
	writeTx(u1);
	u1->txSize = (uint8_t) sprintf(buff, "agc170m %d[dBm]\r\n", vlad.agc172m);
	writeTx(u1);
	u1->txSize = (uint8_t) sprintf(buff, "level170m %d[dBm]\r\n",
			vlad.level172m);
	writeTx(u1);
	cleanTx(u1);
}

void printStatus(UART1_t *u1, RDSS_t *rdss) {

	if (!u1->isDebugModeEnabled)
		return;

	char *str = (char*) u1->txData;
	uint8_t i = 0;
	switch (rdss->status) {
	case CRC_ERROR:

		u1->txSize = (uint8_t) sprintf(str, "CRC mismatch:\r\n");
		writeTx(u1);
		u1->txSize = 0;
		break;
	case WRONG_MODULE_ID:
		u1->txSize = (uint8_t) sprintf(str,
				"ID mismatch - ID %d and ID received %d \r\n", rdss->id,
				rdss->idReceived);
		writeTx(u1);
		u1->txSize = 0;
		break;
	case NOT_VALID_FRAME:
		u1->txSize = (uint8_t) sprintf(str, "Not valid start byte \r\n");
		writeTx(u1);
		u1->txSize = 0;
		break;
	case DATA_OK:
		u1->txSize = (uint8_t) sprintf(str,
				"Validation ok: ID %02x Cmd %02x Bytes %d Data \r\n",
				rdss->buff[2], rdss->buff[3], rdss->buff[5]);
		writeTx(u1);
		for (int i = DATA_START_INDEX; i < rdss->buff[5]; i++) {
			if (i > 250)
				break;
			u1->txSize = (uint8_t) sprintf(str, "%02X", rdss->buff[i]);
			writeTx(u1);
		}
		writeTxReg('\n');
		break;
	case WAITING:
		u1->txSize = (uint8_t) sprintf(str, "Waiting for new data\r\n");
		writeTx(u1);
		u1->txSize = 0;
		break;
	case LORA_SEND:
		u1->txSize = (uint8_t) sprintf(str, "Send uart data to loRa ID: %d\r\n",
				rdss->idReceived);
		writeTx(u1);
		u1->txSize = 0;
		break;
	case LORA_RECEIVE:
		u1->txSize = (uint8_t) sprintf(str,
				"Validation ok: ID %02x Cmd %02x Bytes %d Data \r\n",
				rdss->buff[2], rdss->buff[3], rdss->buff[5]);
		writeTx(u1);
		for (i = DATA_START_INDEX; i < rdss->buff[5]; i++) {
			if (i > 250)
				break;
			u1->txSize = (uint8_t) sprintf(str, "%02X", rdss->buff[i]);
			writeTx(u1);
		}
		if (i > DATA_START_INDEX)
			writeTxReg('\n');
		u1->txSize = 0;
		break;
	case UART_SEND:
		u1->txSize = (uint8_t) sprintf(str, "Reply vlad data: %d\r\n",
				rdss->idReceived);
		writeTx(u1);
		for (i = 0; i < rdss->buffSize; i++) {
			if (i > 250)
				break;
			u1->txSize = (uint8_t) sprintf(str, "%02X", rdss->buff[i]);
			writeTx(u1);
		}
		if (i > 0)
			writeTxReg('\n');
		u1->txSize = 0;
		break;
	case UART_VALID:
		u1->txSize = (uint8_t) sprintf(str,
				"Validation ok: ID %02x Cmd %02x Bytes %d Data \r\n",
				rdss->buff[2], rdss->buff[3], rdss->buff[5]);
		writeTx(u1);
		for (int i = DATA_START_INDEX; i < rdss->buff[5]; i++) {
			if (i > 250)
				break;
			u1->txSize = (uint8_t) sprintf(str, "%02X", rdss->buff[i]);
			writeTx(u1);
		}
		if (i > DATA_START_INDEX)
			writeTxReg('\n');
		u1->txSize = 0;
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

	char *str = (char*) u1->txData;

	switch (loRa->status) {
	case TX_TIMEOUT:
		u1->txSize = (uint8_t) sprintf(str,
				"Transmission Fail: %d seconds Timeout\r\n", TX_TIMEOUT / 1000);
		writeTx(u1);
		break;
	case TX_DONE:
		u1->txSize = (uint8_t) sprintf(str, "Transmission Done: %lu ms\r\n",
				loRa->lastTxTime);
		writeTx(u1);
		break;
	case TX_BUFFER_READY:
		u1->txSize = (uint8_t) sprintf(str,
				"Transmission Buffer: %d bytes data \r\n", loRa->txSize);
		writeTx(u1);
		for (int i = 0; i < loRa->txSize; i++) {
			u1->txSize = (uint8_t) sprintf(str, "%02X", loRa->txData[i]);
			writeTx(u1);
		}
		writeTxReg('\n');
		break;
	case TX_MODE:
		u1->txSize = (uint8_t) sprintf(str, "%s Mode\r\n",
				(loRa->mode == MASTER_SENDER) ? "Master Sender" :
				(loRa->mode == SLAVE_SENDER) ? "Slave Sender" : "Unknown");
		writeTx(u1);
		break;
	case RX_DONE:
		u1->txSize = (uint8_t) sprintf(str, "Reception Done: %d bytes\r\n",
				loRa->rxSize);
		writeTx(u1);
		for (int i = 0; i < loRa->rxSize; i++) {
			u1->txSize = (uint8_t) sprintf(str, "%02X", loRa->rxData[i]);
			writeTx(u1);
		}
		if (loRa->txSize > 0) {
			writeTxReg('\n');
		}
		break;
	case RX_MODE:
		u1->txSize = (uint8_t) sprintf(str, "%s Mode\r\n",
				(loRa->mode == MASTER_RECEIVER) ? "Master Receiver" :
				(loRa->mode == SLAVE_RECEIVER) ? "Slave Receiver" : "Unknown");
		writeTx(u1);
		break;
	case CRC_ERROR_ACTIVATION:
		u1->txSize = (uint8_t) sprintf(str,
				"Reception Fail: CRC error activation\r\n");
		writeTx(u1);
		break;
	default:
		break;
	}
	cleanTx(u1);
}

void clearRx(UART1_t *u1) {
	memset(u1->rxData, 0, sizeof(u1->rxData));
	u1->rxSize = 0;
}

void transmitRdssQuery(RDSS_t *rdss, SX1278_t *loRa) {
	rdss->idQuery = rdss->idReceived;
	loRa->txData = rdss->buff;
	loRa->txSize = rdss->buffSize;
	changeMode(loRa, MASTER_SENDER);
	transmit(loRa); // TODO: se puede mejorar usando enviando la trama a transmitir
	if(loRa->status == TX_DONE)
		HAL_GPIO_WritePin(LORA_TX_OK_GPIO_Port,LORA_TX_OK_Pin,GPIO_PIN_SET);
	changeMode(loRa, MASTER_RECEIVER);
	HAL_GPIO_WritePin(LORA_TX_OK_GPIO_Port,LORA_TX_OK_Pin,GPIO_PIN_RESET);
}

void processServerCmd(UART1_t *u1, RDSS_t *rdss, SX1278_t *loRa,
		Server_t *server) {
	if (rdss->cmd == QUERY_MASTER_STATUS) {
		for (uint8_t i = 0; i < 15; i++)
			writeTxReg(rdss->queryBuffer[i]);
	} else {
		u1->txData = malloc(sizeof(uint8_t) * 25);
		u1->txSize = executeServerCmd(u1->txData, rdss, loRa, server);

		for (uint8_t i = 0; i < u1->txSize; i++)
			writeTxReg(u1->txData[i]);
		u1->txSize = 0;
		free(u1->txData);
	}
}

void processUart1Rx(UART1_t *u1, RDSS_t *rdss, Server_t *server, SX1278_t *loRa) {
	if (u1->isReceivedDataReady == false)
		return;
	u1->isReceivedDataReady = false;
	if (validate(u1->rxData, u1->rxSize) != DATA_OK) {
		// Clear UART buffer and length
		memset(u1->rxData, 0, sizeof(u1->rxData));
		u1->rxSize = 0;
		return;
	}
	updateRdss(rdss, u1->rxData, u1->rxSize);
	if (rdss->idReceived == rdss->id) {

		processServerCmd(u1, rdss, loRa, server);
	} else if (rdss->cmd != 0) {
		transmitRdssQuery(rdss, loRa);
	}

	clearRx(u1);
	rdssReinit(rdss);
}

void masterProcessRdss(RDSS_t *rdss) {
	if (rdss->cmd == QUERY_STATUS)
		for (uint8_t i = 0; i < rdss->buffSize; i++)
			writeTxReg(rdss->buff[i]);
}

void masterProcessLoRaRx(SX1278_t *loRa, RDSS_t *rdss, Vlad_t *vlad) {

	/*
	 if (loRa->operatingMode != RX_CONTINUOUS) {
	 changeMode(loRa, MASTER_RECEIVER); // Set the LoRa operating mode to master receiver
	 setRxFifoAddr(loRa); // Set the FIFO address for receive mode
	 setLoRaLowFreqModeReg(loRa, RX_CONTINUOUS); // Configure low-frequency mode for continuous receive
	 loRa->rxSize = 0;
	 }
	 */

	if (HAL_GPIO_ReadPin(LORA_BUSSY_GPIO_Port,
	LORA_BUSSY_Pin) == GPIO_PIN_RESET)
		return; // if (crcErrorActivation(loRa) != 1)

	loRa->rxData = getRxFifoData(loRa);
	clearIrqFlagsReg(loRa); // Retrieve data from the receive FIFO
	if (loRa->rxData < 0) {
		free(loRa->rxData);
		return;
	}
	if (validate(loRa->rxData, loRa->rxSize) != DATA_OK) {
		free(loRa->rxData);
		return;
	}
	HAL_GPIO_WritePin(LORA_RX_OK_GPIO_Port, LORA_RX_OK_Pin, GPIO_PIN_SET);
	updateRdss(rdss, loRa->rxData, loRa->rxSize);

	if (rdss->idReceived != rdss->idQuery) {
		rdss->status = WRONG_MODULE_ID;
		free(loRa->rxData);
		return;
	}
	masterProcessRdss(rdss);
	rdssReinit(rdss);
	free(loRa->rxData);
	loRa->rxSize = 0;
	HAL_GPIO_WritePin(LORA_RX_OK_GPIO_Port, LORA_RX_OK_Pin, GPIO_PIN_RESET);
}

uint32_t enableKeepAliveLed(uint32_t keepAliveStartTicks) {
	if (HAL_GetTick() - keepAliveStartTicks > 1000) {
		keepAliveStartTicks = HAL_GetTick();
		HAL_GPIO_WritePin(KEEP_ALIVE_GPIO_Port, KEEP_ALIVE_Pin, GPIO_PIN_SET);
	} else if (HAL_GetTick() - keepAliveStartTicks > 50)
		HAL_GPIO_WritePin(KEEP_ALIVE_GPIO_Port, KEEP_ALIVE_Pin, GPIO_PIN_RESET);

	return keepAliveStartTicks;
}

void configureADC() {
	// Enable ADC clock
	RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;

	// Enable GPIOA clock
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;

	// Enable SWSTART
	ADC1->CR2 |= ADC_CR2_EXTTRIG | ADC_CR2_EXTSEL; // Set external trigger and trigger source

	// Enable ADC temperature sensor and Vrefint channels
	ADC1->CR2 |= ADC_CR2_TSVREFE;
	// Enable ADC1
	ADC1->CR2 |= ADC_CR2_ADON;
	// Enable ADC interrupt
//	NVIC_EnableIRQ(ADC1_IRQn);
}

void calibrateADC() {
	// Start ADC calibration
	ADC1->CR2 |= ADC_CR2_CAL;

	// Wait for calibration to complete
	while (ADC1->CR2 & ADC_CR2_CAL) {
	}
}

void configureGPIO() {
	// Enable GPIOA clock
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;

	// Configure PA0 and PA11 as analog input mode
	GPIOA->CRL &= ~(GPIO_CRL_CNF0 | GPIO_CRL_MODE0);
	GPIOA->CRH &= ~(GPIO_CRH_CNF11 | GPIO_CRH_MODE11);
}

void startADCConversion(uint8_t channel) {
	// Clear previous channel selection
	ADC1->SQR3 &= ~ADC_SQR3_SQ1_Msk;

	// Set new channel selection
	ADC1->SQR3 |= (channel << ADC_SQR3_SQ1_Pos);

	// Enable ADC1
	ADC1->CR2 |= ADC_CR2_ADON;

	// Start ADC conversion
	ADC1->CR2 |= ADC_CR2_SWSTART;
}

uint16_t readADCChannel(uint8_t channel) {
	// Start ADC conversion for the specified channel
	startADCConversion(channel);

	// Wait for conversion to complete
	while ((ADC1->SR & ADC_SR_EOC) == 0) {
	}

	// Read the ADC value
	uint16_t adcValue = ADC1->DR;

	return adcValue;
}

void updateMasterStatus(RDSS_t *rdss, volatile uint16_t *adcValues,
		uint32_t timeout) {
	if (HAL_GetTick() - rdss->lastUpdateTicks > timeout) {
		uint8_t index = 0;
		const uint8_t querySize = 5;
		memset(rdss->queryBuffer, 0, 14);
		rdss->queryBuffer[index++] = LTEL_START_MARK;
		rdss->queryBuffer[index++] = SERVER;
		rdss->queryBuffer[index++] = rdss->id;
		rdss->queryBuffer[index++] = QUERY_MASTER_STATUS;
		rdss->queryBuffer[index++] = 0x00;
		rdss->queryBuffer[index++] = querySize;
		rdss->queryBuffer[index++] = adcValues[0];
		rdss->queryBuffer[index++] = adcValues[0] >> 8;
		rdss->queryBuffer[index++] = adcValues[1];
		rdss->queryBuffer[index++] = adcValues[1] >> 8;
		rdss->queryBuffer[index++] = adcValues[2];
		index += setCrc(rdss->queryBuffer, index);
		rdss->queryBuffer[index++] = LTEL_END_MARK;
		rdss->lastUpdateTicks = HAL_GetTick();
	}
}

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
