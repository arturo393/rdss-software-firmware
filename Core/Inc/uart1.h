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

#define RX_BUFFLEN 25
#define TX_BUFFLEN  100
#define SECONDS(x) x*1000

typedef struct UART1 {
	uint8_t rxBuffer[RX_BUFFLEN];
	uint8_t txBuffer[TX_BUFFLEN];
	uint8_t rxCount;
	uint32_t timeout;
} UART1_t;

uint8_t  uart1_clean_by_timeout(UART1_t* uart1,const char* str);
void uart1Init(uint32_t, uint32_t, UART1_t*);
void uart1_write(volatile char);
void uart1_send_str( char*);
void uart1_send_frame(uint8_t[], uint8_t);
void uart1_read(char*, uint8_t);
void uart1_read_test(char*, uint8_t);
uint8_t uart1_1byte_read(void);
void uart1_read_to_frame(UART1_t *u);
void uart1_clean_buffer(UART1_t*);
uint8_t uart1_nonblocking_read(void);

#endif /* INC_UART1_H_ */
