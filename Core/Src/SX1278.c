/**
 * Author Wojciech Domski <Wojciech.Domski@gmail.com>
 * www: www.Domski.pl
 *
 * work based on DORJI.COM sample code and
 * https://github.com/realspinner/SX1278_LoRa
 */

#include "SX1278.h"
#include <string.h>

uint8_t readRegister(SPI_HandleTypeDef *spi, uint8_t address) {
	uint8_t rec;
	HAL_GPIO_WritePin(GPIOB, LORA_NSS_Pin, GPIO_PIN_RESET);  // pull the pin low
	HAL_Delay(1);
	HAL_SPI_Transmit(spi, &address, 1, 100);  // send address
	HAL_SPI_Receive(spi, &rec, 1, 100);  // receive 6 bytes data
	HAL_Delay(1);
	HAL_GPIO_WritePin(GPIOB, LORA_NSS_Pin, GPIO_PIN_SET);  // pull the pin high
	return rec;
}

uint8_t writeRegister(SPI_HandleTypeDef *spi, uint8_t address, uint8_t *cmd,
		uint8_t lenght) {
	uint8_t tx_data[30] = { 0 };
	tx_data[0] = address | 0x80;
	int j = 0;
	for (int i = 1; i <= lenght; i++) {
		tx_data[i] = cmd[j++];
	}
	HAL_GPIO_WritePin(GPIOB, LORA_NSS_Pin, GPIO_PIN_RESET);  // pull the pin low
	HAL_SPI_Transmit(spi, tx_data, lenght + 1, 1000);
	HAL_GPIO_WritePin(GPIOB, LORA_NSS_Pin, GPIO_PIN_SET);  // pull the pin high
	HAL_Delay(10);
	return cmd;  // pull the pin high
}

uint8_t writeRegister2(SPI_HandleTypeDef *spi, uint8_t address, uint8_t *cmd,
		uint8_t lenght) {
	HAL_GPIO_WritePin(GPIOB, LORA_NSS_Pin, GPIO_PIN_RESET);  // pull the pin low
	address = address | 0x80;
	HAL_SPI_Transmit(spi, &address, 1, 1000);
	while (HAL_SPI_GetState(spi) != HAL_SPI_STATE_READY)
		;
	HAL_SPI_Transmit(spi, cmd, lenght, 1000);
	while (HAL_SPI_GetState(spi) != HAL_SPI_STATE_READY)
		;
	HAL_GPIO_WritePin(GPIOB, LORA_NSS_Pin, GPIO_PIN_SET);
	return cmd;  // pull the pin high
}

void setRFFrequency(SX1278_t *module) {
	uint64_t freq = ((uint64_t) module->frequency << 19) / FXOSC;
	uint8_t freq_reg[3];
	freq_reg[0] = (uint8_t) (freq >> 16);
	freq_reg[1] = (uint8_t) (freq >> 8);
	freq_reg[2] = (uint8_t) (freq >> 0);
	writeRegister(module->spi, LR_RegFrMsb, freq_reg, sizeof(freq_reg));
}

void setOutputPower(SX1278_t *module) {
	writeRegister(module->spi, LR_RegPaConfig, &(module->power), 1);
}

void setLORAWAN(SX1278_t *module) {
	writeRegister(module->spi, RegSyncWord, &(module->syncWord), 1);
}
void setOvercurrentProtect(SX1278_t *module) {
	writeRegister(module->spi, LR_RegOcp, &(module->ocp), 1);
	//SX1278_SPIWrite(module, LR_RegOcp, 0x0B, spi);
}
void setLNAGain(SX1278_t *module) {
	writeRegister(module->spi, LR_RegLna, &(module->lnaGain), 1);
	//SX1278_SPIWrite(module, LR_RegLna, 0x23, spi);//RegLNA,High & LNA Enable
}
void setPreambleParameters(SX1278_t *module) {

	writeRegister(module->spi, LR_RegSymbTimeoutLsb, &(module->symbTimeoutLsb),
			1);
	writeRegister(module->spi, LR_RegPreambleMsb, &(module->preambleLengthMsb),
			1);
	writeRegister(module->spi, LR_RegPreambleLsb, &(module->preambleLengthLsb),
			1);
	module->readBytes = 0;
}

