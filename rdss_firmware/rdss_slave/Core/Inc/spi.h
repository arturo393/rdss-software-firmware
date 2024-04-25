/*
 * spi.h
 *
 *  Created on: Apr 17, 2023
 *      Author: artur
 */
#include "main.h"
#ifndef SRC_SPI_H_
#define SRC_SPI_H_

// SPI1 Configuration
#define SPI1_PORT           GPIOA
#define SPI1_SCK_PIN        GPIO_PIN_12
#define SPI1_MISO_PIN       GPIO_PIN_6
#define SPI1_MOSI_PIN       GPIO_PIN_7
#define SPI1_NSS_PIN        GPIO_PIN_4

#define SPI1_AF             GPIO_AF0_SPI1

void spi1_init(void);
void spi1Transmit(uint8_t *data_out, uint8_t *data_in, uint16_t size);

#endif /* SRC_SPI_H_ */
