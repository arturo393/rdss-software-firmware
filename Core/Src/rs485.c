/*
 * rs485.c
 *
 *  Created on: Sep 28, 2022
 *      Author: sigmadev
 */
#include "rs485.h"

RDSS_t* rdssInit(uint8_t id) {
	RDSS_t *r;
	r = malloc(sizeof(RDSS_t));
	r->len = 0;
	r->status = WAITING;
	r->cmd = NONE;
	r->id = id;
	memset(r->buffer, 0, RDSS_BUFFER_SIZE);
	/* PB9 DE485 as output  */
	SET_BIT(GPIOB->MODER, GPIO_MODER_MODE9_0);
	CLEAR_BIT(GPIOB->MODER, GPIO_MODER_MODE9_1);
	return r;
}

RDSS_status_t rs485_check_CRC_module(UART1_t *uart1) {
	unsigned long crc_cal;
	unsigned long crc_save;
	crc_save = uart1->receiveBuffer[7] << 8;
	crc_save |= uart1->receiveBuffer[6];
	crc_cal = crc_get(&(uart1->receiveBuffer[1]), 5); ///ajustar si se cambia el largo
	if (crc_cal == crc_save)
		return DATA_OK;
	return CRC_ERROR;
}

RDSS_status_t checkModuleValidity(uint8_t *frame, uint8_t lenght) {
	if (frame[1] == VLADR) {
		for (int i = 3; i < lenght; i++)
			if (frame[i] == LTEL_END_MARK)
				return VALID_MODULE;
	} else
		return WRONG_MODULE_FUNCTION;
	return WRONG_MODULE_FUNCTION;
}

RDSS_status_t checkFrameValidity(uint8_t *frame, uint8_t lenght) {

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

RDSS_status_t checkCRCValidity(uint8_t *frame, uint8_t len) {
	uint16_t calculatedCrc, savedCrc;
	savedCrc = ((uint16_t) frame[len - 2] << 8);
	savedCrc |= (uint16_t) frame[len - 3];
	calculatedCrc = crc_get(&frame[1], len - 4);

	return (calculatedCrc == savedCrc) ? DATA_OK : CRC_ERROR;
}

uint16_t crc_get(uint8_t *buffer, uint8_t buff_len) {
	uint8_t byte_idx;
	uint8_t bit_idx;
	uint16_t generator = 0x1021; // 16-bit divisor
	uint16_t crc = 0;            // 16-bit CRC value

	for (byte_idx = 0; byte_idx < buff_len; byte_idx++) {
		crc ^= ((uint16_t) (buffer[byte_idx] << 8)); // Move byte into MSB of 16-bit CRC

		for (bit_idx = 0; bit_idx < 8; bit_idx++) {
			if ((crc & 0x8000) != 0) { // Test for MSB = bit 15
				crc = ((uint16_t) ((crc << 1) ^ generator));
			} else {
				crc <<= 1;
			}
		}
	}

	return crc;
}

RDSS_status_t isValidId(RDSS_t *r) {

	r->idReceived = r->buffer[2];
	if (r->idReceived == r->id)
		return DATA_OK;
	else if (r->idReceived == r->idQuery)
		return DATA_OK;
	else if (r->id == ID0)
		return LORA_SEND;
	return WRONG_MODULE_ID;
}

RDSS_status_t validateBuffer(uint8_t *buffer,uint8_t length) {
	RDSS_status_t frameStatus = checkFrameValidity(buffer, length);
	if (frameStatus != VALID_FRAME)
		return frameStatus;
	RDSS_status_t moduleStatus = checkModuleValidity(buffer, length);
	if (moduleStatus != VALID_MODULE)
		return moduleStatus;
	RDSS_status_t crcStatus = checkCRCValidity(buffer, length);
	if (crcStatus != DATA_OK)
		return crcStatus;
	return DATA_OK;
}

void fillValidBuffer(RDSS_t *r, uint8_t *buff, uint8_t len) {
	r->status = validateBuffer(buff, len);
	if (r->status == DATA_OK) {
		r->len = len;
		memcpy(r->buffer, buff, len);
	}
}

RDSS_status_t evaluateRdssStatus(RDSS_t *rdss) {
    if (rdss->buffer[MODULE_ID_INDEX] == rdss->id) {
        return UART_VALID;
    } else {
        return LORA_SEND;
    }
}

bool isModuleCommand(uint8_t cmd) {
    return cmd == QUERY_MODULE_ID || cmd == SET_MODULE_ID;
}

RDSS_status_t checkBuffer(RDSS_t *rs485) {
	rs485->status = checkFrameValidity(rs485->buffer, rs485->len);
	if (!(rs485->status == VALID_FRAME))
		return rs485->status;
	rs485->status = checkModuleValidity(rs485->buffer, rs485->len);
	if (!(rs485->status == VALID_MODULE))
		return rs485->status;
	rs485->status = checkCRCValidity(rs485->buffer,rs485->len);
	if (!(rs485->status == DATA_OK))
		return rs485->status;
	rs485->status = isValidId(rs485);
	if (!(rs485->status == WRONG_MODULE_ID))
		return rs485->status;
	return rs485->status;
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
	uint16_t lineVoltage = (uint16_t) rand() % 610;
	uint16_t baseCurrent = (uint16_t)  rand() % 301;
	uint16_t tunnelCurrent = (uint16_t) rand() % 1001;
	uint16_t unitCurrent = (uint16_t) rand() % 301;
	uint8_t uplinkAgc = (uint8_t) rand() % 43;
	uint8_t downlinkInputPower = (uint8_t) rand() % 130;
	uint8_t downlinkAgc = (uint8_t) rand() % 43;
	uint8_t uplinkOuputPower = (uint8_t) rand() % 130;

	buff[7] = (uint8_t)(lineVoltage >> 8) & 0xFF;
	buff[6] = (uint8_t) lineVoltage & 0xFF;
	buff[9] = (uint8_t)(baseCurrent >> 8) & 0xFF;
	buff[8] = (uint8_t)baseCurrent & 0xFF;
	buff[11] = (uint8_t)(tunnelCurrent >> 8) & 0xFF;
	buff[10] = (uint8_t)tunnelCurrent & 0xFF;
	buff[13] = (uint8_t)(unitCurrent >> 8) & 0xFF;
	buff[12] = (uint8_t)unitCurrent & 0xFF;
	buff[14] = uplinkAgc;
	buff[15] = downlinkInputPower;
	buff[16] = downlinkAgc;
	buff[17] = uplinkOuputPower;
}

uint8_t setCrc(uint8_t* buff,uint8_t size){
	uint8_t crc_frame[2];
	uint16_t crc;
	crc = crc_get(buff+1, size-1);
	memcpy(crc_frame, &crc, 2);
	buff[size++] = crc_frame[0];
	buff[size++] = crc_frame[1];
	return 2;
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

uint32_t freqDecode(uint8_t *buffer) {
	union floatConverter freq;
	freq.i = 0;
	freq.i |= (buffer[0]);
	freq.i |= (buffer[1] << 8);
	freq.i |= (buffer[2] << 16);
	freq.i |= (buffer[3] << 24);
	freq.f = freq.f * 1000000.0f;

	return freq.f;
}

void freqEncode(uint8_t *buffer, uint32_t freqIn) {
	union floatConverter freqOut;
	freqOut.f = freqIn / 1000000.0f;
	memcpy(buffer, &freqOut.i, sizeof(freqOut.i));
}
