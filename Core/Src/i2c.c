/*
 * i2c.c
 *
 *  Created on: Aug 16, 2022
 *      Author: sigmadev
 */

#include <i2c.h>

void i2c1MasterInit() {
	// Enable GPIOB clock
	RCC->IOPENR |= RCC_IOPENR_GPIOBEN;

	// Configure SCL and SDA pins (PB6 and PB7) as alternate function
	GPIOB->MODER &= ~(GPIO_MODER_MODE6 | GPIO_MODER_MODE7);
	GPIOB->MODER |= (GPIO_MODER_MODE6_1 | GPIO_MODER_MODE7_1);

	// Set SCL and SDA pins as open-drain
	GPIOB->OTYPER |= GPIO_OTYPER_OT6 | GPIO_OTYPER_OT7;

	// Set alternate function for SCL and SDA pins to I2C
	GPIOB->AFR[0] &= ~(GPIO_AFRL_AFSEL6_Msk | GPIO_AFRL_AFSEL7_Msk);
	GPIOB->AFR[0] |= (6U << GPIO_AFRL_AFSEL6_Pos)
			| (6U << GPIO_AFRL_AFSEL7_Pos);

	// Enable I2C1 clock
	RCC->APBENR1 |= RCC_APBENR1_I2C1EN;

	// Disable I2C1
	I2C1->CR1 &= ~I2C_CR1_PE;

	// Set I2C1 timing
	I2C1->TIMINGR = 0x00303D5B;

	// Disable I2C1 own address
	I2C1->OAR1 &= ~I2C_OAR1_OA1EN;
	I2C1->OAR2 &= ~I2C_OAR2_OA2EN;

	// Enable I2C1
	I2C1->CR1 |= I2C_CR1_PE;
}

uint8_t isCrcValid(I2C_t *i2c) {
	uint8_t *rx;
	rx = i2c->buffer;
	uint32_t crc_calculated = 0;
	uint8_t len = i2c->len - 1;
	uint32_t crc_received = (rx[len - 3] << 24) | (rx[len - 2] << 16)
			| (rx[len - 1] << 8) | (rx[len]);
	if (len == 2 + 4 - 1)
		crc_calculated = HAL_CRC_Calculate(&hcrc, (uint32_t*) i2c->buffer, 2);
	else if (len == 1 + 4 - 1)
		crc_calculated = HAL_CRC_Calculate(&hcrc, (uint32_t*) i2c->buffer, 1);
	if (crc_calculated == crc_received)
		return 1;
	return 0;
}

void i2cCleanBuffer(I2C_t *i2c) {
	for (int i = 0; i < I2C_RX_BUFFER_SIZE; i++)
		i2c->buffer[i] = '\0';
	for (int i = 0; i < I2C_TX_BUFFER_SIZE; i++)
		i2c->tx[i] = '\0';
}

bool i2c1WaitForBusyWithTimeout(uint32_t timeout) {
	uint32_t start = HAL_GetTick();
	while (I2C1->ISR & I2C_ISR_BUSY) {
		if (HAL_GetTick() - start >= timeout) {
			// Generate the stop condition
			I2C1->CR2 |= I2C_CR2_STOP;
			i2c1MasterInit();
			return false;
		}
	}
	return true;
}

bool i2c1MasterTransmit(uint8_t slave_addr, uint8_t *data, uint8_t len,
		uint32_t timeout) {
	uint32_t tick_start = HAL_GetTick();
	uint8_t index = 0;

	// Make sure the peripheral is enabled and not busy
	if (!(I2C1->CR1 & I2C_CR1_PE) || (I2C1->ISR & I2C_ISR_BUSY)) {
		return false;
	}

	// Set slave address, write transfer, and set number of bytes
	I2C1->CR2 = (slave_addr << 1) | (len << 16) | I2C_CR2_AUTOEND;

	// Send START condition
	SET_BIT(I2C1->CR2, I2C_CR2_START);

	// Transmit data
	while (index < len) {
		// Check for timeout
		if ((HAL_GetTick() - tick_start) > timeout) {
			if (I2C1->ISR & I2C_ISR_BUSY) {
				// Generate the stop condition
				I2C1->CR2 |= I2C_CR2_STOP;
				// Reinitialize the I2C peripheral if needed
				i2c1MasterInit();
				return false;
			}
		}

		// Check if TXIS flag is set (transmit data register empty)
		if (READ_BIT(I2C1->ISR, I2C_ISR_TXIS)) {
			// Write data to TXDR
			I2C1->TXDR = data[index++];
		}

		// Check for NACKF flag (Not Acknowledge Received)
		if (READ_BIT(I2C1->ISR, I2C_ISR_NACKF)) {
			// Generate the stop condition
			I2C1->CR2 |= I2C_CR2_STOP;
			// Clear NACKF flag
			SET_BIT(I2C1->ICR, I2C_ICR_NACKCF);
			// Reinitialize the I2C peripheral if needed
			i2c1MasterInit();
			return i2c1MasterTransmit(slave_addr, data, len, 10);
			//return false;
		}
	}

	// Wait for STOPF flag (stop detection)
	while (!READ_BIT(I2C1->ISR, I2C_ISR_STOPF)) {
		// Check for timeout
		if (HAL_GetTick() - tick_start > timeout) {
			return false;
		}
	}

	// Clear STOPF flag
	SET_BIT(I2C1->ICR, I2C_ICR_STOPCF);

	return true;
}

bool i2c1MasterReceive(uint8_t slave_addr, uint8_t *data, uint8_t len,
		uint32_t timeout) {
	uint32_t tick_start = HAL_GetTick();
	uint8_t index = 0;

	// Make sure the peripheral is enabled
	if (!(I2C1->CR1 & I2C_CR1_PE)) {
		return false;
	}

	// Set slave address, read transfer, and set number of bytes
	I2C1->CR2 = (slave_addr << 1) | I2C_CR2_RD_WRN | (len << 16)
			| I2C_CR2_AUTOEND;

	// Send START condition
	SET_BIT(I2C1->CR2, I2C_CR2_START);

	while (index < len) {
		// Check for timeout
		if (HAL_GetTick() - tick_start > timeout) {
			if (I2C1->ISR & I2C_ISR_BUSY) {
					// Generate the stop condition
					I2C1->CR2 |= I2C_CR2_STOP;
					i2c1MasterInit();
					return false;
			}
		}

		// Check if RXNE flag is set (receive data register not empty)
		if (READ_BIT(I2C1->ISR, I2C_ISR_RXNE)) {
			// Read data from RXDR
			data[index++] = (uint8_t) (I2C1->RXDR & 0xFF);
		}
	}

	while (!(READ_BIT(I2C1->ISR, I2C_ISR_STOPF))) {
	}
	SET_BIT(I2C1->ICR, I2C_ICR_STOPCF);

	return true;
}

