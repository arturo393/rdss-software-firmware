/*
 * i2c.c
 *
 *  Created on: Aug 16, 2022
 *      Author: sigmadev
 */

#include <i2c1_master.h>

void i2cCleanBuffer(I2C_t *i2c) {
	for (int i = 0; i < I2C_RX_BUFFER_SIZE; i++)
		i2c->data_rx[i] = '\0';
	for (int i = 0; i < I2C_TX_BUFFER_SIZE; i++)
		i2c->data_tx[i] = '\0';
}

void i2c1MasterInit() {

	i2c1GpioInit();
	/* clock enable */
	SET_BIT(RCC->APBENR1, RCC_APBENR1_I2C1EN);
	/* peripherial disable */
	CLEAR_BIT(I2C1->CR1, I2C_CR1_PE);
	/* set timing */
	I2C1->TIMINGR = 0x00303D5B;
	/* */
	CLEAR_BIT(I2C1->OAR1, I2C_OAR1_OA1EN);
	CLEAR_BIT(I2C1->OAR2, I2C_OAR2_OA2EN);

	/*tx and rx interrupt enable */
	SET_BIT(I2C1->CR1, I2C_CR1_RXIE);
	SET_BIT(I2C1->CR1, I2C_CR1_TXIE);

	/* peripherial enable */
	SET_BIT(I2C1->CR1, I2C_CR1_PE);
}

void i2c1GpioInit() {

	SET_BIT(RCC->IOPENR, RCC_IOPENR_GPIOBEN);
	/* SCL PB6  as alternate */
	CLEAR_BIT(GPIOB->MODER, GPIO_MODER_MODE6_0);
	SET_BIT(GPIOB->MODER, GPIO_MODER_MODE6_1);
	/* SDA PB7 as alternate */
	CLEAR_BIT(GPIOB->MODER, GPIO_MODER_MODE7_0);
	SET_BIT(GPIOB->MODER, GPIO_MODER_MODE7_1);
	/* SCL PB6  as open-drain */
	SET_BIT(GPIOB->OTYPER, GPIO_OTYPER_OT6);
	/* SDC PB7  as open-drain */
	SET_BIT(GPIOB->OTYPER, GPIO_OTYPER_OT7);
	/* SCL PB6 High Speed output */
	//SET_BIT(GPIOB->OSPEEDR, GPIO_OSPEEDR_OSPEED6_1 | GPIO_OSPEEDR_OSPEED6_0);
	/* SDC PB7  High Speed output */
	//SET_BIT(GPIOB->OSPEEDR, GPIO_OSPEEDR_OSPEED7_1 |GPIO_OSPEEDR_OSPEED7_0);

	CLEAR_BIT(GPIOB->AFR[0], GPIO_AFRL_AFSEL6);
	SET_BIT(GPIOB->AFR[0], GPIO_AFRL_AFSEL6 << 1);
	SET_BIT(GPIOB->AFR[0], GPIO_AFRL_AFSEL6<<2);
	CLEAR_BIT(GPIOB->AFR[0], GPIO_AFRL_AFSEL6<<3);

	CLEAR_BIT(GPIOB->AFR[0], GPIO_AFRL_AFSEL7);
	SET_BIT(GPIOB->AFR[0], GPIO_AFRL_AFSEL7<<1);
	SET_BIT(GPIOB->AFR[0], GPIO_AFRL_AFSEL7<<2);
	CLEAR_BIT(GPIOB->AFR[0], GPIO_AFRL_AFSEL7<<3);
}

char i2c2MasterByteRx(char saddr, uint8_t N) {
	uint32_t counter = HAL_GetTick();
	bool timeout = false;
	i2c1MasterStartTransfer(saddr, READ, N);

	char data = 0;
	for (int i = 0; i < N; i++) {
		while (!READ_BIT(I2C2->ISR, I2C_ISR_RXNE) & !timeout) {
			if (HAL_GetTick() - counter > 500) {
				CLEAR_BIT(I2C2->CR1, I2C_CR1_PE);
				return 0x00;
			}
		}
		data = READ_REG(I2C2->RXDR);

	}
	while (!(READ_BIT(I2C2->ISR, I2C_ISR_STOPF))) {
	}
	SET_BIT(I2C2->ICR, I2C_ICR_STOPCF);

	return data;
}

void i2c1MasterFrameRx(char saddr, uint8_t *rcv, uint8_t N) {
	uint32_t counter = HAL_GetTick();
	bool timeout = false;
	i2c1MasterStartTransfer(saddr, READ, N);

	for (int i = 0; i < N; i++) {
		while (!READ_BIT(I2C1->ISR, I2C_ISR_RXNE) & !timeout) {
			if (HAL_GetTick() - counter > I2C_TIMEOUT_MS) {
				CLEAR_BIT(I2C1->CR1, I2C_CR1_PE);
				return;
			}
		}
		rcv[i] = READ_REG(I2C1->RXDR);

	}
	while (!(READ_BIT(I2C1->ISR, I2C_ISR_STOPF))) {
	}
	SET_BIT(I2C1->ICR, I2C_ICR_STOPCF);
}

