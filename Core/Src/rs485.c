/*
 * rs485.c
 *
 *  Created on: Sep 28, 2022
 *      Author: sigmadev
 */
#include "rs485.h"
#define MINIMUN_FRAME_LEN 6

uint16_t crc_get(uint8_t *buffer, uint8_t buff_len) {
	uint8_t b;
	uint8_t i;
	uint16_t generator = 0x1021; //divisor is 16bit
	uint16_t crc = 0;			 // CRC value is 16bit

	for (b = 0; b < buff_len; b++) {
		crc ^= ((uint16_t) (buffer[b] << 8)); // move byte into MSB of 16bit CRC
		for (i = 0; i < 8; i++) {
			if ((crc & 0x8000) != 0) // test for MSB = bit 15
				crc = ((uint16_t) ((crc << 1) ^ generator));
			else
				crc <<= 1;
		}
	}
	return crc;
}

void rdssInit(RDSS_t *r, uint8_t id) {
	r->len = 0;
	r->status = WAITING;
	r->cmd = NONE;
	r->id = 6;
	memset(r->buffer, 0, RDSS_BUFFER_SIZE);
	/* PB9 DE485 as output  */
	SET_BIT(GPIOB->MODER, GPIO_MODER_MODE9_0);
	CLEAR_BIT(GPIOB->MODER, GPIO_MODER_MODE9_1);

}
Rs485_status_t rdssCheckFrame(RDSS_t *r, UART1_t *u) {

	if (u->txLen > (MINIMUN_FRAME_LEN)) {
		if (u->rx[0] == LTEL_START_MARK) {
			if (u->rx[u->txLen - 1] == LTEL_END_MARK)
				return VALID_FRAME;
			else
				return START_READING;
		} else
			return NOT_VALID_FRAME;
	} else

		return WAITING;
}

Rs485_status_t rs485_check_CRC_module(UART1_t *uart1) {
	unsigned long crc_cal;
	unsigned long crc_save;
	crc_save = uart1->rx[7] << 8;
	crc_save |= uart1->rx[6];
	crc_cal = crc_get(&(uart1->rx[1]), 5); ///ajustar si se cambia el largo
	if (crc_cal == crc_save)
		return DATA_OK;
	return CRC_ERROR;
}

Rs485_status_t rs485_check_valid_module(UART1_t *uart1) {
	if (uart1->rx[1] == VLADR) {
		if (uart1->rx[2] == ID1) {
			for (int i = 3; i < uart1->txLen; i++)
				if (uart1->rx[i] == LTEL_END_MARK)
					return VALID_MODULE;
		} else
			return WRONG_MODULE_ID;
	} else
		return WRONG_MODULE_FUNCTION;
	return WRONG_MODULE_FUNCTION;
}

Rs485_status_t isValidModule(uint8_t *frame, uint8_t lenght) {
	if (frame[1] == VLADR) {
		for (int i = 3; i < lenght; i++)
			if (frame[i] == LTEL_END_MARK)
				return VALID_MODULE;
	} else
		return WRONG_MODULE_FUNCTION;
	return WRONG_MODULE_FUNCTION;
}

Rs485_status_t isValidFrame(uint8_t *frame, uint8_t lenght) {

	if (lenght > (MINIMUN_FRAME_LEN)) {
		if (frame[0] == LTEL_START_MARK) {
			if (frame[lenght - 1] == LTEL_END_MARK)
				return VALID_FRAME;
			else
				return START_READING;
		} else
			return NOT_VALID_FRAME;
	} else

		return WAITING;
}

Rs485_status_t isValidCrc(uint8_t *frame, uint8_t len) {

	unsigned long crc_cal;
	unsigned long crc_save;
	crc_save = frame[(len - 2)] << 8;
	crc_save |= frame[(len - 3)];
	crc_cal = crc_get(&(frame[1]), (len - 4));
	if (crc_cal == crc_save)
		return DATA_OK;

	return CRC_ERROR;
}

Rs485_status_t isValidCrc2(RDSS_t *rs485) {

	uint8_t *frame = rs485->buffer;
	uint8_t len = rs485->len;
	rs485->crcReceived = 0;
	rs485->crcReceived = frame[(len - 2)] << 8 | frame[(len - 3)];
	rs485->crcCalculated = crc_get(&(frame[1]), (len - 4));
	if (rs485->crcCalculated == rs485->crcReceived)
		return DATA_OK;

	return CRC_ERROR;
}

Rs485_status_t isValidId(RDSS_t *r) {

	r->idReceived = r->buffer[2];

	if (r->idReceived == r->id)
		return DATA_OK;
	else if (r->idReceived == r->idQuery)
		return DATA_OK;
	else if (r->id == ID0)
		return LORA_SEND;
	return WRONG_MODULE_ID;
}

Rs485_status_t isValid(uint8_t *buff,uint8_t len) {
	Rs485_status_t status;
	status = isValidFrame(buff,len);
	if (!(status == VALID_FRAME))
		return status;
	status = isValidModule(buff,len);
	if (!(status == VALID_MODULE))
		return status;
	status = isValidCrc(buff,len);
	if (!(status == DATA_OK))
		return status;
	return status;
}

void fillValidBuffer(RDSS_t *r, uint8_t *buff, uint8_t len) {
	r->status = isValid(buff, len);
	if (r->status == DATA_OK) {
		r->len = len;
		memcpy(r->buffer, buff, len);
	}
}

