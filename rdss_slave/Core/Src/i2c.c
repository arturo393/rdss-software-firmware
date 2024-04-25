/*
 * i2c.c
 *
 *  Created on: Aug 16, 2022
 *      Author: sigmadev
 */

#include "i2c.h"

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

void i2cRestart(I2C_t *i2c, uint16_t timeout) {

	if (HAL_GetTick() - i2c->ticks > timeout) {
		// Disable I2C1 peripheral
		i2c->reg->CR1 &= ~I2C_CR1_PE;

		// Reset I2C1 peripheral
		RCC->APBRSTR1 |= RCC_APBRSTR1_I2C1RST;
		RCC->APBRSTR1 &= ~RCC_APBRSTR1_I2C1RST;
		// Configure I2C1 peripheral

		HAL_Delay(100);
		i2c->reg->CR1 &= ~I2C_CR1_PE; // Disable I2C1

		// Configure I2C1 timings and addressing mode here

		i2c->reg->CR1 |= I2C_CR1_PE; // Enable I2C1
		i2c->reg = I2C1;
		i2c->ticks = HAL_GetTick();



	}

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

bool i2c1MasterTransmit(uint8_t slaveAddr, uint8_t *data, uint8_t length,
		uint32_t timeout) {
	uint32_t tickStart = HAL_GetTick();
	uint8_t dataIndex = 0;
	uint8_t retries = 3;

	if (!(I2C1->CR1 & I2C_CR1_PE) || (I2C1->ISR & I2C_ISR_BUSY)) {
		I2C1->CR1 &= ~I2C_CR1_PE;
		I2C1->CR1 |= I2C_CR1_PE;
		return false;
	}

	I2C1->CR2 = (slaveAddr << 1) | (length << 16) | I2C_CR2_AUTOEND;
	SET_BIT(I2C1->CR2, I2C_CR2_START);

	while (dataIndex < length) {
		if (HAL_GetTick() - tickStart > timeout) {
			if (I2C1->ISR & I2C_ISR_BUSY) {
				I2C1->CR2 |= I2C_CR2_STOP;
				i2c1MasterInit();
				return false;
			}
		}

		if (READ_BIT(I2C1->ISR, I2C_ISR_TXIS)) {
			I2C1->TXDR = data[dataIndex++];
		}

		if (READ_BIT(I2C1->ISR, I2C_ISR_NACKF)) {
			if (--retries > 0) {
				// Generate the stop condition
				I2C1->CR2 |= I2C_CR2_STOP;
				// Clear NACKF flag
				SET_BIT(I2C1->ICR, I2C_ICR_NACKCF);
				// Reinitialize the I2C peripheral
				i2c1MasterInit();
				// Restart the transmission process
				dataIndex = 0;
				I2C1->CR2 = (slaveAddr << 1) | (length << 16) | I2C_CR2_AUTOEND;
				SET_BIT(I2C1->CR2, I2C_CR2_START);
			} else {
				I2C1->CR2 |= I2C_CR2_STOP;
				SET_BIT(I2C1->ICR, I2C_ICR_NACKCF);
				i2c1MasterInit();
				return false;
			}
		}
	}

	while (!READ_BIT(I2C1->ISR, I2C_ISR_STOPF)) {
		if (HAL_GetTick() - tickStart > timeout) {
			return false;
		}
	}

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
			}
			return false;
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

bool i2cMasterReceive(I2C_TypeDef *i2c, uint8_t slave_addr, uint8_t *data,
		uint8_t len, uint32_t timeout) {
	uint32_t tick_start = HAL_GetTick();
	uint8_t index = 0;

// Make sure the peripheral is enabled
	if (!(i2c->CR1 & I2C_CR1_PE)) {
		return false;
	}

// Set slave address, read transfer, and set number of bytes
	i2c->CR2 = (slave_addr << 1) | I2C_CR2_RD_WRN | (len << 16)
			| I2C_CR2_AUTOEND;

// Send START condition
	SET_BIT(i2c->CR2, I2C_CR2_START);

	while (index < len) {
		// Check for timeout
		if (HAL_GetTick() - tick_start > timeout) {
			return false;
		}

		// Check if RXNE flag is set (receive data register not empty)
		if (READ_BIT(i2c->ISR, I2C_ISR_RXNE)) {
			// Read data from RXDR
			data[index++] = (uint8_t) (I2C2->RXDR & 0xFF);
		}
	}

	while (!(READ_BIT(i2c->ISR, I2C_ISR_STOPF))) {
	}
	SET_BIT(i2c->ICR, I2C_ICR_STOPCF);

	return true;
}

bool i2cMasterTransmit(I2C_TypeDef *i2c, uint8_t slave_addr, uint8_t *data,
		uint8_t len, uint32_t timeout) {
	uint32_t tick_start = HAL_GetTick();
	uint8_t index = 0;

// Make sure the peripheral is enabled
	if (!(i2c->CR1 & I2C_CR1_PE)) {
		return false;
	}

// Set slave address, write transfer, and set number of bytes
	i2c->CR2 = (slave_addr << 1) | (len << 16) | I2C_CR2_AUTOEND;

// Send START condition
	SET_BIT(i2c->CR2, I2C_CR2_START);

	while (index < len) {
		// Check for timeout
		if (HAL_GetTick() - tick_start > timeout) {
			return false;
		}

		// Check if TXIS flag is set (transmit data register empty)
		if (READ_BIT(I2C2->ISR, I2C_ISR_TXIS)) {
			// Write data to TXDR
			i2c->TXDR = data[index++];

			// Wait for TC flag (transfer complete)

		}
	}

	while (!READ_BIT(I2C2->ISR, I2C_ISR_STOPF)) {
		if (HAL_GetTick() - tick_start > timeout) {
			return false;
		}
	}

	SET_BIT(i2c->ICR, I2C_ICR_STOPCF);

	return true;
}

