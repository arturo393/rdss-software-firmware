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
#define LTEL_START_MARK 0x7e
#define LTEL_END_MARK  0x7f
#define RDSS_BUFFER_SIZE 50
#define LTEL_SET_LENGTH  13
#define LTEL_QUERY_LENGTH  9
#define MINIMUN_FRAME_LEN 6
#define ATTENUATION_VALUE_INDEX 5

typedef enum RS485_CMD {
	NONE,
	QUERY_MODULE_ID = 0x10,
	QUERY_STATUS,
	SET_VLAD_ATTENUATION,

	QUERY_TX_FREQ = 0x20,
	QUERY_RX_FREQ,
	QUERY_UART_BAUDRATE,
	QUERY_BANDWIDTH,
	QUERY_SPREAD_FACTOR,
	QUERY_CODING_RATE,

	SET_MODULE_ID = 0x90,
	SET_TX_FREQ = 0xB0,
	SET_RX_FREQ,
	SET_UART_BAUDRATE,
	SET_BANDWIDTH,
	SET_SPREAD_FACTOR,
	SET_CODING_RATE,



	SET_VLAD_MODE,
	SET_PARAMETER_FREQOUT = 0x31,
	SET_PARAMETERS,
	SET_PARAMETER_FREQBASE,
	QUERY_PARAMETER_PdBm,
} Rs485_cmd_t;

typedef enum RS485_STATUS {
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
	CHECK_LORA_DATA,
	LORA_RECEIVE,
	LORA_SEND,
	UART_SEND,
	UART_VALID
} RDSS_status_t;

typedef enum RS485_i {
	START_INDEX,
	MODULE_TYPE_INDEX,
	MODULE_ID_INDEX,
	CMD_INDEX,
	DATA_LENGHT1_INDEX,
	DATA_LENGHT2_INDEX,
	DATA_START_INDEX
} Rs485_i;

typedef struct RS485 {
	Rs485_cmd_t cmd;
	uint8_t len;
	uint8_t buffer[RDSS_BUFFER_SIZE];
	uint16_t crcCalculated;
	uint16_t crcReceived;
	uint8_t idQuery;
	uint8_t idReceived;
	uint8_t id;
	RDSS_status_t status;
} RDSS_t;

uint16_t crc_get(uint8_t *buffer, uint8_t buff_len);
RDSS_status_t rdssCheckFrame(RDSS_t *r, UART1_t *u);
RDSS_status_t rs485_check_valid_module(UART1_t *uart1);
RDSS_status_t rs485_check_CRC_module(UART1_t *uart1);
RDSS_status_t checkFrameValidity(uint8_t *frame, uint8_t lenght);
RDSS_status_t checkModuleValidity(uint8_t *frame, uint8_t lenght);
RDSS_status_t checkCRCValidity(uint8_t *frame, uint8_t len);
RDSS_status_t isValidId(RDSS_t *r);
RDSS_status_t validateBuffer(uint8_t *buffer,uint8_t length);
RDSS_status_t checkBuffer(RDSS_t *rs485);
void fillValidBuffer(RDSS_t *r, uint8_t *buff, uint8_t len);
//void rs485_set_query_frame(RS485_t* , Module_t *module);
RDSS_t* rdssInit(uint8_t id);
void rs485Uart1Decode(RDSS_t *rs485, UART1_t *uart1, SX1278_t *loraRx);
void reinit(RDSS_t *rs485);
void encodeVlad(uint8_t* buff);
uint8_t setCrc(uint8_t* buff,uint8_t size);
uint8_t setRdssStartData(RDSS_t *rdss, uint8_t *buffer);
uint32_t freqDecode(uint8_t *buffer);
void freqEncode(uint8_t *buffer, uint32_t freqIn);
RDSS_status_t evaluateRdssStatus(RDSS_t *rdss);
bool isModuleCommand(uint8_t cmd);
#endif /* INC_RS485_H_ */

