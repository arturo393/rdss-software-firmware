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

void writeRegister(SPI_HandleTypeDef *spi, uint8_t address, uint8_t *cmd,
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
}

void setLNAGain(SX1278_t *module) {
	writeRegister(module->spi, LR_RegLna, &(module->lnaGain), 1);
}

void setPreambleParameters(SX1278_t *module) {

	writeRegister(module->spi, LR_RegSymbTimeoutLsb, &(module->symbTimeoutLsb),
			1);
	writeRegister(module->spi, LR_RegPreambleMsb, &(module->preambleLengthMsb),
			1);
	writeRegister(module->spi, LR_RegPreambleLsb, &(module->preambleLengthLsb),
			1);
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

void readOperatingMode(SX1278_t *module) {
	module->operatingMode = (0x07 & readRegister(module->spi,
	LR_RegOpMode));
}

void updateLoraLowFreq(SX1278_t *module, SX1278_Status_t mode) {
	uint8_t cmd = LORA_MODE_ACTIVATION | LOW_FREQUENCY_MODE | mode;
	writeRegister(module->spi, LR_RegOpMode, &cmd, 1);
	module->operatingMode = mode;
}

void clearIrqFlags(SX1278_t *module) {
	uint8_t cmd = 0xFF;
	writeRegister(module->spi, LR_RegIrqFlags, &cmd, 1);
}

void writeLoRaParameters(SX1278_t *module) {
	updateLoraLowFreq(module, SLEEP);
	HAL_Delay(15);
	setRFFrequency(module);
	setLORAWAN(module);
	setOutputPower(module);
	setOvercurrentProtect(module);
	writeRegister(module->spi, LR_RegLna, &(module->lnaGain), 1);
	if (module->LoRa_SF == SF_6) {
		module->headerMode = IMPLICIT;
		module->symbTimeoutMsb = 0x03;
		setDetectionParameters(module);
	} else {
		module->headerMode = EXPLICIT;
		module->symbTimeoutMsb = 0x00;
	}
	setReModemConfig(module);
	setPreambleParameters(module);
	writeRegister(module->spi, LR_RegHopPeriod, &(module->fhssValue), 1);
	writeRegister(module->spi, LR_RegDioMapping1, &(module->dioConfig), 1);
	clearIrqFlags(module);
	writeRegister(module->spi, LR_RegIrqFlagsMask, &(module->flagsMode), 1);
}

void updateMode(SX1278_t *module, Lora_Mode_t mode) {
	if(module->mode == mode)
		return;

	if (mode == SLAVE_SENDER || mode == MASTER_SENDER) {
		module->frequency =
				(mode == SLAVE_SENDER) ? UPLINK_FREQ : DOWNLINK_FREQ;
		module->dioConfig = DIO0_TX_DONE | DIO1_RX_TIMEOUT
				| DIO2_FHSS_CHANGE_CHANNEL | DIO3_VALID_HEADER;
		module->flagsMode = 0xff;
		CLEAR_BIT(module->flagsMode, TX_DONE_MASK);
		module->mode = mode;
		module->status = TX_MODE;

	} else if (mode == SLAVE_RECEIVER || mode == MASTER_RECEIVER) {
		module->frequency =
				(mode == SLAVE_RECEIVER) ? DOWNLINK_FREQ : UPLINK_FREQ;

		module->dioConfig = DIO0_RX_DONE | DIO1_RX_TIMEOUT
				| DIO2_FHSS_CHANGE_CHANNEL | DIO3_VALID_HEADER;
		module->flagsMode = 0xff;
		CLEAR_BIT(module->flagsMode, RX_DONE_MASK);
		CLEAR_BIT(module->flagsMode, PAYLOAD_CRC_ERROR_MASK);
		module->mode = mode;
		module->status = RX_MODE;
	}
	updateLoraLowFreq(module, STANDBY);
	HAL_Delay(15);
	setRFFrequency(module);
	writeRegister(module->spi, LR_RegDioMapping1, &(module->dioConfig), 1);
	clearIrqFlags(module);
	writeRegister(module->spi, LR_RegIrqFlagsMask, &(module->flagsMode), 1);
}

void initLoRaParameters(SX1278_t *module, Lora_Mode_t mode) {
	module->power = SX1278_POWER_17DBM;
	module->LoRa_SF = SF_10;
	module->LoRa_BW = LORABW_62_5KHZ;
	module->LoRa_CR = LORA_CR_4_6;
	module->LoRa_CRC_sum = CRC_ENABLE;
	module->syncWord = LORAWAN;
	module->ocp = OVERCURRENTPROTECT;
	module->lnaGain = LNAGAIN;
	module->AgcAutoOn = LNA_SET_BY_AGC;
	module->symbTimeoutLsb = RX_TIMEOUT_LSB;
	module->preambleLengthMsb = PREAMBLE_LENGTH_MSB;
	module->preambleLengthLsb = PREAMBLE_LENGTH_LSB;
	module->fhssValue = HOPS_PERIOD;
	module->len = SX1278_MAX_PACKET;
	updateMode(module, mode);
}

void sx1278Reset() {
	HAL_GPIO_WritePin(LORA_NSS_GPIO_Port, LORA_NSS_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LORA_NSS_GPIO_Port, LORA_NSS_Pin, GPIO_PIN_RESET);
	HAL_Delay(1);
	HAL_GPIO_WritePin(LORA_NSS_GPIO_Port, LORA_NSS_Pin, GPIO_PIN_SET);
	HAL_Delay(100);
}

void waitForTxEnd(SX1278_t *loRa) {
	int timeStart = HAL_GetTick();
	while (1) {
		if (HAL_GPIO_ReadPin(LORA_BUSSY_GPIO_Port, LORA_BUSSY_Pin)) {
			int timeEnd = HAL_GetTick();
			loRa->lastTxTime = timeEnd - timeStart;
			readRegister(loRa->spi, LR_RegIrqFlags);
			clearIrqFlags(loRa);
			loRa->status = TX_DONE;
			return;
		}
		if (HAL_GetTick() - timeStart > LORA_SEND_TIMEOUT) {
			sx1278Reset();
			loRa->status = TX_TIMEOUT;
			return;
		}
		HAL_Delay(1);
	}
}

uint8_t waitForRxDone(SX1278_t *loRa) {
	uint32_t timeout = HAL_GetTick();
	while ((!HAL_GPIO_ReadPin(LORA_BUSSY_GPIO_Port, LORA_BUSSY_Pin))) {
		uint8_t flags = readRegister(loRa->spi, LR_RegIrqFlags);
		if (READ_BIT(flags, PAYLOAD_CRC_ERROR_MASK)) {
			uint8_t cmd = flags | (1 << 7);
			writeRegister(loRa->spi, LR_RegIrqFlags, &cmd, 1);
			flags = readRegister(loRa->spi, LR_RegIrqFlags);
		}
		if (HAL_GetTick() - timeout > 2000)
			return -1;
	}
	return 0;
}

void setRxFifoAddr(SX1278_t *module) {
	updateLoraLowFreq(module, SLEEP); //Change modem mode Must in Sleep mode
	uint8_t cmd = module->len;
	writeRegister(module->spi, LR_RegPayloadLength, &(cmd), 1); //RegPayloadLength 21byte
	uint8_t addr = readRegister(module->spi, LR_RegFifoRxBaseAddr); //RegFiFoTxBaseAddr
	writeRegister(module->spi, LR_RegFifoAddrPtr, &addr, 1); //RegFifoAddrPtr
	module->len = readRegister(module->spi, LR_RegPayloadLength);
}

int crcErrorActivation(SX1278_t *module) {
	uint8_t flags2 = readRegister(module->spi, LR_RegIrqFlags);
	SET_BIT(flags2, RX_DONE_MASK);
	uint8_t cmd = flags2;
	writeRegister(module->spi, LR_RegIrqFlags, &cmd, 1);
	uint8_t flags = readRegister(module->spi, LR_RegIrqFlags);
	int errorActivation = READ_BIT(flags, PAYLOAD_CRC_ERROR_MASK);
	return errorActivation;
}

void getRxFifoData(SX1278_t *module) {
	module->len = readRegister(module->spi, LR_RegRxNbBytes); //Number for received bytes
	uint8_t addr = 0x00;
	HAL_GPIO_WritePin(GPIOB, LORA_NSS_Pin, GPIO_PIN_RESET); // pull the pin low
	HAL_Delay(1);
	HAL_SPI_Transmit(module->spi, &addr, 1, 100); // send address
	HAL_SPI_Receive(module->spi, module->buffer, module->len, 100); // receive 6 bytes data
	HAL_Delay(1);
	HAL_GPIO_WritePin(GPIOB, LORA_NSS_Pin, GPIO_PIN_SET); // pull the pin high
	module->status = RX_DONE;
}

void setTxFifoAddr(SX1278_t *module) {
	uint8_t cmd = module->len;
	writeRegister(module->spi, LR_RegPayloadLength, &(cmd), 1);
	uint8_t addr = readRegister(module->spi, LR_RegFifoTxBaseAddr);
	addr = 0x80;
	writeRegister(module->spi, LR_RegFifoAddrPtr, &addr, 1);
	module->len = readRegister(module->spi, LR_RegPayloadLength);
}

void setTxFifoData(SX1278_t *module) {
	setTxFifoAddr(module);
	for (int i = 0; i < module->len; i++) {
		uint8_t data = module->buffer[i];
		writeRegister(module->spi, 0x00, &data, 1);
	}
}

void clearMemForRx(SX1278_t *module) {
	if (module->status == RX_MODE) {
		memset(module->buffer, 0, SX1278_MAX_PACKET);
	}
}

void receive(SX1278_t *loRa) {
	setRxFifoAddr(loRa);
	updateLoraLowFreq(loRa, RX_CONTINUOUS);
	clearMemForRx(loRa);
	waitForRxDone(loRa);
	getRxFifoData(loRa);
}

void transmit(SX1278_t *loRa) {
	setTxFifoData(loRa);
	updateLoraLowFreq(loRa, TX);
	waitForTxEnd(loRa);
	memset(loRa->buffer, 0, sizeof(loRa->buffer));
	loRa->len = 0;
}
