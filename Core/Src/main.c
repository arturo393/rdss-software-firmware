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
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

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
static void MX_DMA_Init(void);
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
uint8_t decodeVlad(Vlad_t *vlad, uint8_t *frame);
void print_parameters(UART1_t *u1, Vlad_t vlad);
void printStatus(UART1_t *u1, RDSS_t *rdss);
void printLoRaStatus(UART1_t *u1, SX1278_t *loRa);
void parseLoRaMaster(RDSS_t *rs485, SX1278_t *loRa);
void parseUartMaster(UART1_t *uart1, RDSS_t *rdss);
void sendQueryToLoRa(RDSS_t *rdss, SX1278_t *loRaModule,
		UART1_t *uart1Interface);
void processReceivedUartCommand(Vlad_t *vlad, UART1_t *u1, RDSS_t *rdss,
		SX1278_t *loRa);
GPIO_PinState processLoRaSlaveReceiver(SX1278_t *loRa, UART1_t *u1);
void processLoraReceiveState(UART1_t *uartHandle, RDSS_t *rdssHandle,
		Vlad_t *vladHandle, SX1278_t *loRaHandle);
void processLoraSendState(UART1_t *uartHandle, RDSS_t *rdssHandle,
		SX1278_t *loRaHandle);
void processMasterReceiverMode(UART1_t *uartHandle, SX1278_t *loRaHandle);
uint8_t setBufferWithLtelCmd(uint8_t *buffer, RDSS_t *rdss, SX1278_t *loRa,
		Vlad_t *vlad);
uint8_t setBufferWithLtelServerCmd(uint8_t *buffer, RDSS_t *rdss,
		SX1278_t *loRa, Server_t *server);
void processReceivedUartCommand(Vlad_t *vlad, UART1_t *u1, RDSS_t *rdss,
		SX1278_t *loRa);
void processServerReceivedUartCommand(Server_t *server, UART1_t *u1,
		RDSS_t *rdss, SX1278_t *loRa);
uint8_t handleSetModuleID(uint8_t *buffer, RDSS_t *rdss, Vlad_t *vlad);
uint8_t handleSetServerModuleID(uint8_t *buffer, RDSS_t *rdss, Server_t *server);
uint16_t decodeUint16(uint8_t *frame, uint8_t *index);
void timer2Init(void);
void TIM2_IRQHandler(void);
HAL_StatusTypeDef MCP3421_Init(I2C_TypeDef *i2c);
float MCP3421_ProcessData(uint8_t *data);
void adc_init(void);
uint16_t adc_read(uint8_t channel, uint32_t timeout_ms);
void adc_moving_average(void);
#define SAMPLE_COUNT 20

volatile uint32_t adc_buffer[3];
volatile uint32_t moving_avg_buffer[3][SAMPLE_COUNT];
volatile uint8_t sample_index = 0;
volatile uint32_t moving_avg[3];
bool adcReadyToRead = false;

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {
	for (int i = 0; i < 3; i++) {
		// Subtract the oldest sample
		uint32_t sum = 0;
		for (int j = 0; j < SAMPLE_COUNT; j++) {
			sum -= moving_avg_buffer[i][j];
		}

		// Replace the oldest sample with the new one
		moving_avg_buffer[i][sample_index] = adc_buffer[i];

		// Add the newest sample
		sum += adc_buffer[i];

		// Calculate the average
		sum /= SAMPLE_COUNT;

		// Store the moving average
		moving_avg[i] = sum;
		// Use 'sum' here or store it in a global variable
	}

	// Increment the index to replace the oldest sample next time
	sample_index = (sample_index + 1) % SAMPLE_COUNT;
}

/*void USART1_IRQHandler(void) {
 uart1_read_to_frame(uart1_ptr);
 }*/

/* In the interrupt handler, read the received data from the UART1 data register */
/* Enable UART1 interrupt */

void i2cScanner(I2C_HandleTypeDef *hi2c1) {
	HAL_StatusTypeDef res;
	uint8_t addrs[4];
	uint8_t addrCount = 0;
	memset(addrs, 0, 4);
	for (uint16_t i = 0; i < 128; i++) {
		res = HAL_I2C_IsDeviceReady(&*hi2c1, i << 1, 1, 10);
		if (res == HAL_OK) {
			addrs[addrCount++] = i;
		}
	}
}

