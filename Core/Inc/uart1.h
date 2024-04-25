/*
 * uart1.h
 *
 *  Created on: Aug 29, 2022
 *      Author: sigmadev
 */

#ifndef INC_UART1_H_
#define INC_UART1_H_
#include "stm32f103xb.h"
#include "stm32f1xx.h"
#include "stdbool.h"
#include "stdlib.h"
#include "string.h"

#define UART2_RX_BUFFLEN 300
#define TX_BUFFLEN  35
#define SECONDS(x) x*1000

typedef struct UART1 {
	uint8_t rxData[UART2_RX_BUFFLEN];
	uint8_t *txData;
	uint8_t txSize;
	uint8_t rxSize;
	uint32_t operationTimeout;
	bool isReceivedDataReady;
	bool isDebugModeEnabled;
} UART1_t;

uint8_t  cleanByTimeout(UART1_t* uart1,const char* str);
void uart1Init(uint32_t, uint32_t, UART1_t*);
void writeTxReg(volatile char);
void writeTxStr( char*);
void writeTxBuffer(uint8_t[], uint8_t);
void writeTx(UART1_t *uart1);
void uart1_read(char*, uint8_t);
uint8_t readRxReg(void);
void readRx(UART1_t *u);
void cleanRx(UART1_t *u);
void cleanTx(UART1_t *u);

#endif /* INC_UART1_H_ */
