/*
 * spi.c
 *
 *  Created on: Apr 17, 2023
 *      Author: artur
 */

#include "spi.h"

void spi1_init(void) {
	// Enable GPIOA and GPIOB clock
	RCC->IOPENR |= RCC_IOPENR_GPIOAEN | RCC_IOPENR_GPIOBEN;

	// Configure PB3 as AF0 (SCK)
	GPIOB->MODER &= ~GPIO_MODER_MODE3_Msk;
	GPIOB->MODER |= GPIO_MODER_MODE3_1;
	GPIOB->AFR[0] &= ~(GPIO_AFRL_AFSEL3_Msk);
	GPIOB->AFR[0] |= (0b0001 << GPIO_AFRL_AFSEL3_Pos);

	// Configure PA12 as AF0 (MOSI)
	GPIOA->MODER &= ~GPIO_MODER_MODE12_Msk;
	GPIOA->MODER |= GPIO_MODER_MODE12_1;
	// Select alternate function 0 (AF0) for PA11
	GPIOA->AFR[1] &= ~GPIO_AFRH_AFSEL12_Msk;
	GPIOA->AFR[1] |= 0x0 << GPIO_AFRH_AFSEL12_Pos;

	// Configure PA11 output type as push-pull
	GPIOA->OTYPER &= ~GPIO_OTYPER_OT12_Msk;

	// Configure PA11 output speed as high speed
	GPIOA->OSPEEDR |= GPIO_OSPEEDR_OSPEED12_Msk;

	// Configure PA11 pull-up resistor as enabled
	GPIOA->PUPDR &= ~GPIO_PUPDR_PUPD12_Msk;
	GPIOA->PUPDR |= GPIO_PUPDR_PUPD12_0;

	// Configure PA11 as AF0 (MISO)
	GPIOA->MODER &= ~GPIO_MODER_MODE11_Msk;
	GPIOA->MODER |= GPIO_MODER_MODE11_1;
	// Select alternate function 0 (AF0) for PA11
	GPIOA->AFR[1] &= ~GPIO_AFRH_AFSEL11_Msk;
	GPIOA->AFR[1] |= 0x0 << GPIO_AFRH_AFSEL11_Pos;

	// Configure PA11 output type as push-pull
	GPIOA->OTYPER &= ~GPIO_OTYPER_OT11_Msk;

	// Configure PA11 output speed as high speed
	GPIOA->OSPEEDR |= GPIO_OSPEEDR_OSPEED11_Msk;

	// Configure PA11 pull-up resistor as enabled
	GPIOA->PUPDR &= ~GPIO_PUPDR_PUPD11_Msk;
	GPIOA->PUPDR |= GPIO_PUPDR_PUPD11_0;

	// Enable SPI1 clock

	RCC->APBENR2 |= RCC_APBENR2_SPI1EN;

	// Configure SPI1
	SPI1->CR1 |= SPI_CR1_BR_0 | SPI_CR1_BR_1; // Set baud rate to fPCLK/8
	SPI1->CR1 |= SPI_CR1_MSTR; // Set as master
	SPI1->CR1 |= SPI_CR1_SSM | SPI_CR1_SSI; // Software slave management
	SPI1->CR1 &= ~SPI_CR1_LSBFIRST; // MSB first
	SPI1->CR2 |= SPI_CR2_FRXTH; // Set RXNE flag when receive FIFO is half full
	SPI1->CR2 |= SPI_CR2_DS_0 | SPI_CR2_DS_1 | SPI_CR2_DS_2; // 8-bit data size
	SPI1->CR1 |= SPI_CR1_SPE; // Enable SPI1
}

