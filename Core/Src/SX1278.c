/**
 * Author Wojciech Domski <Wojciech.Domski@gmail.com>
 * www: www.Domski.pl
 *
 * work based on DORJI.COM sample code and
 * https://github.com/realspinner/SX1278_LoRa
 */

#include <SX1278.h>

uint8_t readRegister(SPI_HandleTypeDef *spi, uint8_t address) {
	uint8_t rec = 0;
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
	if (lenght > 4)
		return;
	uint8_t tx_data[5] = { 0 };
	tx_data[0] = address | 0x80;
	int j = 0;
	for (int i = 1; i <= lenght; i++) {
		tx_data[i] = cmd[j++];
	}
	HAL_GPIO_WritePin(GPIOB, LORA_NSS_Pin, GPIO_PIN_RESET);  // pull the pin low
	HAL_SPI_Transmit(spi, tx_data, lenght + 1, 1000);
	HAL_GPIO_WritePin(GPIOB, LORA_NSS_Pin, GPIO_PIN_SET);  // pull the pin high
//	HAL_Delay(10);
}

// Function to write to register via SPI
void writeRegister2(SPI_HandleTypeDef *spi, uint8_t address, uint8_t *data,
		uint8_t length) {

	if (length > 4)
		return;
	uint8_t tx_data[5] = { 0 };
	uint16_t timeout = 1000;
	tx_data[0] = address | 0x80; // set MSB high for write operation
	for (int i = 0; i < length; i++) {
		tx_data[i + 1] = data[i];
	}
	HAL_GPIO_WritePin(GPIOB, LORA_NSS_Pin, GPIO_PIN_RESET);  // pull the pin low
	for (volatile int i = 0; i < 10; i++) {
	} // delay for stabilization
	for (int i = 0; i < length + 1; i++) {
		spi->Instance->DR = tx_data[i]; // send data
		while ((spi->Instance->SR & SPI_SR_TXE) == 0) { // wait for transmit buffer empty
			if (--timeout == 0) { // check timeout
				HAL_GPIO_WritePin(GPIOB, LORA_NSS_Pin, GPIO_PIN_SET); // pull the pin high
				return; // return error
			}
		}
		while ((spi->Instance->SR & SPI_SR_RXNE) == 0) { // wait for receive buffer not empty
			if (--timeout == 0) { // check timeout
				HAL_GPIO_WritePin(GPIOB, LORA_NSS_Pin, GPIO_PIN_SET); // pull the pin high
				return; // return error
			}
		}
		volatile uint8_t dummy_read = spi->Instance->DR; // read dummy data
	}
	HAL_GPIO_WritePin(GPIOB, LORA_NSS_Pin, GPIO_PIN_SET);  // pull the pin high
}

// Function to read register via SPI
uint8_t readRegister2(SPI_HandleTypeDef *spi, uint8_t address) {
	uint8_t rec;
	uint16_t timeout = 1000;
	HAL_GPIO_WritePin(GPIOB, LORA_NSS_Pin, GPIO_PIN_RESET);  // pull the pin low
	for (volatile int i = 0; i < 10; i++) {
	} // delay for stabilization
	spi->Instance->DR = address; // send address
	while ((spi->Instance->SR & SPI_SR_RXNE) == 0) { // wait for receive buffer not empty
		if (--timeout == 0) { // check timeout
			HAL_GPIO_WritePin(GPIOB, LORA_NSS_Pin, GPIO_PIN_SET); // pull the pin high
			return 0; // return error
		}
	}
	rec = spi->Instance->DR; // read data
	HAL_GPIO_WritePin(GPIOB, LORA_NSS_Pin, GPIO_PIN_SET);  // pull the pin high
	return rec;
}

void setRFFrequencyReg(SX1278_t *module) {
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
	cmd = module->bandwidth << 4;
	cmd += module->codingRate << 1;
	cmd += module->headerMode;
	writeRegister(module->spi, LR_RegModemConfig1, &cmd, 1); //Explicit Enable CRC Enable(0x02) & Error Coding rate 4/5(0x01), 4/6(0x02), 4/7(0x03), 4/8(0x04)

	cmd = module->spreadFactor << 4;
	cmd += module->LoRa_CRC_sum << 2;
	cmd += module->symbTimeoutMsb;
	writeRegister(module->spi, LR_RegModemConfig2, &cmd, 1);
	writeRegister(module->spi, LR_RegModemConfig3, &(module->AgcAutoOn), 1);
}

