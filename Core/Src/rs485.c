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

void rs485Init(RS485_t *r) {
	r->len = 0;
	r->status = DONE;
	r->cmd = NONE;
	memset(r->buffer, 0, 100);
	/* PB8 DE485 as output  */
	SET_BIT(GPIOB->ODR, GPIO_ODR_ODR8);
	CLEAR_BIT(GPIOB->ODR, GPIO_ODR_ODR8);

}
Rs485_status_t rs485_check_frame(RS485_t *r, UART1_t *u) {

	if (u->len > (MINIMUN_FRAME_LEN)) {
		if (u->rxBuffer[0] == LTEL_START_MARK) {
			if (u->rxBuffer[u->len - 1] == LTEL_END_MARK)
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
	crc_save = uart1->rxBuffer[7] << 8;
	crc_save |= uart1->rxBuffer[6];
	crc_cal = crc_get(&(uart1->rxBuffer[1]), 5);
	if (crc_cal == crc_save)
		return DATA_OK;
	return CRC_ERROR;
}

Rs485_status_t rs485_check_valid_module(UART1_t *uart1) {
	if (uart1->rxBuffer[1] == VLAD) {
		if (uart1->rxBuffer[2] == ID1) {
			for (int i = 3; i < uart1->len; i++)
				if (uart1->rxBuffer[i] == LTEL_END_MARK)
					return VALID_MODULE;
		} else
			return WRONG_MODULE_ID;
	} else
		return WRONG_MODULE_FUNCTION;
	return WRONG_MODULE_FUNCTION;
}

Rs485_status_t check_valid_module(uint8_t *frame, uint8_t lenght) {
	if (frame[1] == VLAD) {
		if (frame[2] == ID1) {
			for (int i = 3; i < lenght; i++)
				if (frame[i] == LTEL_END_MARK)
					return VALID_MODULE;
		} else
			return WRONG_MODULE_ID;
	} else
		return WRONG_MODULE_FUNCTION;
	return WRONG_MODULE_FUNCTION;
}

Rs485_status_t check_frame(uint8_t *frame, uint8_t lenght) {

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

Rs485_status_t check_CRC_module(uint8_t *frame, uint8_t len) {

	unsigned long crc_cal;
	unsigned long crc_save;
	crc_save = frame[(len - 2)] << 8;
	crc_save |= frame[(len - 3)];
	crc_cal = crc_get(&(frame[1]), (len - 4));
	if (crc_cal == crc_save)
		return DATA_OK;
	return CRC_ERROR;
}

Rs485_status_t checkBuffer(RS485_t *rs485){
	rs485->status = check_frame(rs485->buffer,rs485->len);
	if (!(rs485->status == VALID_FRAME))
		return rs485->status;
	rs485->status = check_valid_module(rs485->buffer, rs485->len);
	if (!(rs485->status == VALID_MODULE))
		return rs485->status;
	rs485->status = check_CRC_module(rs485->buffer, rs485->len);
	if (!(rs485->status == DATA_OK))
		return rs485->status;
	return rs485->status;
}

void rs485Uart1Decode(RS485_t *rs485, UART1_t *uart1, SX1278_t *loraRx) {
	switch (rs485->status) {
	case VALID_MODULE:
		rs485->status = rs485_check_CRC_module(uart1);
		break;
	case DATA_OK:
		rs485->cmd = uart1->rxBuffer[3];
		uart1_send_str("DATA OK\r\n");
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
		uart1_send_str("NOT VALID FRAME\r\n");
		uart1_clean_buffer(uart1);
		rs485->status = DONE;
		break;
	case WRONG_MODULE_ID:
		uart1_send_str("WRONG MODULE ID\r\n");
		uart1_clean_buffer(uart1);
		rs485->status = DONE;
		break;
	case WRONG_MODULE_FUNCTION:
		uart1_send_str("WRONG MODULE FUNCTION\r\n");
		uart1_clean_buffer(uart1);
		rs485->status = DONE;
		break;
	case CRC_ERROR:
		uart1_send_str("CRC ERROR\r\n");
		uart1_clean_buffer(uart1);
		rs485->status = DONE;
		break;
	case WAITING:
		rs485->status = rs485_check_frame(rs485, uart1);
		cleanByTimeout(uart1, "WAITING");
		break;
	case DONE:
		uart1_send_str("DONE\r\n");
		uart1_clean_buffer(uart1);
		rs485->status = WAITING;
		break;
	case CHECK_LORA_DATA:
		uart1_send_str("Check Lora\r\n");
		if (check_frame(loraRx->buffer, loraRx->len) == VALID_FRAME)
			if (check_valid_module(loraRx->buffer, loraRx->len)
					== VALID_MODULE)
				if (check_CRC_module(loraRx->buffer, loraRx->len)
						== DATA_OK) {
					rs485->cmd = loraRx->buffer[3];
					uart1_send_str("DATA OK\r\n");
					rs485->status = DONE;
				}
	default:
		rs485->status = DONE;
		uart1_clean_buffer(uart1);
		break;
	}
}
