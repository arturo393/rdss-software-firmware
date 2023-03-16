/*
 * uart1.c
 *
 *  Created on: Aug 29, 2022
 *      Author: sigmadev
 */

#include <uart1.h>

uint8_t  uart1_clean_by_timeout(UART1_t* uart1,const char* str){
		if (HAL_GetTick() - uart1->timeout > SECONDS(5)) {
			uart1_send_str((char*)str);
			uart1_send_str("-TIMEOUT\r\n");
			if(strlen(str)>0)
				uart1_clean_buffer(uart1);
			uart1->timeout = HAL_GetTick();
			return 1;
		}
		return 0;
}

void uart1_gpio_init() {
	/**USART1 GPIO Configuration
	 PA9     ------> USART1_TX
	 PA10    ------> USART1_RX **/

	/* PA10 alternate mode */
	CLEAR_BIT(GPIOA->MODER, GPIO_MODER_MODE10_0);
	SET_BIT(GPIOA->MODER, GPIO_MODER_MODE10_1);

	CLEAR_BIT(GPIOA->MODER, GPIO_MODER_MODE9_0);
	SET_BIT(GPIOA->MODER, GPIO_MODER_MODE9_1);

	SET_BIT(GPIOA->AFR[1], GPIO_AFRH_AFSEL10_0);
	CLEAR_BIT(GPIOA->AFR[1], GPIO_AFRH_AFSEL10_1);
	CLEAR_BIT(GPIOA->AFR[1], GPIO_AFRH_AFSEL10_2);
	CLEAR_BIT(GPIOA->AFR[1], GPIO_AFRH_AFSEL10_3);

	SET_BIT(GPIOA->AFR[1], GPIO_AFRH_AFSEL9_0);
	CLEAR_BIT(GPIOA->AFR[1], GPIO_AFRH_AFSEL9_1);
	CLEAR_BIT(GPIOA->AFR[1], GPIO_AFRH_AFSEL9_2);
	CLEAR_BIT(GPIOA->AFR[1], GPIO_AFRH_AFSEL9_3);
}

void uart1Init(uint32_t pclk, uint32_t baud_rate, UART1_t *u) {
	uint32_t br_value = 0;

	uart1_gpio_init();

	/*enable clock access to USART1 */
	SET_BIT(RCC->APBENR2, RCC_APBENR2_USART1EN);
	if (pclk == 16000000) {
		/*set HSI 16 CLK */
		CLEAR_BIT(RCC->CCIPR, RCC_CCIPR_USART1SEL_0);
		SET_BIT(RCC->CCIPR, RCC_CCIPR_USART1SEL_1);
	}
	//MODIFY_REG(USART1->PRESC,USART_PRESC_PRESCALER,0x0010);
	/* set baud rate */
	br_value = (pclk) / baud_rate;
	USART1->BRR = (uint16_t) br_value;
	/* transmitter enable*/
	USART1->CR1 = USART_CR1_TE | USART_CR1_RE;
	u->rxCount = 0;

	uart1_clean_buffer(u);

	/* enable FIFO */
	//SET_BIT(USART1->CR2, USART_CR1_FIFOEN);
	/* Enable interrupt */
	SET_BIT(USART1->CR1, USART_CR1_RXNEIE_RXFNEIE);
	NVIC_EnableIRQ(USART1_IRQn);
	SET_BIT(USART1->CR1, USART_CR1_UE);
}

void uart1_dma_init() {
	/* enable clk access */
	SET_BIT(RCC->AHBENR, RCC_AHBENR_DMA1EN);
	/* clear all interrupt flags on stream */

	/* set the destination buffer */
	/*set the source buffer */

}

void uart1_write(char ch) {
	SET_BIT(GPIOB->ODR, GPIO_ODR_OD9);

	while (!READ_BIT(USART1->ISR, USART_ISR_TXE_TXFNF))
		;
	USART1->TDR = (uint8_t) (ch & 0xFFU);

	while (!READ_BIT(USART1->ISR, USART_ISR_TC))
		;

	CLEAR_BIT(GPIOB->ODR, GPIO_ODR_OD9);
}

void uart1_read(char *data, uint8_t size) {
	bool override = READ_BIT(USART1->ISR, USART_ISR_ORE);
	bool data_present = READ_BIT(USART1->ISR, USART_ISR_RXNE_RXFNE);
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
			data[i] = USART1->RDR;
			if (override)
				SET_BIT(USART1->ICR, USART_ICR_ORECF);
		}
		rxfne = 0;
	} else {
		while (!READ_BIT(USART1->ISR, USART_ISR_RXNE_RXFNE)) {
		}
		data[0] = USART1->RDR;
	}
}

uint8_t uart1_1byte_read(void) {
	volatile uint8_t data;
	bool override = READ_BIT(USART1->ISR, USART_ISR_ORE);
	bool data_present = READ_BIT(USART1->ISR, USART_ISR_RXNE_RXFNE);
//	bool busy = READ_BIT(USART1->ISR, USART_ISR_BUSY);
	if ((data_present || override)) {
		if (override)
			SET_BIT(USART1->ICR, USART_ICR_ORECF);
		data = USART1->RDR;
		return data;
	} else
		return '\0';
}

void  uart1_read_to_frame(UART1_t *u) {
	if (u->rxCount >= RX_BUFFLEN) {
		uart1_clean_buffer(u);
		u->rxCount = 0;
	}
	u->rxBuffer[u->rxCount++] = uart1_1byte_read();
}

void uart1_send_str(char *str) {
	uint8_t i;
	for (i = 0; str[i] != '\0'; i++)
		uart1_write(str[i]);
}

void uart1_send_frame(uint8_t str[], uint8_t len) {

	if (len > 0) {
		for (int i = 0; i < len; i++){
			uart1_write(str[i]);
			str[i] = (char) '\0';
		}
	}
}

void uart1_clean_buffer(UART1_t *u) {
	u->rxCount = 0;
	if (TX_BUFFLEN > RX_BUFFLEN) {
		for (int i = 0; i < TX_BUFFLEN; i++) {
			if (i < RX_BUFFLEN)
				u->rxBuffer[i] = 0x00;
			u->txBuffer[i] = 0x00;
		}
	} else {
		for (int i = 0; i < RX_BUFFLEN; i++) {
			if (i < TX_BUFFLEN)
				u->txBuffer[i] = 0x00;
			u->rxBuffer[i] = 0x00;
		}
	}
}

uint8_t uart1_nonblocking_read(void) {
	uint8_t rcvcount = 0;
	uint32_t tickstart = HAL_GetTick();
	bool timeout = false;
	uint8_t timeout_value = 10;

	while (rcvcount < RX_BUFFLEN && timeout == false) {
		if (((HAL_GetTick() - tickstart) > timeout_value)
				|| (timeout_value == 0U)) {
			timeout = true;
			if (READ_BIT(USART1->ISR, USART_ISR_ORE))
				SET_BIT(USART1->ICR, USART_ICR_ORECF);
			if (READ_BIT(USART1->ISR, USART_ISR_IDLE))
				SET_BIT(USART1->ICR, USART_ICR_IDLECF);
			if (READ_BIT(USART1->ISR, USART_ISR_FE))
				SET_BIT(USART1->ICR, USART_ICR_FECF);
		}

		if (READ_BIT(USART1->ISR, USART_ISR_RXNE_RXFNE))
			return USART1->RDR;
	}
	return '\0';
}