uint32_t enableKeepAliveLed(uint32_t keepAliveStartTicks) {
	if (HAL_GetTick() - keepAliveStartTicks > 1000) {
		keepAliveStartTicks = HAL_GetTick();
		HAL_GPIO_WritePin(KEEP_ALIVE_GPIO_Port, KEEP_ALIVE_Pin, GPIO_PIN_SET);
	} else if (HAL_GetTick() - keepAliveStartTicks > 50)
		HAL_GPIO_WritePin(KEEP_ALIVE_GPIO_Port, KEEP_ALIVE_Pin, GPIO_PIN_RESET);

	return keepAliveStartTicks;
}

void timer2Init(void) {
	// Enable Timer2 clock
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

	// Configure Timer2
	TIM2->CR1 = 0;      // Clear control register 1
	TIM2->CR2 = 0;      // Clear control register 2
	TIM2->SMCR = 0;     // Clear slave mode control register
	TIM2->DIER = 0;     // Clear DMA/Interrupt enable register
	TIM2->CCMR1 = 0;    // Clear capture/compare mode register 1
	TIM2->CCMR2 = 0;    // Clear capture/compare mode register 2
	TIM2->CCER = 0;     // Clear capture/compare enable register
	TIM2->PSC = (SystemCoreClock / 1000) - 1; // Set prescaler to achieve 1ms resolution
	TIM2->ARR = 3000 - 1;   // Set auto-reload value for 3 seconds
	TIM2->CNT = 0;      // Clear counter value

	// Enable Timer2 interrupt
	TIM2->DIER |= TIM_DIER_UIE;

	// Enable Timer2
	TIM2->CR1 |= TIM_CR1_CEN;

	// Enable Timer2 interrupt in NVIC
	NVIC_SetPriority(TIM2_IRQn, 0); // Set interrupt priority
	NVIC_EnableIRQ(TIM2_IRQn);
}

void TIM2_IRQHandler(void) {
	if (TIM2->SR & TIM_SR_UIF) {
		TIM2->SR &= ~TIM_SR_UIF;    // Clear update interrupt flag
		//       HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_buffer, 3);
		// Your interrupt handling code here
		// This code will be executed every 3 seconds
		adcReadyToRead = true;

	}
}

HAL_StatusTypeDef MCP3421_Init(I2C_TypeDef *i2c) {
	uint8_t config_byte = MCP3421_GAIN_1 | MCP3421_240SPS | MCP3421_CONT_CONV;
	return HAL_I2C_Master_Receive(&hi2c1, MCP3421_ADDRESS, &config_byte, 1, 100);
}

float MCP3421_ProcessData(uint8_t *data) {
	int32_t adc_value = ((data[0] << 16) | (data[1] << 8) | data[2]) >> 2; // shift right 2 bits because data is 18-bit
	if (adc_value & 0x00020000) {
		adc_value |= 0xFFFC0000;
	}
	float voltage = (float) adc_value * 3.3f / (1 << 18); // convert to voltage assuming V_REF is the reference voltage
	return voltage;
}

void adc_init(void) {
	// Enable clock for ADC1
	RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;

	// Set GPIOA Pin 0, 1, and 4 as analog input
	GPIOA->CRL &= ~(GPIO_CRL_MODE0_Msk | GPIO_CRL_CNF0_Msk |
	GPIO_CRL_MODE1_Msk | GPIO_CRL_CNF1_Msk |
	GPIO_CRL_MODE4_Msk | GPIO_CRL_CNF4_Msk);

	// Set sample time for channels 0, 1, and 4 to 239.5 ADC clock cycles
	ADC1->SMPR2 |= (ADC_SMPR2_SMP0_2 | ADC_SMPR2_SMP0_1 | ADC_SMPR2_SMP0_0 |
	ADC_SMPR2_SMP1_2 | ADC_SMPR2_SMP1_1 | ADC_SMPR2_SMP1_0 |
	ADC_SMPR2_SMP4_2 | ADC_SMPR2_SMP4_1 | ADC_SMPR2_SMP4_0);

	// Set regular sequence length to 2
	ADC1->SQR1 &= ~ADC_SQR1_L_Msk;
	ADC1->SQR1 |= ADC_SQR1_L_0;

	// Configure regular sequence for channels 0, 1, and 4
	ADC1->SQR3 &= ~(ADC_SQR3_SQ1_Msk | ADC_SQR3_SQ2_Msk | ADC_SQR3_SQ3_Msk);
	ADC1->SQR3 |= (ADC_SQR3_SQ1_0 | ADC_SQR3_SQ2_1 | ADC_SQR3_SQ4_0);

	// Enable ADC1
	ADC1->CR2 |= ADC_CR2_ADON;
}