void setReModemConfig(SX1278_t *module) {

	uint8_t cmd = 0;
	cmd = module->LoRa_BW << 4;
	cmd += module->LoRa_CR << 1;
	cmd += module->headerMode;
	writeRegister(module->spi, LR_RegModemConfig1, &cmd, 1); //Explicit Enable CRC Enable(0x02) & Error Coding rate 4/5(0x01), 4/6(0x02), 4/7(0x03), 4/8(0x04)

	cmd = module->LoRa_SF << 4;
	cmd += module->LoRa_CRC_sum << 2;
	cmd += module->symbTimeoutMsb;
	writeRegister(module->spi, LR_RegModemConfig2, &cmd, 1);
	writeRegister(module->spi, LR_RegModemConfig3, &(module->AgcAutoOn), 1);
}
void setDetectionParameters(SX1278_t *module) {
	uint8_t tmp;
	tmp = readRegister(module->spi, LR_RegDetectOptimize);
	tmp &= 0xF8;
	tmp |= 0x05;
	writeRegister(module->spi, LR_RegDetectOptimize, &tmp, 1);
	tmp = 0x0C;
	writeRegister(module->spi, LR_RegDetectionThreshold, &tmp, 1);
}

void standbyMode(SX1278_t *module) {
	uint8_t cmd = STANDBY;
	writeRegister(module->spi, LR_RegOpMode, &cmd, 1);
	module->operatingMode = STANDBY;
}

void sleepMode(SX1278_t *module) {
	uint8_t cmd = SLEEP;
	writeRegister(module->spi, LR_RegOpMode, &cmd, 1);
	module->operatingMode = SLEEP;
}

/*void setLoraLowFreqMode(SX1278_t *module) {
	uint8_t cmd = LORA_LOWFREQ;
	writeRegister(module->spi, LR_RegOpMode, &cmd, 1);
	module->status = LORA_LOWFREQ;
}*/

void setMode(SX1278_t *module, SX1278_Status_t mode) {
	uint8_t cmd = mode;
	writeRegister(module->spi, LR_RegOpMode, &cmd, 1);
	module->operatingMode = mode;
}
SX1278_Status_t readMode(SX1278_t *module) {
	return (0x07 & readRegister(module->spi,
	LR_RegOpMode));
}

void updateLoraLowFreq(SX1278_t *module, SX1278_Status_t mode) {
	uint8_t cmd = LORA_MODE_ACTIVATION | LOW_FREQUENCY_MODE | mode;
	writeRegister(module->spi, LR_RegOpMode, &cmd, 1);
	module->operatingMode = mode;
}


uint8_t SX1278_SPIRead(SX1278_t *module, uint8_t addr, SPI_HandleTypeDef *spi) {
	uint8_t tmp;

	SX1278_hw_SetNSS(module->hw, 0);
	SX1278_hw_SPICommand(module->hw, addr, spi);
	tmp = SX1278_hw_SPIReadByte(module->hw, spi);
	SX1278_hw_SetNSS(module->hw, 1);

	return tmp;
}

void SX1278_SPIWrite(SX1278_t *module, uint8_t addr, uint8_t cmd,
		SPI_HandleTypeDef *spi) {
	SX1278_hw_SetNSS(module->hw, 0);
	SX1278_hw_SPICommand(module->hw, addr | 0x80, spi);
	SX1278_hw_SPICommand(module->hw, cmd, spi);
	SX1278_hw_SetNSS(module->hw, 1);
}

