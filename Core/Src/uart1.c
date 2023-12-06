/*
 * uart1.c
 *
 *  Created on: Aug 29, 2022
 *      Author: sigmadev
 */

#include <uart1.h>

uint8_t cleanByTimeout(UART1_t *uart1, const char *str) {
	if (HAL_GetTick() - uart1->operationTimeout > SECONDS(5)) {

		writeTxStr((char*) str);
		writeTxStr("-TIMEOUT\r\n");

		if (strlen(str) > 0)
			cleanTx(uart1);
		uart1->operationTimeout = HAL_GetTick();
		return 1;
	}
	return 0;
}

void uart1_gpio_init() {
	/**USART1 GPIO Configuration
	 PA2     ------> USART2_TX
	 PA3    ------> USART2_RX **/
	//alternate function output
	//open-drain = CNF1 1 ; CNF0 1 ; MODE1 Y MODE0 EN 01.
	/* PA3 alternate mode */
	SET_BIT(GPIOA->CRL, GPIO_CRL_CNF2_0);
	CLEAR_BIT(GPIOA->CRL, GPIO_CRL_CNF2_1);

	CLEAR_BIT(GPIOA->CRL, GPIO_CRL_MODE2_0);
	CLEAR_BIT(GPIOA->CRL, GPIO_CRL_MODE2_1);

	SET_BIT(GPIOA->CRL, GPIO_CRL_CNF3_0);
	CLEAR_BIT(GPIOA->CRL, GPIO_CRL_CNF3_1);

	CLEAR_BIT(GPIOA->CRL, GPIO_CRL_MODE3_0);
	CLEAR_BIT(GPIOA->CRL, GPIO_CRL_MODE3_1);

	SET_BIT(GPIOA->CRL, GPIO_CRL_CNF0_0);
	CLEAR_BIT(GPIOA->CRL, GPIO_CRL_CNF0_1);

	SET_BIT(GPIOA->CRL, GPIO_CRL_CNF1_0);
	CLEAR_BIT(GPIOA->CRL, GPIO_CRL_CNF1_1);

	SET_BIT(GPIOA->CRL, GPIO_CRL_CNF4_0);
	CLEAR_BIT(GPIOA->CRL, GPIO_CRL_CNF4_1);
}

void uart1Init(uint32_t pclk, uint32_t baud_rate, UART1_t *u) {
	uint32_t br_value = 0;

	//uart1_gpio_init();
// clock HSI de 8 MHz
	/*enable clock access to USART1 */
	SET_BIT(RCC->APB2ENR, RCC_APB2ENR_USART1EN);

	SET_BIT(RCC->APB2ENR, RCC_APB2ENR_SPI1EN);
	SET_BIT(RCC->APB2ENR, RCC_APB2ENR_ADC1EN);

	if (pclk == 16000000) {
		/*set HSI 16 CLK */
		//CLEAR_BIT(RCC->CCIPR, RCC_CCIPR_USART1SEL_0);
		SET_BIT(USART1->CR2, USART_CR2_CLKEN);
		//SET_BIT(RCC->CCIPR, RCC_CCIPR_USART1SEL_1);
	}
	//MODIFY_REG(USART1->PRESC,USART_PRESC_PRESCALER,0x0010);
	/* set baud rate */
	br_value = (pclk) / baud_rate;
	USART1->BRR = (uint16_t) br_value;
	/* transmitter enable*/
	USART1->CR1 = USART_CR1_TE | USART_CR1_RE;
	//uart1_clean_buffer(u);

	/* enable FIFO */
	//SET_BIT(USART1->CR2, USART_CR1_FIFOEN);
	/* Enable interrupt */
	SET_BIT(USART1->CR1, USART_CR1_RXNEIE);
	NVIC_EnableIRQ(USART1_IRQn);
	SET_BIT(USART1->CR1, USART_CR1_UE);
}

void writeTxReg(char ch) {
	SET_BIT(GPIOB->ODR, GPIO_ODR_ODR8);

	while (!READ_BIT(USART1->SR, USART_SR_TXE))
		;
	USART1->DR = (uint8_t) (ch & 0xFFU);

	while (!READ_BIT(USART1->SR, USART_SR_TC))
		;

	CLEAR_BIT(GPIOB->ODR, GPIO_ODR_ODR8);
}

void uart1_read(char *data, uint8_t size) {
	bool override = READ_BIT(USART1->SR, USART_SR_ORE);
	bool data_present = READ_BIT(USART1->SR, USART_SR_RXNE); // Read data register not empty
//	bool busy = READ_BIT(USART1->ISR, USART_ISR_BUSY);
	bool timeout = false;
	uint16_t rxfne = 0;
	if (size > 1) {
		for (int i = 0; (i < size - 1); i++) {
			while ((!data_present && !override && !timeout)) {
				if (rxfne > 1000)
					timeout = true;
				else
					rxfne++;
			}
			data[i] = USART1->DR; // DR = Data Register
			if (override)
				SET_BIT(USART1->SR, USART_SR_ORE); // Overrun error is detected
		}
		rxfne = 0;
	} else {
		while (!READ_BIT(USART1->SR, USART_SR_RXNE)) {
		}
		data[0] = USART1->DR;
	}
}

uint8_t readRxReg(void) {
	volatile uint8_t data;
	bool override = READ_BIT(USART1->SR, USART_SR_ORE);
	bool data_present = READ_BIT(USART1->SR, USART_SR_RXNE);
//	bool busy = READ_BIT(USART1->ISR, USART_ISR_BUSY);
	if ((data_present || override)) {
		if (override)
			SET_BIT(USART1->SR, USART_SR_ORE);
		data = USART1->DR;
		return data;
	} else
		return '\0';
}

void readRx(UART1_t *u) {
	if (u->rxSize >= UART2_RX_BUFFLEN) {
		cleanRx(u);
		u->rxSize = 0;
	}
	u->rxData[u->rxSize++] = readRxReg();
}

void writeTxStr(char *str) {
	for (uint8_t i = 0; str[i] != '\0'; i++)
		writeTxReg(str[i]);
}

void writeTxBuffer(uint8_t str[], uint8_t len) {
	for (uint8_t i = 0; i < len; i++) {
		writeTxReg(str[i]);
		str[i] = (char) '\0';
	}
}


void writeTx(UART1_t *uart1) {
	writeTxBuffer(uart1->txData, uart1->txSize);
}

void cleanRx(UART1_t *u) {
	memset(u->rxData, 0, sizeof(u->rxData));
	u->rxSize = 0;
	u->isReceivedDataReady = false;
}

void cleanTx(UART1_t *u) {
	memset(u->txData, 0, sizeof(u->txData));
	u->txSize = 0;
}