uint16_t adc_read(uint8_t channel, uint32_t timeout_ms) {
	// Set the desired channel in the ADC sequence
	ADC1->SQR3 = (ADC1->SQR3 & ~(0x1F)) | (channel & 0x1F);

	// Start the conversion
	ADC1->CR2 |= ADC_CR2_SWSTART;

	// Wait for the conversion to complete
	uint32_t start_time = HAL_GetTick();
	while (!(ADC1->SR & ADC_SR_EOC)) {
		if ((HAL_GetTick() - start_time) >= timeout_ms) {
			// Timeout occurred, return an invalid value
			return 0xFFFF;
		}
	}
	// Read the converted value
	uint16_t result = ADC1->DR;

	return result;
}

void adc_moving_average() {

	adc_buffer[0] = adc_read(0, 10);
	adc_buffer[1] = adc_read(1, 10);
	adc_buffer[2] = adc_read(4, 10);
	for (int i = 0; i < 3; i++) {
		// Subtract the oldest sample
		uint32_t sum = 0;
		for (int j = 0; j < SAMPLE_COUNT; j++) {
			sum -= moving_avg_buffer[i][j];
		}

		// Replace the oldest sample with the new one
		moving_avg_buffer[i][sample_index] = adc_buffer[i];

		// Add the newest sample
		sum += adc_buffer[i];

		// Calculate the average
		sum /= SAMPLE_COUNT;

		// Store the moving average
		moving_avg[i] = sum;
		// Use 'sum' here or store it in a global variable
	}

	// Increment the index to replace the oldest sample next time
	sample_index = (sample_index + 1) % SAMPLE_COUNT;
}
/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
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
//	MX_DMA_Init();
	MX_I2C1_Init();
	MX_SPI1_Init();
	MX_USART1_UART_Init();
	MX_USART2_UART_Init();
	MX_USART3_UART_Init();
	MX_ADC1_Init();
	MX_CRC_Init();
	/* USER CODE BEGIN 2 */
//	vlad = vladInit(SERVER);
	server = serverInit(SERVER);
	ledInit(&led);
	rdss = rdssInit(0);
	loRa = loRaInit(&hspi1, MASTER_RECEIVER);
	printStatus(&u1, rdss);
	printLoRaStatus(&u1, loRa);
	lm75_init();

	timer2Init();
	HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(USART1_IRQn);
	HAL_UART_Receive_IT(&huart1, &rxData, 1);

	//HAL_TIM_Base_Start_IT(&htim2);
//	HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_buffer, 3);
	uint32_t keepAliveStartTicks = HAL_GetTick();
	bool isMCP341Configured = MCP3421_Init(I2C1);

	if (!isMCP341Configured) {
		//handle error
	}

//	adc_init();

	// Read ADC values for channels 0, 1, and 4

	/*
	 * ADC 0 - Vin
	 * ADC 1 - current
	 * ADC 4 - temperature
	 */
	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		if (rdss->status == LORA_RECEIVE) {
			processLoraReceiveState(&u1, rdss, vlad, loRa);
		} else if (rdss->status == LORA_SEND) {
			processLoraSendState(&u1, rdss, loRa);
		} else if (rdss->status == UART_VALID) {
			processServerReceivedUartCommand(server, &u1, rdss, loRa);
		} else if (u1.isReceivedDataReady) {
			parseUartMaster(&u1, rdss);
		} else if (loRa->len > 0) {
			parseLoRaMaster(rdss, loRa);
		} else if (loRa->mode == MASTER_RECEIVER) {
			processMasterReceiverMode(&u1, loRa);
		} else if (loRa->mode == MASTER_SENDER) {
			RX_MODE_OFF_LED();
			TX_MODE_ON_LED();
		}
