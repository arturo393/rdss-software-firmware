/*
 * rdss_slave.c
 *
 *  Created on: Aug 11, 2023
 *      Author: artur
 */

#include "rdss_slave.h"

uint8_t executeCommand(uint8_t *buffer, RDSS_t *rdss, SX1278_t *loRa,
		Vlad_t *vlad) {
	uint8_t index = 0;
	uint8_t attenuationCommand[2];
	const uint8_t i2cSlaveAddress = 0x08;

	if (rdss->buffSize < LTEL_QUERY_LENGTH || rdss->buffSize > LTEL_SET_LENGTH)
		return 0;

	index = setRdssStartData(rdss, buffer);

	switch (rdss->cmd) {
	case QUERY_RX_FREQ:
		buffer[index++] = 4;
		freqEncode(buffer + index, loRa->dlFreq);
		index += sizeof(loRa->dlFreq);
		index++;
		break;
	case QUERY_TX_FREQ:
		buffer[index++] = 4;
		freqEncode(buffer + index, loRa->upFreq);
		index += sizeof(loRa->upFreq);
		index++;
		break;
	case QUERY_SPREAD_FACTOR:
		buffer[index++] = 1;
		buffer[index++] = loRa->spreadFactor - 6;
		break;
	case QUERY_CODING_RATE:
		buffer[index++] = 1;
		buffer[index++] = loRa->codingRate;
		break;
	case QUERY_BANDWIDTH:
		buffer[index++] = 1;
		buffer[index++] = loRa->bandwidth + 1;
		break;
	case QUERY_MODULE_ID:
		index = 0;
		buffer[index++] = LTEL_START_MARK;
		buffer[index++] = VLADR;
		buffer[index++] = rdss->id;
		buffer[index++] = QUERY_MODULE_ID;
		buffer[index++] = 0x00;
		buffer[index++] = 2;
		buffer[index++] = VLADR;
		buffer[index++] = rdss->id;
		break;
	case SET_MODULE_ID:
		vlad->function = rdss->buff[6];
		vlad->id = rdss->buff[7];
		rdss->id = vlad->id;
		index = setRdssStartData(rdss, buffer);
		buffer[index++] = VLADR;
		buffer[index++] = rdss->id;
		savePage(CAT24C02_PAGE0_START_ADDR, (uint8_t*) &(vlad->function), 3, 1);
		savePage(CAT24C02_PAGE0_START_ADDR, (uint8_t*) &(vlad->id), 4, 1);
		break;
	case SET_TX_FREQ:
		buffer[index++] = 4;
		loRa->upFreq = freqDecode(rdss->buff + index);
		index += sizeof(loRa->upFreq);
		savePage(CAT24C02_PAGE1_START_ADDR, (uint8_t*) &(loRa->upFreq), 0, 4);
		changeMode(loRa, loRa->mode);
		writeLoRaParametersReg(loRa);
		break;
	case SET_RX_FREQ:
		buffer[index++] = 4;
		loRa->dlFreq = freqDecode(rdss->buff + index);
		index += sizeof(loRa->dlFreq);
		savePage(CAT24C02_PAGE1_START_ADDR, (uint8_t*) &(loRa->dlFreq), 4, 4);
		changeMode(loRa, loRa->mode);
		writeLoRaParametersReg(loRa);
		break;
	case SET_BANDWIDTH:
		buffer[index++] = 1;
		loRa->bandwidth = rdss->buff[index++] - 1;
		savePage(CAT24C02_PAGE0_START_ADDR, &(loRa->bandwidth), 1, 1);
		changeMode(loRa, loRa->mode);
		writeLoRaParametersReg(loRa);
		break;
	case SET_SPREAD_FACTOR:
		buffer[index++] = 1;
		loRa->spreadFactor = rdss->buff[index++] + 6;
		savePage(CAT24C02_PAGE0_START_ADDR, &(loRa->spreadFactor), 0, 1);
		changeMode(loRa, loRa->mode);
		writeLoRaParametersReg(loRa);
		break;
	case SET_CODING_RATE:
		buffer[index++] = 1;
		loRa->codingRate = rdss->buff[index++];
		savePage(CAT24C02_PAGE0_START_ADDR, &(loRa->codingRate), 2, 1);
		changeMode(loRa, loRa->mode);
		writeLoRaParametersReg(loRa);
		break;
	case SET_VLAD_ATTENUATION:
		attenuationCommand[0] = SET_VLAD_ATTENUATION;
		attenuationCommand[1] = rdss->buff[ATTENUATION_VALUE_INDEX];
		vlad->is_attenuation_updated = i2c1MasterTransmit(i2cSlaveAddress,
				attenuationCommand, sizeof(attenuationCommand), 10);
		buffer[index++] = vlad->is_attenuation_updated;
		break;
	default:
		break;
	}

	index += setCrc(buffer, index);
	buffer[index++] = LTEL_END_MARK;
	return (index);

}