Rs485_status_t checkBuffer(RDSS_t *rs485) {
	rs485->status = isValidFrame(rs485->buffer, rs485->len);
	if (!(rs485->status == VALID_FRAME))
		return rs485->status;
	rs485->status = isValidModule(rs485->buffer, rs485->len);
	if (!(rs485->status == VALID_MODULE))
		return rs485->status;
	rs485->status = isValidCrc2(rs485);
	if (!(rs485->status == DATA_OK))
		return rs485->status;
	rs485->status = isValidId(rs485);
	if (!(rs485->status == WRONG_MODULE_ID))
		return rs485->status;
	return rs485->status;
}

void rs485Uart1Decode(RDSS_t *rs485, UART1_t *uart1, SX1278_t *loraRx) {
	switch (rs485->status) {
	case VALID_MODULE:
		rs485->status = rs485_check_CRC_module(uart1);
		break;
	case DATA_OK:
		rs485->cmd = uart1->rx[3];
		writeTxStr("DATA OK\r\n");
		rs485->status = DONE;
		break;
	case START_READING:
		rs485->status = WAITING;
		if (cleanByTimeout(uart1, "START_READING"))
			rs485->status = DONE;
		break;
	case VALID_FRAME:
		rs485->status = rs485_check_valid_module(uart1);
		break;
	case NOT_VALID_FRAME:
		HAL_Delay(50);
		writeTxStr("NOT VALID FRAME\r\n");
		cleanRx(uart1);
		rs485->status = DONE;
		break;
	case WRONG_MODULE_ID:
		writeTxStr("WRONG MODULE ID\r\n");
		uart1_clean_buffer(uart1);
		rs485->status = DONE;
		break;
	case WRONG_MODULE_FUNCTION:
		writeTxStr("WRONG MODULE FUNCTION\r\n");
		uart1_clean_buffer(uart1);
		rs485->status = DONE;
		break;
	case CRC_ERROR:
		writeTxStr("CRC ERROR\r\n");
		uart1_clean_buffer(uart1);
		rs485->status = DONE;
		break;
	case WAITING:
		rs485->status = rdssCheckFrame(rs485, uart1);
		cleanByTimeout(uart1, "WAITING");
		break;
	case DONE:
		writeTxStr("DONE\r\n");
		uart1_clean_buffer(uart1);
		rs485->status = WAITING;
		break;
	case CHECK_LORA_DATA:
		writeTxStr("Check Lora\r\n");
		if (isValidFrame(loraRx->buffer, loraRx->len) == VALID_FRAME)
			if (isValidModule(loraRx->buffer, loraRx->len) == VALID_MODULE)
				if (isValidCrc(loraRx->buffer, loraRx->len) == DATA_OK) {
					rs485->cmd = loraRx->buffer[3];
					writeTxStr("DATA OK\r\n");
					rs485->status = DONE;
				}
		break;
	default:
		rs485->status = DONE;
		uart1_clean_buffer(uart1);
		break;
	}
}

void reinit(RDSS_t *rs485) {
	rs485->cmd = NONE;
	rs485->status = WAITING;
	if (rs485->buffer[0] == '\0')
		return;
	memset(rs485->buffer, 0, sizeof(rs485->buffer));
	rs485->len = 0;
}

void encodeVlad(uint8_t* buff){
	uint16_t lineVoltage = rand() % 610;
	uint16_t baseCurrent = rand() % 301;
	uint16_t tunnelCurrent = rand() % 1001;
	uint16_t unitCurrent = rand() % 301;
	uint8_t uplinkAgc = rand() % 43;
	uint8_t downlinkInputPower = rand() % 130;
	uint8_t downlinkAgc = rand() % 43;
	uint8_t uplinkOuputPower = rand() % 130;

	buff[7] = (lineVoltage >> 8) & 0xFF;
	buff[6] = lineVoltage & 0xFF;
	buff[9] = (baseCurrent >> 8) & 0xFF;
	buff[8] = baseCurrent & 0xFF;
	buff[11] = (tunnelCurrent >> 8) & 0xFF;
	buff[10] = tunnelCurrent & 0xFF;
	buff[13] = (unitCurrent >> 8) & 0xFF;
	buff[12] = unitCurrent & 0xFF;
	buff[14] = uplinkAgc;
	buff[15] = downlinkInputPower;
	buff[16] = downlinkAgc;
	buff[17] = uplinkOuputPower;
}

uint8_t setCrc(uint8_t* buff,uint8_t i){
	uint8_t crc_frame[2];
	uint16_t crc;
	crc = crc_get(&(buff[1]), i - 1);
	memcpy(crc_frame, &crc, 2);
	buff[i++] = crc_frame[0];
	buff[i++] = crc_frame[1];
	return i;
}

uint8_t setRdssStartData(RDSS_t *rdss, uint8_t *buffer) {
	uint8_t i = 0;
	if (rdss->cmd == 0)
		return i;
	if (rdss->id == 0)
		return i;
	buffer[i++] = LTEL_START_MARK;
	buffer[i++] = VLADR;
	buffer[i++] = rdss->id;
	buffer[i++] = rdss->cmd;
	buffer[i++] = 0x00;
	return i;
}

float freqDecode(uint8_t *buffer) {
	union floatConverter freq;
	freq.i = 0;
	freq.i |= (buffer[0]);
	freq.i |= (buffer[1] << 8);
	freq.i |= (buffer[2] << 16);
	freq.i |= (buffer[3] << 24);
	return freq.f * 1000000.0f;
}

void freqEncode(uint8_t *buffer, uint32_t freqIn) {
	union floatConverter freqOut;
	freqOut.f = freqIn / 1000000.0f;
	memcpy(buffer, &freqOut.i, sizeof(freqOut.i));
}