/*		if (adcReadyToRead) {
			adc_moving_average();
			adcReadyToRead = false;
		}
		server->lm75Temperature = lm75_read();
		server->inputVoltageReal = moving_avg[0] * ADC_VOLTAGE_FACTOR;
		server->analoglineCurrentReal = moving_avg[1] * ADC_LINE_CURRENT_FACTOR;
		server->analogTemperature = moving_avg[2];
		uint8_t buffer[3];

		if (HAL_I2C_Master_Receive(&hi2c1, MCP3421_ADDRESS, buffer, 3, 500)) {
			server->mcp3421lineCurrentReal = MCP3421_ProcessData(buffer);
		}
*/
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
	hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
	hadc1.Init.ContinuousConvMode = ENABLE;
	hadc1.Init.DiscontinuousConvMode = DISABLE;
	hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
	hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	hadc1.Init.NbrOfConversion = 3;
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

	/** Configure Regular Channel
	 */
	sConfig.Channel = ADC_CHANNEL_1;
	sConfig.Rank = ADC_REGULAR_RANK_2;
	if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
		Error_Handler();
	}

	/** Configure Regular Channel
	 */
	sConfig.Channel = ADC_CHANNEL_4;
	sConfig.Rank = ADC_REGULAR_RANK_3;
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
	huart1.Init.BaudRate = 19200;
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
 * Enable DMA controller clock
 */
static void MX_DMA_Init(void) {

	/* DMA controller clock enable */
	__HAL_RCC_DMA1_CLK_ENABLE();

	/* DMA interrupt init */
	/* DMA1_Channel1_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

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
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOB,
			LORA_NSS_Pin | LORA_RST_Pin | LORA_DIO3_Pin | LORA_DIO1_Pin
					| LORA_BUSSY_Pin | LED2_Pin | KEEP_ALIVE_Pin | RS485_DE_Pin
					| BUZZER_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pins : LORA_NSS_Pin LORA_RST_Pin LORA_DIO3_Pin LORA_DIO1_Pin
	 LORA_BUSSY_Pin LED2_Pin KEEP_ALIVE_Pin RS485_DE_Pin
	 BUZZER_Pin */
	GPIO_InitStruct.Pin = LORA_NSS_Pin | LORA_RST_Pin | LORA_DIO3_Pin
			| LORA_DIO1_Pin | LORA_BUSSY_Pin | LED2_Pin | KEEP_ALIVE_Pin
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

	/* USER CODE BEGIN MX_GPIO_Init_2 */
	/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	/* Read received data from UART1 */
	if (uart1_ptr->receivedDataLength >= RX_BUFFLEN) {
		cleanRx(uart1_ptr);
		uart1_ptr->receivedDataLength = 0;
	}
	HAL_UART_Receive_IT(&huart1, &rxData, 1);
	uart1_ptr->receiveBuffer[uart1_ptr->receivedDataLength++] = rxData;
	if (rxData == LTEL_END_MARK)
		uart1_ptr->isReceivedDataReady = true;
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi) {
	printf("SPI TX Done .. Do Something ...");
}

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi) {
	printf("SPI RX Done .. Do Something ...");
}

void processReceivedUartCommand(Vlad_t *vlad, UART1_t *u1, RDSS_t *rdss,
		SX1278_t *loRa) {
	u1->transmittedDataLength = setBufferWithLtelCmd(u1->transmitBuffer, rdss,
			loRa, vlad);
	writeTx(u1);
	printStatus(u1, rdss);
	reinit(rdss);
	cleanTx(u1);
	u1->transmittedDataLength = 0;
}

void processServerReceivedUartCommand(Server_t *server, UART1_t *u1,
		RDSS_t *rdss, SX1278_t *loRa) {
	u1->transmittedDataLength = setBufferWithLtelServerCmd(u1->transmitBuffer,
			rdss, loRa, server);
	writeTx(u1);
	printStatus(u1, rdss);
	reinit(rdss);
	cleanTx(u1);
	u1->transmittedDataLength = 0;
}