void printLoRaStatus(UART1_t *u1, SX1278_t *loRa) {
	if (!u1->isDebugModeEnabled) {
		return;
	}

	char *buff;
	uint8_t size;
	buff = malloc(sizeof(char) * 20);

	switch (loRa->status) {
	case TX_TIMEOUT:
		size = (uint8_t) sprintf(buff,
				"Transmission Fail: %d seconds Timeout\r\n", TX_TIMEOUT / 1000);
		writeTx(u1);
		break;
	case TX_DONE:
		size = (uint8_t) sprintf(buff, "Transmission Done: %lu ms\r\n",
				loRa->lastTxTime);
		writeTx(u1);
		break;
	case TX_BUFFER_READY:
		size = (uint8_t) sprintf(buff,
				"Transmission Buffer: %d bytes data \r\n", loRa->len);
		writeTx(u1);
		for (int i = 0; i < loRa->len; i++) {
			size = (uint8_t) sprintf(buff, "%02X", loRa->txData[i]);
			writeTx(u1);
		}
		writeTxReg('\n');
		break;
	case TX_MODE:
		size = (uint8_t) sprintf(buff, "%s Mode\r\n",
				(loRa->mode == MASTER_SENDER) ? "Master Sender" :
				(loRa->mode == SLAVE_SENDER) ? "Slave Sender" : "Unknown");
		writeTx(u1);
		break;
	case RX_DONE:
		size = (uint8_t) sprintf(buff, "Reception Done: %d bytes\r\n",
				loRa->len);
		writeTx(u1);
		for (int i = 0; i < loRa->len; i++) {
			size = (uint8_t) sprintf(buff, "%02X", loRa->rxData[i]);
			writeTx(u1);
		}
		if (loRa->len > 0) {
			writeTxReg('\n');
		}
		break;
	case RX_MODE:
		size = (uint8_t) sprintf(buff, "%s Mode\r\n",
				(loRa->mode == MASTER_RECEIVER) ? "Master Receiver" :
				(loRa->mode == SLAVE_RECEIVER) ? "Slave Receiver" : "Unknown");
		writeTx(u1);
		break;
	case CRC_ERROR_ACTIVATION:
		size = (uint8_t) sprintf(buff,
				"Reception Fail: CRC error activation\r\n");
		writeTx(u1);
		break;
	default:
		break;
	}
	cleanTx(u1);
}

uint8_t i2cGetVladMeasurements(uint8_t *buffer, uint8_t querySize) {
	const uint8_t slaveAddress = 0x08;
	uint8_t vladMeasurementsCmd[2] = { 0x10, 0x00 };

	if (!i2c1MasterTransmit(slaveAddress, vladMeasurementsCmd,
			sizeof(vladMeasurementsCmd), 10)) {
		memset(buffer, 0, querySize);
		return querySize;
	}
	HAL_Delay(6);
	if (!i2c1MasterReceive(slaveAddress, buffer, querySize, 10)) {
		memset(buffer, 0, querySize);
		return querySize;
	}

	return querySize;
}

