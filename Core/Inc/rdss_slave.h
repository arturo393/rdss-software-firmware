/*
 * rdss_slave.h
 *
 *  Created on: Aug 11, 2023
 *      Author: artur
 */

#ifndef INC_RDSS_SLAVE_H_
#define INC_RDSS_SLAVE_H_

#include "main.h"
#include "SX1278.h"
#include "led.h"
#include "uart1.h"
#include "module.h"
#include <string.h>
#include "spi.h"
#include "max4003.h"
#include "i2c.h"

#define QUERY_SIZE 34


typedef struct {
	uint8_t id;
	I2C_t *i2c;
	uint8_t query[QUERY_SIZE];
	uint32_t queryTicks;
} RDSS_SLAVE_t;


void print_parameters(UART1_t *u1, Vlad_t vlad);
void printStatus(UART1_t *u1, RDSS_t *rdss);
void printLoRaStatus(UART1_t *u1, SX1278_t *loRa);
void setQueryStatusBuffer(RDSS_t *rdss, Vlad_t *vlad);
void setQueryRdssBuffer(RDSS_t *rdss, Vlad_t *vlad);
void i2cSetQueryRdssBuffer(RDSS_t *rdss);
uint8_t executeCommand(uint8_t *buffer, RDSS_t *rdss, SX1278_t *loRa,
		Vlad_t *vlad);
void processUart1Rx(UART1_t *u1, RDSS_t *rdss, SX1278_t *loRa, Vlad_t *vlad);
void processCommand(SX1278_t *loRa, RDSS_t *rdss, Vlad_t *vlad);
void transmitStatus(SX1278_t *loRa, RDSS_t *rdss);
void slaveProcessRdss(RDSS_t *rdss, SX1278_t *loRa, Vlad_t *vlad);
void readWhenDataArrive(SX1278_t *loRa, RDSS_t *rdss, Vlad_t *vlad);
void updateQueryBuffer(RDSS_SLAVE_t *slave,uint32_t timeout);
HAL_StatusTypeDef getVlad(RDSS_SLAVE_t *slave, uint8_t querySize);


#endif /* INC_RDSS_SLAVE_H_ */