uint8_t decodeVlad(Vlad_t *vlad, uint8_t *frame) {
	uint8_t index = 0;

	if (vlad == NULL)
		return index;
	vlad->state = frame[index++];
	vlad->lineVoltageReal = decodeUint16(frame, &index) / 10.0;
	vlad->lineCurrentReal = decodeUint16(frame, &index) / 1000.0;
	vlad->tone_level = decodeUint16(frame, &index);

	vlad->level152m_real = frame[index++] / 10;
	vlad->level152m_real = frame[index++];

	vlad->agc172m_real = frame[index++] / 10;
	vlad->level172m_real = frame[index++];

	return index;
}

uint16_t decodeUint16(uint8_t *frame, uint8_t *index) {
	uint16_t result = ((uint16_t) frame[*index])
			| ((uint16_t) frame[(*index) + 1] << 8);
	(*index) += 2;
	return result;
}
uint8_t setBufferWithLtelCmd(uint8_t *buffer, RDSS_t *rdss, SX1278_t *loRa,
		Vlad_t *vlad) {
	uint8_t index = 0;
	HAL_StatusTypeDef res;

	if (rdss->len < LTEL_QUERY_LENGTH || rdss->len > LTEL_SET_LENGTH)
		return 0;

	index = setRdssStartData(rdss, buffer, vlad->function);

	switch (rdss->cmd) {
	case QUERY_STATUS:
		index += encodeVladToLtel(buffer + index, vlad);
		break;
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
		buffer[index++] = vlad->function;
		buffer[index++] = rdss->id;
		buffer[index++] = QUERY_MODULE_ID;
		buffer[index++] = 0x00;
		buffer[index++] = 2;
		buffer[index++] = vlad->function;
		buffer[index++] = rdss->id;
		break;
	case SET_MODULE_ID:
		index = handleSetModuleID(buffer, rdss, vlad);
		break;
	case SET_TX_FREQ:
		buffer[index++] = 4;
		loRa->upFreq = freqDecode(rdss->buffer + index);
		index += sizeof(loRa->upFreq);
		res = HAL_savePage(M24C64_PAGE1, (uint8_t*) &(loRa->upFreq), 0, 4);
		break;
	case SET_RX_FREQ:
		buffer[index++] = 4;
		loRa->dlFreq = freqDecode(rdss->buffer + index);
		index += sizeof(loRa->dlFreq);
		res = HAL_savePage(M24C64_PAGE1, (uint8_t*) &(loRa->dlFreq), 4, 4);
		break;
	case SET_BANDWIDTH:
		buffer[index++] = 1;
		loRa->bandwidth = rdss->buffer[index++] - 1;
		res = HAL_savePage(M24C64_PAGE0, &(loRa->bandwidth), 1, 1);
		break;
	case SET_SPREAD_FACTOR:
		buffer[index++] = 1;
		loRa->spreadFactor = rdss->buffer[index++] + 6;
		res = HAL_savePage(M24C64_PAGE0, &(loRa->spreadFactor), 0, 1);
		break;
	case SET_CODING_RATE:
		buffer[index++] = 1;
		loRa->codingRate = rdss->buffer[index++];
		res = HAL_savePage(M24C64_PAGE0, &(loRa->codingRate), 2, 1);
		break;

	default:
		break;
	}

	index += setCrc(buffer, index);
	buffer[index++] = LTEL_END_MARK;
	rdss->status = UART_SEND;
	if (res == HAL_OK)
		changeLoRaOperatingMode(loRa, MASTER_RECEIVER);

	return index;
}

