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

#include "main.h"
#include "string.h"
#include <stdbool.h>

extern I2C_HandleTypeDef hi2c1;

#define CHIP_ADDR 0x50
#define PAGE_SIZE 16
#define PAGE_NUM 32
#define IS_READY 0xaa
#define PADDRPOSITION 3
#define M24C64_CHIP_ADDR 0xA0

// EEPROM ADDRESS (8bits)
#define EEPROM_ADDR 0x50
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

#define M24C64_PAGE0_START_ADDR     0x0000
#define M24C64_PAGE0_END_ADDR       0x000F

#define M24C64_PAGE1_START_ADDR     0x0010
#define M24C64_PAGE1_END_ADDR       0x001F

#define M24C64_PAGE2_START_ADDR     0x0020
#define M24C64_PAGE2_END_ADDR       0x002F

/* Define labels for each page */
#define M24C64_PAGE0                0
#define M24C64_PAGE1                1
#define M24C64_PAGE2                2
/* ... */
#define M24C64_PAGE510              510
#define M24C64_PAGE511              511


void m24c64_page_read(uint8_t address,uint8_t page, uint8_t *data);
//bool readPage(uint8_t page, uint8_t *data, uint8_t offset,uint8_t size);
void savePage(uint8_t page, uint8_t *data, uint8_t offset,uint8_t size);
HAL_StatusTypeDef HAL_savePage(uint16_t page, uint8_t *data, uint16_t offset,uint16_t size);
HAL_StatusTypeDef readPage(uint16_t page, uint8_t *data, uint16_t offset, uint16_t size);
unsigned long getULFromEeprom(uint8_t page);


#ifdef __cplusplus
}
#endif

#endif /* M24C64_H */
