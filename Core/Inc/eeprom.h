/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : m24c64.h
 * @brief          : Header for m24c64.c file.
 *                   This file contains the common defines of the application.
 ******************************************************************************
 *
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __M24C64_H
#define M24C64_H

#ifdef __cplusplus
extern "C" {
#endif


#include "main.h"
#include <i2c.h>
#include "string.h"

#define CHIP_ADDR 0xa0
#define PAGE_SIZE 16
#define PAGE_NUM 32
#define IS_READY 0xaa
#define PADDRPOSITION 3

// EEPROM ADDRESS (8bits)
#define EEPROM_ADDR 0xA0
//#define EEPROM_ADDR 0x50

#define BASE_ADDR 0x03

#define CAT24C02_PAGE0_START_ADDR   0x0000
#define CAT24C02_PAGE0_END_ADDR     0x000F

#define CAT24C02_PAGE1_START_ADDR   0x0010
#define CAT24C02_PAGE1_END_ADDR     0x001F

#define CAT24C02_PAGE2_START_ADDR   0x0020
#define CAT24C02_PAGE2_END_ADDR     0x002F

/* Define labels for each page */
#define CAT24C02_PAGE0              0
#define CAT24C02_PAGE1              1
#define CAT24C02_PAGE2				2
#define CAT24C02_PAGE3              3
#define CAT24C02_PAGE4              4
#define CAT24C02_PAGE5				5
#define CAT24C02_PAGE6              6
#define CAT24C02_PAGE7              7
#define CAT24C02_PAGE8				8

/*
@page is the number of the start page. Range from 0 to PAGE_NUM-1
@offset is the start byte offset in the page. Range from 0 to PAGE_SIZE-1
@data is the pointer to the data to write in bytes
@size is the size of the data
*/
typedef enum{
	FREQ_OUT_ADDR = BASE_ADDR,
	FREQ_BASE_ADDR,
	POUT_ADDR,
	MODE_ADDR
}M24C64_ADDR_t;


typedef struct M24C64 {
	M24C64_ADDR_t  addrs;
	 uint8_t data[PAGESIZE];
} M24C64_t;


void m24c64_page_read(uint8_t address,uint8_t page, uint8_t *data);
uint8_t readPage(uint8_t page, uint8_t *data, uint8_t offset,uint8_t size);

void savePage(uint8_t page, uint8_t *data, uint8_t offset,uint8_t size);
void m24c64_init_16uvalue(M24C64_ADDR_t addr,uint16_t value);
void saveU16(M24C64_ADDR_t addr,uint16_t value);
unsigned long getULFromEeprom(uint8_t page);


#ifdef __cplusplus
}
#endif

#endif /* M24C64_H */