void i2c1MasterStartTransfer(char saddr, uint8_t transfer_request, uint8_t N) {
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
	bool timeout = false;

	for (int i = 1; i < max_addr; i++) {
		i2c1MasterStartTransfer(i << 1 | 1, READ, 1);
		timeout = false;

		while (!READ_BIT(I2C1->ISR, I2C_ISR_RXNE) & !timeout) {
			if (HAL_GetTick() - counter > 50) {
				counter = HAL_GetTick();
				timeout = true;
			}
		}

		if (!timeout) {
			addr[j++] = i;
			READ_REG(I2C1->RXDR);
		}
	}

}

void i2c1MasterByteTx(uint8_t saddr, uint8_t *data, uint8_t N) {
	i2c1MasterStartTransfer(saddr, WRITE, N);
	uint32_t counter = HAL_GetTick();

	for (int i = 0; i < N; i++) {
		while (!READ_BIT(I2C1->ISR, I2C_ISR_TXIS)) {

			if (HAL_GetTick() - counter > I2C_TIMEOUT_MS) {
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

uint8_t i2c1SlaveTx(I2C_t *i2c) {
	uint8_t i;
	uint32_t I2C_InterruptStatus = I2C1->ISR; /* Get interrupt status */
	/* Check address match */

	/* Check if transfer direction is read (slave transmitter) */
	if ((I2C1->ISR & I2C_ISR_DIR) == I2C_ISR_DIR) {
		I2C1->CR1 |= I2C_CR1_TXIE; /* Set transmit IT */
	}
	for (i = 0; i < i2c->rx_count; i++) {
		if ((I2C_InterruptStatus & I2C_ISR_TXIS) == I2C_ISR_TXIS) {
			I2C1->TXDR = i2c->data_tx[i]; /* Byte to send */
		}
	}
	I2C1->CR1 &= ~ I2C_CR1_TXIE; /* Disable transmit IT */

	return i;
}

uint8_t isCrcValid(I2C_t *i2c) {
	uint8_t *rx = &(i2c->data_rx);
	uint32_t crc_calculated = 0;
	int len = strlen(i2c->data_rx) - 1;
	uint32_t crc_received = (rx[len - 3] << 24) | (rx[len - 2] << 16)
			| (rx[len - 1] << 8) | (rx[len]);
	if (len == 2 + 4 - 1)
		crc_calculated = HAL_CRC_Calculate(&hcrc, (uint32_t*) i2c->data_rx, 2);
	else if (len == 1 + 4 - 1)
		crc_calculated = HAL_CRC_Calculate(&hcrc, (uint32_t*) i2c->data_rx, 1);
	if (crc_calculated == crc_received)
		return 1;
	return 0;
}

void i2c1SlaveDelayedReset(I2C_t *i2c_slave, uint32_t timeout) {
	if (READ_BIT(I2C1->ISR, I2C_ISR_BUSY)) {
		if (HAL_GetTick() - i2c_slave->ticks > timeout) {
			CLEAR_BIT(I2C1->CR1, I2C_CR1_PE);
			SET_BIT(I2C1->ICR, I2C_ICR_STOPCF);
			HAL_Delay(1);
			SET_BIT(I2C1->CR1, I2C_CR1_PE);
			i2c_slave->ticks = HAL_GetTick();
		}
	}
}

bool i2c1SlaveAddrsMatch(I2C_t *i2c_slave) {
	uint32_t I2C_InterruptStatus = I2C1->ISR;
	/* Get interrupt status */
	if ((I2C_InterruptStatus & I2C_ISR_ADDR)
			== I2C_ISR_ADDR) {
		I2C1->ICR |= I2C_ICR_ADDRCF;
		i2c_slave->rx_count = 0;
	return true;
	}
	return false;
}

void i2c1SalveTx(const I2C_t *i2c_slave) {
	/* Check if transfer direction is read (slave transmitter) */
	if ((I2C1->ISR & I2C_ISR_DIR) == I2C_ISR_DIR) {
		I2C1->CR1 |= I2C_CR1_TXIE;
		uint32_t timeout = HAL_GetTick();
		for (int i = 0; (i < i2c_slave->tx_count); i++) {
			if (READ_BIT(I2C1->ISR, I2C_ISR_TXIS))
				I2C1->TXDR = i2c_slave->data_tx[i];
			/* Byte to send */}
		I2C1->CR1 &= ~ I2C_CR1_TXIE;
	}
}

void i2c1SalveRx(I2C_t *i2c_slave) {
	uint32_t I2C_InterruptStatus = I2C1->ISR;
	if ((I2C_InterruptStatus & I2C_ISR_RXNE) == I2C_ISR_RXNE)
		i2c_slave->data_rx[i2c_slave->rx_count++] = I2C1->RXDR;
}


