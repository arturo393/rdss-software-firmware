/*
 * rs485.h
 *
 *  Created on: Sep 28, 2022
 *      Author: sigmadev
 */

#ifndef INC_RS485_H_
#define INC_RS485_H_

#include "main.h"
#include "stdbool.h"
#include "module.h"
#include "uart1.h"
#include "stdio.h"
#include "string.h"
#include "SX1278.h"

#define LTEL_FRAME_SIZE 14
#define SIGMA_FRAME_SIZE 14

typedef enum RS485_CMD {
	NONE,
	QUERY_PARAMETERS_VLAD = 0x11,
	SET_VLAD_MODE,
	SET_PARAMETER_FREQOUT = 0x31,
    SET_PARAMETERS,
    SET_PARAMETER_FREQBASE,
    QUERY_PARAMETER_PdBm,
    SET_MODE,
} Rs485_cmd_t;

typedef enum RS485_STATUS{
	DATA_OK,
	START_READING,
	VALID_FRAME,
	NOT_VALID_FRAME,
	WRONG_MODULE_FUNCTION,
	WRONG_MODULE_ID,
	CRC_ERROR,
	DONE,
	WAITING,
	VALID_MODULE,
    CHECK_LORA_DATA
}Rs485_status_t ;

typedef struct RS485{
	Rs485_cmd_t cmd;
	uint8_t len;
	uint8_t buffer[100];
	Rs485_status_t status;
}RS485_t;

Rs485_status_t rs485_check_frame(RS485_t *r, UART1_t *u);
Rs485_status_t rs485_check_valid_module(UART1_t *uart1);
Rs485_status_t rs485_check_CRC_module(UART1_t *uart1);
Rs485_status_t check_frame(uint8_t *frame, uint8_t lenght);
Rs485_status_t check_valid_module(uint8_t *frame, uint8_t lenght);
Rs485_status_t check_CRC_module(uint8_t *frame, uint8_t len);
//void rs485_set_query_frame(RS485_t* , Module_t *module);
void rs485Init(RS485_t *);
void rs485Uart1Decode(RS485_t *rs485, UART1_t *uart1, SX1278_t *loraRx);

#endif /* INC_RS485_H_ */