void processUart1Rx(UART1_t *u1, RDSS_t *rdss, SX1278_t *loRa, Vlad_t *vlad) {
	if (u1->isReceivedDataReady == false)
		return;
	u1->isReceivedDataReady = false;
	HAL_Delay(1);
	if (validate(u1->rxData, u1->rxSize) != DATA_OK) {
		// Clear UART buffer and length
		memset(u1->rxData, 0, sizeof(u1->rxData));
		u1->rxSize = 0;
		return;
	}
	updateRdss(rdss, u1->rxData, u1->rxSize);
	u1->txData = malloc(sizeof(uint8_t) * 25);
	u1->txSize = executeCommand(u1->txData, rdss, loRa, vlad);
	for (uint8_t i = 0; i < u1->txSize; i++)
		writeTxReg(u1->txData[i]);
	free(u1->txData);
	memset(u1->rxData, 0, sizeof(u1->rxData));
	u1->rxSize = 0;
	reinit(rdss);
}

void processCommand(SX1278_t *loRa, RDSS_t *rdss, Vlad_t *vlad) {
	loRa->txData = malloc(sizeof(uint8_t) * 25);
	loRa->txSize = executeCommand(loRa->txData, rdss, loRa, vlad);
	changeMode(loRa, SLAVE_SENDER); // Change LoRa operating mode to SLAVE_SENDER
	transmit(loRa); // Transmit data using LoRa
	changeMode(loRa, SLAVE_RECEIVER); // Change LoRa operating mode to SLAVE_RECEIVER
	free(loRa->txData);
}

void setQueryStatusBuffer(RDSS_t *rdss, Vlad_t *vlad) {
	uint8_t index = 0;
	rdss->queryBuffer[index++] = LTEL_START_MARK;
	rdss->queryBuffer[index++] = VLADR;
	rdss->queryBuffer[index++] = vlad->id;
	rdss->queryBuffer[index++] = QUERY_STATUS;
	rdss->queryBuffer[index++] = 0x00;
	index += encodeVladToLtel(rdss->queryBuffer + index, vlad);
	index += setCrc(rdss->queryBuffer, index);
	rdss->queryBuffer[index++] = LTEL_END_MARK;
}

void setQueryRdssBuffer(RDSS_t *rdss, Vlad_t *vlad) {
	uint8_t index = 0;
	rdss->queryBuffer[index++] = LTEL_START_MARK;
	rdss->queryBuffer[index++] = VLADR;
	rdss->queryBuffer[index++] = vlad->id;
	rdss->queryBuffer[index++] = QUERY_STATUS;
	rdss->queryBuffer[index++] = 0x00;
	index += encodeVladToRdss(rdss->queryBuffer + index, vlad);
	index += setCrc(rdss->queryBuffer, index);
	rdss->queryBuffer[index++] = LTEL_END_MARK;
}

void i2cSetQueryRdssBuffer(RDSS_t *rdss) {
	uint8_t index = 0;
	const uint8_t querySize = 25;
	memset(rdss->queryBuffer, 0, QUERY_STATUS_BUFFER_SIZE);
	rdss->queryBuffer[index++] = LTEL_START_MARK;
	rdss->queryBuffer[index++] = VLADR;
	rdss->queryBuffer[index++] = rdss->id;
	rdss->queryBuffer[index++] = QUERY_STATUS;
	rdss->queryBuffer[index++] = 0x00;
	rdss->queryBuffer[index++] = querySize;
	index += i2cGetVladMeasurements(rdss->queryBuffer + index, querySize);
	index += setCrc(rdss->queryBuffer, index);
	rdss->queryBuffer[index++] = LTEL_END_MARK;
}

void transmitStatus(SX1278_t *loRa, RDSS_t *rdss) {
	loRa->txData = rdss->queryBuffer;
	loRa->txSize = QUERY_STATUS_BUFFER_SIZE;
	changeMode(loRa, SLAVE_SENDER); // Change LoRa operating mode to SLAVE_SENDER
	transmit(loRa); // Transmit data using LoRa
	if (loRa->status == TX_DONE)
		HAL_GPIO_WritePin(LORA_TX_OK_GPIO_Port, LORA_TX_OK_Pin, GPIO_PIN_SET);
	changeMode(loRa, SLAVE_RECEIVER); // Change LoRa operating mode to SLAVE_RECEIVER
	HAL_GPIO_WritePin(LORA_TX_OK_GPIO_Port, LORA_TX_OK_Pin, GPIO_PIN_RESET);
}

