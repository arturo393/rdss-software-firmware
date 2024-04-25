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

#define RDSS_FRAME_SIZE 14
#define SIGMA_FRAME_SIZE 14
#define RDSS_START_MARK 0x7e
#define RDSS_END_MARK  0x7f
#define RDSS_BUFFER_SIZE 50
#define LTEL_SET_LENGTH  13
#define LTEL_QUERY_LENGTH  9
#define MINIMUN_FRAME_LEN 6
#define CRC_HIGH_BYTE_OFFSET 2
#define CRC_LOW_BYTE_OFFSET 3
#define FRAME_HEADER_SIZE 4

typedef enum RS485_CMD {
	NONE,
	QUERY_MODULE_ID = 0x10,
	QUERY_STATUS,
	SET_VLAD_ATTENUATION,
	QUERY_MASTER_STATUS,
	QUERY_UART1,
	SET_UART1,


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
	SET_OUT,
	SET_AOUT_0_10V,
	SET_AOUT_4_20mA,
	SET_AOUT_0_20mA,
	SET_DOUT1,

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
	uint8_t *buff;
	uint8_t buffSize;
	uint16_t crcCalculated;
	uint16_t crcReceived;
	uint8_t idQuery;
	uint8_t idReceived;
	uint8_t id;
	RDSS_status_t status;
	RDSS_status_t lastStatus;
	uint8_t queryBuffer[30];
	uint32_t lastUpdateTicks;
} RDSS_t;

/**
 * Initialize the RDSS structure.
 *
 * @param id ID of the RDSS
 * @return Pointer to the initialized RDSS structure
 */
RDSS_t* rdssInit(uint8_t id);

/**
 * Reinitialize the RDSS structure by resetting its fields.
 *
 * @param rdss Pointer to the RDSS structure to be reinitialized
 */
void rdssReinit(RDSS_t *rdss);

/**
 * Check the CRC validity of the received data in the UART1 buffer.
 *
 * @param uart1 Pointer to the UART1 structure containing the received data
 * @return RDSS_status_t indicating the status of the CRC check
 */
RDSS_status_t rs485_check_CRC_module(UART1_t *uart1);

/**
 * Check the validity of the module specified in the frame.
 *
 * @param frame Pointer to the frame
 * @param length Length of the frame
 * @return RDSS_status_t indicating the status of the module validity check
 */
RDSS_status_t checkModuleValidity(uint8_t *frame, uint8_t length);

/**
 * Check the validity of the frame.
 *
 * @param frame Pointer to the frame
 * @param length Length of the frame
 * @return RDSS_status_t indicating the status of the frame validity check
 */
RDSS_status_t checkFrameValidity(uint8_t *frame, uint8_t length);

/**
 * Check the validity of the CRC in the frame.
 *
 * @param frame Pointer to the frame
 * @param len Length of the frame
 * @return RDSS_status_t indicating the status of the CRC validity check
 */
RDSS_status_t checkCRCValidity(uint8_t *frame, uint8_t len);

/**
 * Calculate the CRC value of the given buffer.
 *
 * @param buffer Pointer to the buffer
 * @param buff_len Length of the buffer
 * @return Calculated CRC value
 */
uint16_t crc_get(uint8_t *buffer, uint8_t buff_len);

/**
 * Validate the received data by checking the frame validity, module validity, and CRC validity.
 *
 * @param buffer Pointer to the received data buffer
 * @param length Length of the received data
 * @return RDSS_status_t indicating the overall data validity
 */
RDSS_status_t validate(uint8_t *buffer, uint8_t length);

/**
 * Encode the VLAD (Voltage, Current, AGC, Power) data into the buffer.
 *
 * @param buff Pointer to the buffer to store the encoded data
 */
void encodeVlad(uint8_t *buff);

/**
 * Set the CRC in the buffer and return the number of bytes added for the CRC.
 *
 * @param buff Pointer to the buffer
 * @param size Size of the buffer
 * @return Number of bytes added for the CRC
 */
uint8_t setCrc(uint8_t *buff, uint8_t size);

/**
 * Set the start data in the buffer for the RDSS communication.
 *
 * @param rdss Pointer to the RDSS structure
 * @param buffer Pointer to the buffer to store the start data
 * @param function Function to be performed by the RDSS
 * @return Number of bytes added for the start data
 */
uint8_t setRdssStartData(RDSS_t *rdss, uint8_t *buffer, Function_t function);

/**
 * Decode the frequency value from the buffer.
 *
 * @param buffer Pointer to the buffer containing the frequency value
 * @return Decoded frequency value
 */
int freqDecode(uint8_t *buffer);

/**
 * Encode the frequency value and store it in the buffer.
 *
 * @param buffer Pointer to the buffer to store the encoded frequency value
 * @param freqIn Frequency value to be encoded
 */
void freqEncode(uint8_t *buffer, uint32_t freqIn);

/**
 * Update the RDSS structure with the received buffer and its size.
 *
 * @param rdss Pointer to the RDSS structure
 * @param buffer Pointer to the received buffer
 * @param bufferSize Size of the received buffer
 */
void updateRdss(RDSS_t *rdss, uint8_t *buffer, uint8_t bufferSize);
#endif /* INC_RS485_H_ */
