/*
 * i2c.h
 *
 *  Created on: Aug 16, 2022
 *      Author: sigmadev
 */

#include "main.h"
#include <stdbool.h>

#ifndef INC_I2C1_H_
#define INC_I2C1_H_

#define I2C_WRITE 0
#define I2C_READ 1
#define I2C1_OWN_ADDRESS 0x5C
#define I2C_TIMEOUT_MS 1000
#define I2C_RX_BUFFER_SIZE 20
#define I2C_TX_BUFFER_SIZE 20
#define I2C_BUSY_TIMEOUT	5000
#define I2C_SET_ATTENUATION 0x11
#define I2C_QUERY_VLAD_PARAMETERS 0x21


typedef enum State{
	WAITING_DATA,
	RX_DATA_COMPLETE,
	CRC_OK
}I2CState;

typedef struct I2C{
uint8_t tx[I2C_TX_BUFFER_SIZE];
uint8_t buffer[I2C_RX_BUFFER_SIZE];
uint8_t index;
uint8_t tx_count;
I2CState state;
uint32_t ticks;
uint32_t crc_value;
}I2C_t;

void i2c1MasterInit();
//void i2cMasterInit(I2C_TypeDef *i2c);
void i2c1SlaveInit(I2C_t *i2c);
void i2c1GpioInit();
void i2c2GpioInit();
bool  i2c1MasterStartTransfer(uint8_t,uint8_t,uint8_t);
char i2c1MasterByteRx(char ,uint8_t);
void i2c1MasterByteTx(uint8_t,uint8_t*,uint8_t);
void  i2c1AddresScannerold(uint8_t *addr,uint8_t max_addr);
//uint8_t  i2c1MasterFrameRx2(uint8_t saddr, uint8_t *rcv,  uint8_t N);
bool  i2c1MasterFrameRx(uint8_t saddr, uint8_t *rcv,  uint8_t N);
uint8_t i2c1SlaveTx2(I2C_t *i2c);
uint8_t i2c1SlaveRx(I2C_t *i2c);
void i2cCleanBuffer(I2C_t *i2c);
uint8_t isCrcValid(I2C_t *i2c);
void i2c1SlaveDelayedReset(I2C_t *i2c_slave, uint32_t timeout);
bool i2c1SlaveAddrsMatch(I2C_t *i2c_slave);
void i2c1SlaveTx(const I2C_t *i2c_slave);
void i2c1SalveRx(I2C_t *i2c_slave);
bool i2c1MasterTransmit(uint8_t slave_addr, uint8_t *data, uint8_t len, uint32_t timeout);
bool i2c1MasterReceive(uint8_t slave_addr, uint8_t *data, uint8_t len, uint32_t timeout);
bool i2c1AddresScanner(uint8_t *addresses, uint8_t *address_count);

#endif /* INC_I2C1_H_ */