void SX1278_SPIBurstRead(SX1278_t *module, uint8_t addr, uint8_t *rxBuf,
		uint8_t length, SPI_HandleTypeDef *spi) {
	uint8_t i;
	if (length <= 1) {
		return;
	} else {
		SX1278_hw_SetNSS(module->hw, 0);
		SX1278_hw_SPICommand(module->hw, addr, spi);
		for (i = 0; i < length; i++) {
			*(rxBuf + i) = SX1278_hw_SPIReadByte(module->hw, spi);
		}
		SX1278_hw_SetNSS(module->hw, 1);
	}
}

void SX1278_SPIBurstWrite(SX1278_t *module, uint8_t addr, uint8_t *txBuf,
		uint8_t length, SPI_HandleTypeDef *spi) {
	unsigned char i;
	if (length <= 1) {
		return;
	} else {
		SX1278_hw_SetNSS(module->hw, 0);
		SX1278_hw_SPICommand(module->hw, addr | 0x80, spi);
		for (i = 0; i < length; i++) {
			uint8_t cmd = txBuf[i];
			SX1278_hw_SPICommand(module->hw, cmd, spi);
		}
		SX1278_hw_SetNSS(module->hw, 1);
	}
}

/*void configLoraMode(SX1278_t *module, SPI_HandleTypeDef *spi) {
 sleepMode(module, spi); //Change modem mode Must in Sleep mode
 SX1278_hw_DelayMs(15);
 setLoraLowFreqMode(module, spi);
 //SX1278_SPIWrite(module, 0x5904); //?? Change digital regulator form 1.6V to 1.47V: see errata note

 uint64_t freq = ((uint64_t) module->frequency << 19) / 32000000;
 uint8_t freq_reg[3];
 freq_reg[0] = (uint8_t) (freq >> 16);
 freq_reg[1] = (uint8_t) (freq >> 8);
 freq_reg[2] = (uint8_t) (freq >> 0);
 SX1278_SPIBurstWrite(module, LR_RegFrMsb, (uint8_t*) freq_reg, 3, spi); //setting  frequency parameter
 SX1278_SPIWrite(module, RegSyncWord, 0x34, spi);

 //setting base parameter

 //SX1278_SPIWrite(module, LR_RegPaConfig, SX1278_Power[module->power], spi); //Setting output power parameter

 SX1278_SPIWrite(module, LR_RegOcp, 0x0B, spi);		//RegOcp,Close Ocp
 SX1278_SPIWrite(module, LR_RegLna, 0x23, spi);	//RegLNA,High & LNA Enable
 if (SX1278_SpreadFactor[module->LoRa_SF] == 6) {	//SFactor=6
 uint8_t tmp;
 SX1278_SPIWrite(module,
 LR_RegModemConfig1,
 ((SX1278_LoRaBandwidth[module->LoRa_BW] << 4)
 + (SX1278_CodingRate[module->LoRa_CR] << 1) + 0x01),
 spi); //Implicit Enable CRC Enable(0x02) & Error Coding rate 4/5(0x01), 4/6(0x02), 4/7(0x03), 4/8(0x04)

 SX1278_SPIWrite(module,
 LR_RegModemConfig2,
 ((SX1278_SpreadFactor[module->LoRa_SF] << 4)
 + (SX1278_CRC_Sum[module->LoRa_CRC_sum] << 2) + 0x03),
 spi);

 tmp = SX1278_SPIRead(module, 0x31, spi);
 tmp &= 0xF8;
 tmp |= 0x05;
 SX1278_SPIWrite(module, 0x31, tmp, spi);
 SX1278_SPIWrite(module, 0x37, 0x0C, spi);
 } else {
 SX1278_SPIWrite(module,
 LR_RegModemConfig1,
 ((SX1278_LoRaBandwidth[module->LoRa_BW] << 4)
 + (SX1278_CodingRate[module->LoRa_CR] << 1) + 0x00),
 spi); //Explicit Enable CRC Enable(0x02) & Error Coding rate 4/5(0x01), 4/6(0x02), 4/7(0x03), 4/8(0x04)

 SX1278_SPIWrite(module,
 LR_RegModemConfig2,
 ((SX1278_SpreadFactor[module->LoRa_SF] << 4)
 + (SX1278_CRC_Sum[module->LoRa_CRC_sum] << 2) + 0x00),
 spi); //SFactor &  LNA gain set by the internal AGC loop
 }

 SX1278_SPIWrite(module, LR_RegModemConfig3, 0x04, spi);
 SX1278_SPIWrite(module, LR_RegSymbTimeoutLsb, 0x08, spi); //RegSymbTimeoutLsb Timeout = 0x3FF(Max)
 SX1278_SPIWrite(module, LR_RegPreambleMsb, 0x00, spi); //RegPreambleMsb
 SX1278_SPIWrite(module, LR_RegPreambleLsb, 8, spi); //RegPreambleLsb 8+4=12byte Preamble
 SX1278_SPIWrite(module, REG_LR_DIOMAPPING2, 0x01, spi); //RegDioMapping2 DIO5=00, DIO4=01
 module->readBytes = 0;

 }*/