uint8_t setBufferWithLtelServerCmd(uint8_t *buffer, RDSS_t *rdss,
		SX1278_t *loRa, Server_t *server) {
	uint8_t index = 0;
	HAL_StatusTypeDef res;

	if (rdss->len < LTEL_QUERY_LENGTH || rdss->len > LTEL_SET_LENGTH)
		return 0;

	index = setRdssStartData(rdss, buffer, server->function);

	switch (rdss->cmd) {
	case QUERY_STATUS:
//		index += encodeVladToLtel(buffer + index, vlad);
		index = 0;
		break;
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
		buffer[index++] = server->function;
		buffer[index++] = rdss->id;
		buffer[index++] = QUERY_MODULE_ID;
		buffer[index++] = 0x00;
		buffer[index++] = 2;
		buffer[index++] = server->function;
		buffer[index++] = rdss->id;
		break;
	case SET_MODULE_ID:
		index = handleSetServerModuleID(buffer, rdss, server);
		break;
	case SET_TX_FREQ:
		buffer[index++] = 4;
		loRa->upFreq = freqDecode(rdss->buffer + index);
		index += sizeof(loRa->upFreq);
		res = HAL_savePage(M24C64_PAGE1, (uint8_t*) &(loRa->upFreq), 0, 4);
		break;
	case SET_RX_FREQ:
		buffer[index++] = 4;
		loRa->dlFreq = freqDecode(rdss->buffer + index);
		index += sizeof(loRa->dlFreq);
		res = HAL_savePage(M24C64_PAGE1, (uint8_t*) &(loRa->dlFreq), 4, 4);
		break;
	case SET_BANDWIDTH:
		buffer[index++] = 1;
		loRa->bandwidth = rdss->buffer[index++] - 1;
		res = HAL_savePage(M24C64_PAGE0, &(loRa->bandwidth), 1, 1);
		break;
	case SET_SPREAD_FACTOR:
		buffer[index++] = 1;
		loRa->spreadFactor = rdss->buffer[index++] + 6;
		res = HAL_savePage(M24C64_PAGE0, &(loRa->spreadFactor), 0, 1);
		break;
	case SET_CODING_RATE:
		buffer[index++] = 1;
		loRa->codingRate = rdss->buffer[index++];
		res = HAL_savePage(M24C64_PAGE0, &(loRa->codingRate), 2, 1);
		break;

	default:
		break;
	}

	index += setCrc(buffer, index);
	buffer[index++] = LTEL_END_MARK;
	rdss->status = UART_SEND;
	if (res == HAL_OK)
		changeLoRaOperatingMode(loRa, MASTER_RECEIVER);

	return index;
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

void parseUartMaster(UART1_t *uart1, RDSS_t *rdss) {
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

void sendQueryToLoRa(RDSS_t *rdss, SX1278_t *loRaModule,
		UART1_t *uart1Interface) {
	const uint8_t expectedQueryLength = 9;
	if (rdss->len != expectedQueryLength)
		return;
	rdss->idQuery = rdss->buffer[MODULE_ID_INDEX];
	loRaModule->len = rdss->len;
	changeLoRaOperatingMode(loRaModule, MASTER_SENDER);
	memcpy(loRaModule->buffer, rdss->buffer, rdss->len);
	loRaModule->status = TX_BUFFER_READY;
	printLoRaStatus(uart1Interface, loRaModule);
	transmitDataUsingLoRa(loRaModule);
}

GPIO_PinState processLoRaSlaveReceiver(SX1278_t *loRa, UART1_t *u1) {
	if (loRa->operatingMode != RX_CONTINUOUS) {
		changeLoRaOperatingMode(loRa, SLAVE_RECEIVER);
		setRxFifoAddr(loRa);
		setLoraLowFreqModeReg(loRa, RX_CONTINUOUS);
	}
	clearMemForRx(loRa);
	GPIO_PinState bussy = HAL_GPIO_ReadPin(LORA_BUSSY_GPIO_Port,
	LORA_BUSSY_Pin);
	if (bussy == GPIO_PIN_SET)
		if (crcErrorActivation(loRa) != 1) {
			getRxFifoData(loRa);
			printLoRaStatus(u1, loRa);
		}

	if (loRa->status == RX_DONE) {
		setRxFifoAddr(loRa);
		setLoraLowFreqModeReg(loRa, RX_CONTINUOUS);
		readOperatingMode(loRa);
		loRa->status = RX_MODE;
		printLoRaStatus(u1, loRa);
	}
	return bussy;
}

// Function to process LORA receive state
void processLoraReceiveState(UART1_t *uartHandle, RDSS_t *rdssHandle,
		Vlad_t *vladHandle, SX1278_t *loRaHandle) {
	printStatus(uartHandle, rdssHandle);
	if (rdssHandle->cmd == QUERY_STATUS && rdssHandle->len == 21) {
		decodeVlad(vladHandle, rdssHandle->buffer);
		memcpy(uartHandle->transmitBuffer, rdssHandle->buffer, rdssHandle->len);
		uartHandle->transmittedDataLength = rdssHandle->len;
		writeTx(uartHandle);
		uartHandle->transmittedDataLength = 0;
		print_parameters(uartHandle, *vladHandle);
	}
	printLoRaStatus(uartHandle, loRaHandle);
	reinit(rdssHandle);
}

// Function to process LORA send state
void processLoraSendState(UART1_t *uartHandle, RDSS_t *rdssHandle,
		SX1278_t *loRaHandle) {
	printStatus(uartHandle, rdssHandle);
	if (rdssHandle->cmd != 0) {
		sendQueryToLoRa(rdssHandle, loRaHandle, uartHandle);
	}
	printLoRaStatus(uartHandle, loRaHandle);
	reinit(rdssHandle);
	changeLoRaOperatingMode(loRaHandle, MASTER_RECEIVER);
	printLoRaStatus(uartHandle, loRaHandle);
}

// Function to process LORA master receive mode
void processMasterReceiverMode(UART1_t *uartHandle, SX1278_t *loRaHandle) {
	RX_MODE_ON_LED();
	TX_MODE_OFF_LED();
	if (loRaHandle->operatingMode != RX_CONTINUOUS) {
		changeLoRaOperatingMode(loRaHandle, MASTER_RECEIVER);
		setRxFifoAddr(loRaHandle);
		setLoraLowFreqModeReg(loRaHandle, RX_CONTINUOUS);
		printLoRaStatus(uartHandle, loRaHandle);
	}
	clearMemForRx(loRaHandle);
	GPIO_PinState bussy = HAL_GPIO_ReadPin(LORA_BUSSY_GPIO_Port,
	LORA_BUSSY_Pin);
	if (bussy == GPIO_PIN_SET && crcErrorActivation(loRaHandle) != 1) {
		getRxFifoData(loRaHandle);
		printLoRaStatus(uartHandle, loRaHandle);
	}
	if (loRaHandle->status == RX_DONE) {
		setRxFifoAddr(loRaHandle);
		setLoraLowFreqModeReg(loRaHandle, RX_CONTINUOUS);
		loRaHandle->status = RX_MODE;
		printLoRaStatus(uartHandle, loRaHandle);
	}
}

uint8_t handleSetModuleID(uint8_t *buffer, RDSS_t *rdss, Vlad_t *vlad) {
	vlad->function = rdss->buffer[6];
	vlad->id = rdss->buffer[7];
	rdss->id = vlad->id;
	uint8_t index = setRdssStartData(rdss, buffer, vlad->function);
	buffer[index++] = VLADR;
	buffer[index++] = rdss->id;
	if (HAL_savePage(M24C64_PAGE0, (uint8_t*) &(vlad->function), 3, 1)
			!= HAL_OK) {
		// Add error handling here
	}
	if (HAL_savePage(M24C64_PAGE0, (uint8_t*) &(vlad->id), 4, 1) != HAL_OK) {
		// Add error handling here
	}
	return index;
}

uint8_t handleSetServerModuleID(uint8_t *buffer, RDSS_t *rdss, Server_t *server) {
	server->function = rdss->buffer[6];
	server->id = rdss->buffer[7];
	rdss->id = server->id;
	uint8_t index = setRdssStartData(rdss, buffer, server->function);
	buffer[index++] = VLADR;
	buffer[index++] = rdss->id;
	if (HAL_savePage(M24C64_PAGE0, (uint8_t*) &(server->function), 3, 1)
			!= HAL_OK) {
		// Add error handling here
	}
	if (HAL_savePage(M24C64_PAGE0, (uint8_t*) &(server->id), 4, 1) != HAL_OK) {
		// Add error handling here
	}
	return index;
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
