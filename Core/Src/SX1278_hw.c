/**
 * Author Wojciech Domski <Wojciech.Domski@gmail.com>
 * www: www.Domski.pl
 *
 * Hardware layer for SX1278 LoRa module
 */

#include "SX1278_hw.h"



 void SX1278_hw_SetNSS(SX1278_hw_t *hw, int value) {
	HAL_GPIO_WritePin(hw->nss.port, hw->nss.pin,
			(value == 1) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

 void SX1278_hw_Reset(SX1278_hw_t *hw) {
	SX1278_hw_SetNSS(hw, 1);
	HAL_GPIO_WritePin(hw->reset.port, hw->reset.pin, GPIO_PIN_RESET);

	SX1278_hw_DelayMs(1);

	HAL_GPIO_WritePin(hw->reset.port, hw->reset.pin, GPIO_PIN_SET);

	SX1278_hw_DelayMs(100);
}

 void SX1278_hw_SPICommand(SX1278_hw_t *hw, uint8_t cmd,SPI_HandleTypeDef *spi) {
	HAL_SPI_Transmit(spi, &cmd, 1, 1000);
	while (HAL_SPI_GetState(spi) != HAL_SPI_STATE_READY);
}

 uint8_t SX1278_hw_SPIReadByte(SX1278_hw_t *hw, SPI_HandleTypeDef *spi) {
	uint8_t txByte = 0x00;
	uint8_t rxByte = 0x00;

	SX1278_hw_SetNSS(hw, 0);
//	HAL_SPI_Transmit (hw->spi, &address, 1, 100);  // send address
//	HAL_SPI_Receive (spi, &rxByte, 1, 100);  // receive 1 bytes data

	HAL_SPI_TransmitReceive(spi, &txByte, &rxByte, 1, 1000);
	while (HAL_SPI_GetState(spi) != HAL_SPI_STATE_READY)
		;
	return rxByte;
}

 void SX1278_hw_DelayMs(uint32_t msec) {
	HAL_Delay(msec);
}

 int SX1278_hw_GetDIO0(SX1278_hw_t *hw) {
	return (HAL_GPIO_ReadPin(hw->dio0.port, hw->dio0.pin) == GPIO_PIN_SET);
}