void clearIrqFlags(SX1278_t *module) {
	uint8_t cmd = 0xFF;
	writeRegister(module->spi, LR_RegIrqFlags, &cmd, 1);
}

/*int SX1278_LoRaEntryRx(SX1278_t *module, uint8_t length, uint32_t timeout,
		SPI_HandleTypeDef *spi) {
	uint8_t addr;

	module->packetLength = length;

	configLoraMode(module, spi);		//Setting base parameter
	SX1278_SPIWrite(module, LR_RegPaDac, 0x84, spi);	//Normal and RX
	SX1278_SPIWrite(module, LR_RegHopPeriod, 0xFF, spi);	//No FHSS
	SX1278_SPIWrite(module, LR_RegDioMapping1, 0x01, spi);//DIO=00,DIO1=00,DIO2=00, DIO3=01
	SX1278_SPIWrite(module, LR_RegIrqFlagsMask, 0x3F, spi);	//Open RxDone interrupt & Timeout
	clearIrqFlags(module);
	SX1278_SPIWrite(module, LR_RegPayloadLength, length, spi);//Payload Length 21byte(this register must difine when the data long of one byte in SF is 6)
	addr = SX1278_SPIRead(module, LR_RegFifoRxBaseAddr, spi); //Read RxBaseAddr
	SX1278_SPIWrite(module, LR_RegFifoAddrPtr, addr, spi); //RxBaseAddr->FiFoAddrPtr
	SX1278_SPIWrite(module, LR_RegOpMode, 0x8d, spi); //Mode//Low Frequency Mode
	//SX1278_SPIWrite(module, LR_RegOpMode,0x05);	//Continuous Rx Mode //High Frequency Mode
	module->readBytes = 0;

	while (1) {
		if ((SX1278_SPIRead(module, LR_RegModemStat, spi) & RX) == RX) {//Rx-on going RegModemStat
			module->status = RX;
			return 1;
		}
		if (--timeout == 0) {
			SX1278_hw_Reset(module->hw);
			configLoraMode(module, spi);
			return 0;
		}
		SX1278_hw_DelayMs(1);
	}
}*/

uint8_t SX1278_LoRaRxPacket(SX1278_t *module, SPI_HandleTypeDef *spi) {
	unsigned char addr;
	unsigned char packet_size;

	if (SX1278_hw_GetDIO0(module->hw)) {
		memset(module->buffer, 0x00, SX1278_MAX_PACKET);

		addr = SX1278_SPIRead(module, LR_RegFifoRxCurrentaddr, spi); //last packet addr
		SX1278_SPIWrite(module, LR_RegFifoAddrPtr, addr, spi); //RxBaseAddr -> FiFoAddrPtr

		if (module->LoRa_SF == SF_6) { //When SpreadFactor is six,will used Implicit Header mode(Excluding internal packet length)
			packet_size = module->packetLength;
		} else {
			packet_size = SX1278_SPIRead(module, LR_RegRxNbBytes, spi); //Number for received bytes
		}

		SX1278_SPIBurstRead(module, 0x00, module->buffer, packet_size, spi);
		module->readBytes = packet_size;
		clearIrqFlags(module);
	}
	return module->readBytes;
}