void setDetectionParametersReg(SX1278_t *module) {
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

void setLoRaLowFreqModeReg(SX1278_t *module, OPERATING_MODE_t mode) {
	uint8_t cmd = LORA_MODE_ACTIVATION | LOW_FREQUENCY_MODE | mode;
	writeRegister(module->spi, LR_RegOpMode, &cmd, 1);
	module->operatingMode = mode;
}

void clearIrqFlagsReg(SX1278_t *module) {
	uint8_t cmd = 0xFF;
	writeRegister(module->spi, LR_RegIrqFlags, &cmd, 1);
}

void writeLoRaParametersReg(SX1278_t *module) {
	setLoRaLowFreqModeReg(module, SLEEP);
	HAL_Delay(15);
	setRFFrequencyReg(module);
	writeRegister(module->spi, RegSyncWord, &(module->syncWord), 1);
	setOutputPower(module);
	setOvercurrentProtect(module);
	writeRegister(module->spi, LR_RegLna, &(module->lnaGain), 1);
	if (module->spreadFactor == SF_6) {
		module->headerMode = IMPLICIT;
		module->symbTimeoutMsb = 0x03;
		setDetectionParametersReg(module);
	} else {
		module->headerMode = EXPLICIT;
		module->symbTimeoutMsb = 0x00;
	}

	setReModemConfig(module);
	setPreambleParameters(module);
	writeRegister(module->spi, LR_RegHopPeriod, &(module->fhssValue), 1);
	writeRegister(module->spi, LR_RegDioMapping1, &(module->dioConfig), 1);
	clearIrqFlagsReg(module);
	writeRegister(module->spi, LR_RegIrqFlagsMask, &(module->flagsMode), 1);
}

void changeMode(SX1278_t *module, Lora_Mode_t mode) {
	uint32_t timeStart = HAL_GetTick();
	if (mode == SLAVE_SENDER || mode == MASTER_SENDER) {
		module->frequency =
				(mode == SLAVE_SENDER) ? module->upFreq : module->dlFreq;
		module->dioConfig = DIO0_TX_DONE | DIO1_RX_TIMEOUT
				| DIO2_FHSS_CHANGE_CHANNEL | DIO3_VALID_HEADER;
		module->flagsMode = 0xff;
		CLEAR_BIT(module->flagsMode, TX_DONE_MASK);
		module->mode = mode;
		module->status = TX_MODE;

	} else if (mode == SLAVE_RECEIVER || mode == MASTER_RECEIVER) {
		module->frequency =
				(mode == SLAVE_RECEIVER) ? module->dlFreq : module->upFreq;

		module->dioConfig = DIO0_RX_DONE | DIO1_RX_TIMEOUT
				| DIO2_FHSS_CHANGE_CHANNEL | DIO3_VALID_HEADER;
		module->flagsMode = 0xff;
		module->flagsMode &= ~(RX_DONE_MASK) & ~(PAYLOAD_CRC_ERROR_MASK);
		module->mode = mode;
		module->status = RX_MODE;
	}

	setLoRaLowFreqModeReg(module, STANDBY);
	//HAL_Delay(1);
	setRFFrequencyReg(module);
	writeRegister(module->spi, LR_RegDioMapping1, &(module->dioConfig), 1);
	clearIrqFlagsReg(module);
	writeRegister(module->spi, LR_RegIrqFlagsMask, &(module->flagsMode), 1);
}

void initLoRaParameters(SX1278_t *module) {
	module->power = SX1278_POWER_17DBM;
	module->LoRa_CRC_sum = CRC_ENABLE;
	module->ocp = OVERCURRENTPROTECT;
	module->lnaGain = LNAGAIN;
	module->AgcAutoOn = 12; // for L-TEL PROTOCOL
	module->syncWord = 0x12; // for L-TEL PROTOCOL
	module->symbTimeoutLsb = RX_TIMEOUT_LSB;
	module->preambleLengthMsb = PREAMBLE_LENGTH_MSB;
	module->preambleLengthLsb = PREAMBLE_LENGTH_LSB;
	module->preambleLengthLsb = 12; // for L-TEL PROTOCOL
	module->fhssValue = HOPS_PERIOD; // for L-TEL PROTOCOL
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

			readRegister(loRa->spi, LR_RegIrqFlags);
			clearIrqFlagsReg(loRa);
			loRa->status = TX_DONE;
			loRa->lastTxTime = HAL_GetTick() - timeStart;
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

uint8_t waitForRxDone2(SX1278_t *loRa) {
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

uint8_t waitForRxDone(SX1278_t *loRa) {
	uint32_t timeout = HAL_GetTick();
	while (!HAL_GPIO_ReadPin(LORA_BUSSY_GPIO_Port, LORA_BUSSY_Pin)) {
		uint8_t flags = readRegister(loRa->spi, LR_RegIrqFlags);
		if (flags & PAYLOAD_CRC_ERROR_MASK) {
			flags |= (1 << 7);
			writeRegister(loRa->spi, LR_RegIrqFlags, &flags, 1);
			flags = readRegister(loRa->spi, LR_RegIrqFlags);
		}
		if (HAL_GetTick() - timeout > 2000) {
			return -1;
		}
	}
	return 0;
}


int crcErrorActivation(SX1278_t *module) {
	uint8_t flags;
	flags = readRegister(module->spi, LR_RegIrqFlags);
	SET_BIT(flags, RX_DONE_MASK);
	writeRegister(module->spi, LR_RegIrqFlags, &flags, 1);
	flags = readRegister(module->spi, LR_RegIrqFlags);
	uint8_t errorActivation = READ_BIT(flags, PAYLOAD_CRC_ERROR_MASK);
	return errorActivation;
}

void getRxFifoData(SX1278_t *loRa) {
	uint8_t timeout = 100;
	uint8_t addr = 0x00;
	loRa->rxSize = readRegister(loRa->spi, LR_RegRxNbBytes); //Number for received bytes
	if (loRa->rxSize > 0) {
		HAL_GPIO_WritePin(GPIOB, LORA_NSS_Pin, GPIO_PIN_RESET); // pull the pin low
		HAL_SPI_Transmit(loRa->spi, &addr, 1, timeout); // send address
		HAL_SPI_Receive(loRa->spi, loRa->rxData, 256, timeout); // receive 6 bytes data
		HAL_GPIO_WritePin(GPIOB, LORA_NSS_Pin, GPIO_PIN_SET); // pull the pin high
		loRa->status = RX_DONE;
	}

}

uint8_t setTxFifoAddr(SX1278_t *loRa) {
	uint8_t cmd = loRa->txSize;
	writeRegister(loRa->spi, LR_RegPayloadLength, &(cmd), 1);
	uint8_t addr = readRegister(loRa->spi, LR_RegFifoTxBaseAddr);
	addr = 0x80;
	writeRegister(loRa->spi, LR_RegFifoAddrPtr, &addr, 1);
	cmd = readRegister(loRa->spi, LR_RegPayloadLength);

	return cmd;
}

void setRxFifoAddr(SX1278_t *module) {
	setLoRaLowFreqModeReg(module, SLEEP); //Change modem mode Must in Sleep mode
	uint8_t cmd = module->rxSize;
	writeRegister(module->spi, LR_RegPayloadLength, &(cmd), 1); //RegPayloadLength 21byte
	uint8_t addr = 0x00;
	writeRegister(module->spi, LR_RegFifoAddrPtr, &addr, 1); //RegFifoAddrPtr
	module->rxSize = readRegister(module->spi, LR_RegPayloadLength);
}

uint8_t setTxFifoData(SX1278_t *loRa) {
	uint8_t cmd = loRa->txSize;
	uint8_t base_addr = 0x80;
	if (loRa->txSize > 0) {
		writeRegister(loRa->spi, LR_RegPayloadLength, &(cmd), 1);
		writeRegister(loRa->spi, LR_RegFifoAddrPtr, &base_addr, 1);
		for (int i = 0; i < loRa->txSize; i++)
			writeRegister(loRa->spi, 0x00, loRa->txData + i, 1);
	}
	return (loRa->txSize);
}

uint8_t setTxFifoDataOld(SX1278_t *loRa) {
	uint8_t cmd = loRa->txSize;
	uint8_t base_addr = 0x00;
	if (loRa->txSize > 0) {
		writeRegister(loRa->spi, LR_RegPayloadLength, &(cmd), 1);
		writeRegister(loRa->spi, LR_RegFifoTxBaseAddr, &base_addr, 1);
		uint8_t addr = readRegister(loRa->spi, LR_RegFifoTxBaseAddr);
		addr = 0x00;
		writeRegister(loRa->spi, LR_RegFifoAddrPtr, &addr, 1);
		loRa->txSize = readRegister(loRa->spi, LR_RegPayloadLength);
		for (int i = 0; i < loRa->txSize; i++)
			writeRegister(loRa->spi, 0x00, loRa->txData + i, 1);
	}
	return (loRa->txSize);
}


void receive(SX1278_t *loRa) {
	setRxFifoAddr(loRa);
	setLoRaLowFreqModeReg(loRa, RX_CONTINUOUS);
	clearRxMemory(loRa);
	waitForRxDone(loRa);
	getRxFifoData(loRa);
}

void transmit(SX1278_t *loRa) {
	setTxFifoData(loRa);
	setLoRaLowFreqModeReg(loRa, TX);
	waitForTxEnd(loRa);
}

void validateSettings(SX1278_t *loRa) {

	readPage(M24C64_PAGE0, &(loRa->spreadFactor), 0, 1);
	readPage(M24C64_PAGE0, &(loRa->bandwidth), 1, 1);
	readPage(M24C64_PAGE0, &(loRa->codingRate), 2, 1);
	readPage(M24C64_PAGE1, (uint8_t*) &(loRa->dlFreq), 0, 4);
	readPage(M24C64_PAGE1, (uint8_t*) &(loRa->upFreq), 4, 4);
	if (loRa->spreadFactor < SF_9 || loRa->spreadFactor > SF_12)
		loRa->spreadFactor = SF_10;

	if (loRa->bandwidth < LORABW_7_8KHZ || loRa->bandwidth > LORABW_500KHZ)
		loRa->bandwidth = LORABW_62_5KHZ;

	if (loRa->codingRate < LORA_CR_4_5 || loRa->codingRate > LORA_CR_4_8)
		loRa->codingRate = LORA_CR_4_6;

	if (loRa->upFreq < UPLINK_FREQ_MIN || loRa->upFreq > UPLINK_FREQ_MAX)
		loRa->upFreq = UPLINK_FREQ;

	if (loRa->dlFreq < DOWNLINK_FREQ_MIN || loRa->dlFreq > DOWNLINK_FREQ_MAX)
		loRa->dlFreq = DOWNLINK_FREQ;
}

void HAL_readLoRaSettings(SX1278_t *loRa) {

	readPage(M24C64_PAGE0, &(loRa->spreadFactor), 0, 1);
	readPage(M24C64_PAGE0, &(loRa->bandwidth), 1, 1);
	readPage(M24C64_PAGE0, &(loRa->codingRate), 2, 1);
	readPage(M24C64_PAGE1, (uint8_t*) &(loRa->dlFreq), 0, 4);
	readPage(M24C64_PAGE1, (uint8_t*) &(loRa->upFreq), 4, 4);
	if (loRa->spreadFactor < SF_6 || loRa->spreadFactor > SF_12)
		loRa->spreadFactor = SF_10;

	if (loRa->bandwidth < LORABW_7_8KHZ || loRa->bandwidth > LORABW_500KHZ)
		loRa->bandwidth = LORABW_62_5KHZ;

	if (loRa->codingRate < LORA_CR_4_5 || loRa->codingRate > LORA_CR_4_8)
		loRa->codingRate = LORA_CR_4_6;

	if (loRa->upFreq < UPLINK_FREQ_MIN || loRa->upFreq > UPLINK_FREQ_MAX)
		loRa->upFreq = UPLINK_FREQ;

	if (loRa->dlFreq < DOWNLINK_FREQ_MIN || loRa->dlFreq > DOWNLINK_FREQ_MAX)
		loRa->dlFreq = DOWNLINK_FREQ;
}

SX1278_t* loRaInit(SPI_HandleTypeDef *hspi1, Lora_Mode_t loRaMode) {
	SX1278_t *loRa;
	loRa = malloc(sizeof(SX1278_t));
	HAL_GPIO_WritePin(LORA_NSS_GPIO_Port, LORA_NSS_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LORA_RST_GPIO_Port, LORA_RST_Pin, GPIO_PIN_SET);
	loRa->spi = hspi1;
	loRa->operatingMode = readRegister(loRa->spi, LR_RegOpMode);
	loRa->mode = -1;
	loRa->power = SX1278_POWER_17DBM;
	loRa->LoRa_CRC_sum = CRC_ENABLE;
	loRa->ocp = OVERCURRENTPROTECT;
	loRa->lnaGain = LNAGAIN;
	loRa->AgcAutoOn = 12; // for L-TEL PROTOCOL
	loRa->syncWord = 0x12; // for L-TEL PROTOCOL
	loRa->symbTimeoutLsb = RX_TIMEOUT_LSB;
	loRa->preambleLengthMsb = PREAMBLE_LENGTH_MSB;
	loRa->preambleLengthLsb = PREAMBLE_LENGTH_LSB;
	loRa->preambleLengthLsb = 12; // for L-TEL PROTOCOL
	loRa->fhssValue = HOPS_PERIOD; // for L-TEL PROTOCOL
	loRa->rxSize = 0;
	HAL_readLoRaSettings(loRa);
	changeMode(loRa, loRaMode);
	writeLoRaParametersReg(loRa);
	return loRa;
}

void configureLoRaRx(SX1278_t *loRa, Lora_Mode_t mode) {
	if (loRa->mode != mode)
		return;
	if (loRa->operatingMode == RX_CONTINUOUS)
		return;

	changeMode(loRa, mode);
	writeLoRaParametersReg(loRa);
	setRxFifoAddr(loRa);
	setLoRaLowFreqModeReg(loRa, RX_CONTINUOUS);
}