void slaveProcessRdss(RDSS_t *rdss, SX1278_t *loRa, Vlad_t *vlad) {
	if (rdss->idReceived != rdss->id) {
		rdss->status = WRONG_MODULE_ID;
		return;
	}
	if (rdss->cmd == QUERY_STATUS)
		transmitStatus(loRa, rdss);
	else
		processCommand(loRa, rdss, vlad);
}

void readWhenDataArrive(SX1278_t *loRa, RDSS_t *rdss, Vlad_t *vlad) {
	if (HAL_GPIO_ReadPin(LORA_BUSSY_GPIO_Port, LORA_BUSSY_Pin)
			== GPIO_PIN_RESET)
		return; // if (crcErrorActivation(loRa) != 1)

	clearIrqFlagsReg(loRa);
	loRa->rxData = getRxFifoData(loRa);
	if (loRa->rxSize <= 0) {
		changeMode(loRa, loRa->mode);
		return;

	}
	if (validate(loRa->rxData, loRa->rxSize) != DATA_OK) {
		changeMode(loRa, loRa->mode);
		return;
	}
	HAL_GPIO_WritePin(LORA_RX_OK_GPIO_Port, LORA_RX_OK_Pin, GPIO_PIN_SET);
	updateRdss(rdss, loRa->rxData, loRa->rxSize);
	slaveProcessRdss(rdss, loRa, vlad);
	rdssReinit(rdss);
	free(loRa->rxData);
	loRa->rxSize = 0;
	HAL_GPIO_WritePin(LORA_RX_OK_GPIO_Port, LORA_RX_OK_Pin, GPIO_PIN_RESET);
}

void updateQueryBuffer(RDSS_SLAVE_t *slave, uint32_t timeout) {
	uint8_t *query = slave->query;
	HAL_StatusTypeDef res;
	if (HAL_GetTick() - slave->queryTicks > timeout) {
		const uint8_t dataLen = 25;
		res = getVlad(slave, dataLen);
		if (res != HAL_OK) {
			memset(query + DATA_START_INDEX, 0, dataLen);
			return;
		}
		query[START_INDEX] = LTEL_START_MARK;
		query[MODULE_TYPE_INDEX] = VLADR;
		query[MODULE_ID_INDEX] = slave->id;
		query[CMD_INDEX] = QUERY_STATUS;
		query[DATA_LENGHT1_INDEX] = 0x00;
		uint8_t CRC_INDEX = DATA_START_INDEX + dataLen;
		uint8_t END_INDEX = CRC_INDEX + CRC_SIZE;
		query[DATA_LENGHT2_INDEX] = dataLen;
		query[END_INDEX] = LTEL_END_MARK;
		setCrc(query, DATA_START_INDEX + dataLen);
		slave->queryTicks = HAL_GetTick();
	}
}

HAL_StatusTypeDef getVlad(RDSS_SLAVE_t *slave, uint8_t querySize) {
	I2C_HandleTypeDef *i2c = slave->i2c->handler;
	uint8_t *response = slave->query;
	const uint8_t slaveAddress = 0x08;
	uint8_t cmd[2] = { 0x10, 0x00 };
	HAL_StatusTypeDef res;
	res = HAL_I2C_Master_Transmit(i2c, slaveAddress, cmd, sizeof(cmd),
	HAL_MAX_DELAY);
	if (res != HAL_OK)
		return (res);
	HAL_Delay(6);
	res = HAL_I2C_Master_Receive(i2c, slaveAddress, response, QUERY_SIZE,
	HAL_MAX_DELAY);
	if (res != HAL_OK)
		return (res);
	return (res);
}