void SX1278_clearLoRaIrq(SX1278_t *module, SPI_HandleTypeDef *spi) {
	SX1278_SPIWrite(module, LR_RegIrqFlags, 0xFF, spi);
}

int configTxRegister(SX1278_t *module, SPI_HandleTypeDef *spi) {
	uint8_t addr;

	SX1278_SPIWrite(module, REG_LR_PADAC, 0x87, spi);	//Tx for 20dBm
	SX1278_SPIWrite(module, LR_RegHopPeriod, 0x00, spi); //RegHopPeriod NO FHSS
	SX1278_SPIWrite(module, LR_RegDioMapping1, 0x41, spi); //DIO0=01, DIO1=00,DIO2=00, DIO3=01
	SX1278_clearLoRaIrq(module, spi);
	SX1278_SPIWrite(module, LR_RegIrqFlagsMask, 0xF7, spi); //Open TxDone interrupt
	SX1278_SPIWrite(module, LR_RegPayloadLength, module->packetLength, spi); //RegPayloadLength 21byte
	addr = SX1278_SPIRead(module, LR_RegFifoTxBaseAddr, spi); //RegFiFoTxBaseAddr
	SX1278_SPIWrite(module, LR_RegFifoAddrPtr, addr, spi); //RegFifoAddrPtr

	uint32_t timeStart = HAL_GetTick();
	module->packetLength = SX1278_SPIRead(module, LR_RegPayloadLength, spi);
	return module->packetLength;

}

/*SX1278_Status_t SX1278_LoRaTxPacket(SX1278_t *module, SPI_HandleTypeDef *spi) {
	SX1278_SPIBurstWrite(module, 0x00, module->txBuffer, module->packetLength,
			spi);
	SX1278_SPIWrite(module, LR_RegOpMode, (LORA_LOWFREQ | TX), spi); //Tx Mode
	uint32_t timeStart = HAL_GetTick();
	while (1) {
		if (SX1278_hw_GetDIO0(module->hw)) { //if(Get_NIRQ()) //Packet send over
			SX1278_SPIRead(module, LR_RegIrqFlags, spi);
			clearIrqFlags(module); //Clear irq
//			standbyMode(module); //Entry Standby mode
			SX1278_SPIWrite(module, LR_RegOpMode, STANDBY, spi);
			module->status = STANDBY;
			return 1;
		}

		if (HAL_GetTick() - timeStart > LORA_SEND_TIMEOUT)
			return 0;

		SX1278_hw_DelayMs(1);
	}
}*/

int SX1278_transmit(SX1278_t *module, uint8_t *txBuf, uint8_t length,
		uint32_t timeout, SPI_HandleTypeDef *spi) {
	if (configTxRegister(module, spi)) {
		return SX1278_LoRaTxPacket(module, spi);
	}
	return 0;
}

int SX1278_receive(SX1278_t *module, uint8_t length, uint32_t timeout,
		SPI_HandleTypeDef *spi) {
	return SX1278_LoRaEntryRx(module, length, timeout, spi);
}

uint8_t SX1278_available(SX1278_t *module, SPI_HandleTypeDef *spi) {
	return SX1278_LoRaRxPacket(module, spi);
}

uint8_t SX1278_RSSI_LoRa(SX1278_t *module, SPI_HandleTypeDef *spi) {
	uint32_t temp = 10;
	temp = SX1278_SPIRead(module, LR_RegRssiValue, spi); //Read RegRssiValue, Rssi value
	temp = temp + 127 - 137; //127:Max RSSI, 137:RSSI offset
	return (uint8_t) temp;
}

uint8_t SX1278_RSSI(SX1278_t *module, SPI_HandleTypeDef *spi) {
	uint8_t temp = 0xff;
	temp = SX1278_SPIRead(module, RegRssiValue, spi);
	temp = 127 - (temp >> 1);	//127:Max RSSI
	return temp;
}
