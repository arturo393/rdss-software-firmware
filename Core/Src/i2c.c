/*
 * i2c.c
 *
 *  Created on: Aug 16, 2022
 *      Author: sigmadev
 */

#include <i2c.h>


void i2c1MasterInit() {
	// Enable GPIOB clock
	SET_BIT(RCC->IOPENR, RCC_IOPENR_GPIOBEN);

	// Set SCL and SDA pins as alternate function
	CLEAR_BIT(GPIOB->MODER, GPIO_MODER_MODE6_0 | GPIO_MODER_MODE7_0);
	SET_BIT(GPIOB->MODER, GPIO_MODER_MODE6_1 | GPIO_MODER_MODE7_1);

	// Set SCL and SDA pins as open-drain
	SET_BIT(GPIOB->OTYPER, GPIO_OTYPER_OT6 | GPIO_OTYPER_OT7);

	// Set alternate function to I2C
	CLEAR_BIT(GPIOB->AFR[0],
			GPIO_AFRL_AFSEL6 | GPIO_AFRL_AFSEL6_1 | GPIO_AFRL_AFSEL6_2 | GPIO_AFRL_AFSEL6_3);
	SET_BIT(GPIOB->AFR[0], GPIO_AFRL_AFSEL6_1 | GPIO_AFRL_AFSEL6_2);
	CLEAR_BIT(GPIOB->AFR[0],
			GPIO_AFRL_AFSEL7 | GPIO_AFRL_AFSEL7_1 | GPIO_AFRL_AFSEL7_2 | GPIO_AFRL_AFSEL7_3);
	SET_BIT(GPIOB->AFR[0], GPIO_AFRL_AFSEL7_1 | GPIO_AFRL_AFSEL7_2);

	// Enable I2C1 clock
	SET_BIT(RCC->APBENR1, RCC_APBENR1_I2C1EN);

	// Disable I2C1
	CLEAR_BIT(I2C1->CR1, I2C_CR1_PE);

	// Set I2C1 timing
	I2C1->TIMINGR = 0x00303D5B;

	// Disable I2C1 own address
	CLEAR_BIT(I2C1->OAR1, I2C_OAR1_OA1EN);
	CLEAR_BIT(I2C1->OAR2, I2C_OAR2_OA2EN);

	// Enable I2C1 RX and TX interrupts
	SET_BIT(I2C1->CR1, I2C_CR1_RXIE | I2C_CR1_TXIE);

	// Enable I2C1
	SET_BIT(I2C1->CR1, I2C_CR1_PE);
}

uint8_t i2c1MasterFrameRx(uint8_t saddr, uint8_t *rcv, uint8_t N) {
	uint32_t counter = HAL_GetTick();
	bool timeout = false;
	uint8_t i = 0;
	i2c1MasterStartTransfer(saddr, I2C_READ, N);

	for (i = 0; i < N; i++) {
		while (!READ_BIT(I2C1->ISR, I2C_ISR_RXNE) & !timeout) {
			if (HAL_GetTick() - counter > I2C_TIMEOUT_MS) {
				CLEAR_BIT(I2C1->CR1, I2C_CR1_PE);
				return i;
			}
		}
		rcv[i] = READ_REG(I2C1->RXDR);

	}
	while (!(READ_BIT(I2C1->ISR, I2C_ISR_STOPF))) {
	}
	SET_BIT(I2C1->ICR, I2C_ICR_STOPCF);
	return i;
}

void i2c1MasterStartTransfer(uint8_t saddr, uint8_t transfer_request, uint8_t N) {
	/* peripherial disable */
	SET_BIT(I2C1->CR1, I2C_CR1_PE);
	/* set slave address */
	MODIFY_REG(I2C1->CR2, I2C_CR2_SADD, saddr << I2C_CR2_SADD_Pos);
	/* read N byte */
	MODIFY_REG(I2C1->CR2, I2C_CR2_NBYTES, N << I2C_CR2_NBYTES_Pos);
	/* stops when NBytes are transferred */
	SET_BIT(I2C1->CR2, I2C_CR2_AUTOEND);
	/* set START condition  automatically changes to master */
	if (transfer_request == 1) {
		/* request a read transfer */
		SET_BIT(I2C1->CR2, I2C_CR2_RD_WRN);
	} else if (transfer_request == 0) {
		/* request a write transfer */
		CLEAR_BIT(I2C1->CR2, I2C_CR2_RD_WRN);
	}

	SET_BIT(I2C1->CR2, I2C_CR2_START);

}

void i2c1AddresScanner(uint8_t *addr, uint8_t max_addr) {
	uint32_t counter = HAL_GetTick();
	uint8_t j = 0;
	bool waintUntilTimeout = false;

	for (int i = 1; i < max_addr; i++) {
		i2c1MasterStartTransfer(i << 1 | 1, I2C_READ, 1);
		waintUntilTimeout = false;

		while (!READ_BIT(I2C1->ISR, I2C_ISR_RXNE) & !waintUntilTimeout) {
			if (HAL_GetTick() - counter > 50) {
				counter = HAL_GetTick();
				waintUntilTimeout = true;
			}
		}

		if (!waintUntilTimeout) {
			addr[j++] = i;
			READ_REG(I2C1->RXDR);
		}
	}

}

void i2c1MasterByteTx(uint8_t slave_address, uint8_t *data, uint8_t num_bytes) {
	i2c1MasterStartTransfer(slave_address, I2C_WRITE, num_bytes);
	uint32_t startTick = HAL_GetTick();

	for (int i = 0; i < num_bytes; i++) {
		while (!READ_BIT(I2C1->ISR, I2C_ISR_TXIS)) {

			if (HAL_GetTick() - startTick > I2C_TIMEOUT_MS) {
				CLEAR_BIT(I2C1->CR1, I2C_CR1_PE);
				return;
			}
		}
		MODIFY_REG(I2C1->TXDR, I2C_TXDR_TXDATA, data[i]);
	}

	while (!READ_BIT(I2C1->ISR, I2C_ISR_STOPF)) {
	}
	SET_BIT(I2C1->ISR, I2C_ICR_STOPCF);

}

uint8_t isCrcValid(I2C_t *i2c) {
	uint8_t *rx = &(i2c->rx);
	uint32_t crc_calculated = 0;
	int len = strlen(i2c->rx) - 1;
	uint32_t crc_received = (rx[len - 3] << 24) | (rx[len - 2] << 16)
			| (rx[len - 1] << 8) | (rx[len]);
	if (len == 2 + 4 - 1)
		crc_calculated = HAL_CRC_Calculate(&hcrc, (uint32_t*) i2c->rx, 2);
	else if (len == 1 + 4 - 1)
		crc_calculated = HAL_CRC_Calculate(&hcrc, (uint32_t*) i2c->rx, 1);
	if (crc_calculated == crc_received)
		return 1;
	return 0;
}

void i2cCleanBuffer(I2C_t *i2c) {
	for (int i = 0; i < I2C_RX_BUFFER_SIZE; i++)
		i2c->rx[i] = '\0';
	for (int i = 0; i < I2C_TX_BUFFER_SIZE; i++)
		i2c->tx[i] = '\0';
}
