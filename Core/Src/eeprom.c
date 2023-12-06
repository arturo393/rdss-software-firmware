#include "eeprom.h"

void m24c64_page_read(uint8_t address, uint8_t page, uint8_t *data) {
	uint8_t buff[2] = { 0 };
	uint16_t MemAddress = page << PADDRPOSITION;

	buff[0] = MemAddress >> 8;
	buff[1] = MemAddress & 0xff;

	i2c1MasterByteTx(CHIP_ADDR, buff, 2);
	i2c1MasterFrameRx(CHIP_ADDR, data, 32);
}

/*
bool readPage(uint8_t page, uint8_t *data, uint8_t offset, uint8_t size) {
	uint8_t buff[1] = { 0 };
	uint16_t MemAddress = page << PADDRPOSITION | offset;
	buff[0] = (uint8_t) MemAddress & 0xff;

	if (!i2c1MasterTransmit(CHIP_ADDR, buff, 1, 1000))
		return false;
	HAL_Delay(5);
	if (!i2c1MasterReceive(CHIP_ADDR, data, size, 1000))
		return false;
	return true;
}
*/

void savePage(uint8_t page, uint8_t *data, uint8_t offset, uint8_t size) {
	uint8_t buff[16 + 1];
	uint8_t read[16];
	uint8_t i = 0;

	readPage(page, read, offset, size);
	bool notEqual = false;

	for (i = 0; i < size; i++)
		if (data[i] != read[i]) {
			notEqual = true;
			break;
		}

	if (notEqual) {
		buff[0] = (uint8_t) (page << PADDRPOSITION | offset) & 0xff;
		for (i = 0; i < size; i++) {
			buff[i + 1] = data[i];
		}
		i2c1MasterTransmit(CHIP_ADDR, buff, size + 1, 50);
	}
	HAL_Delay(6);
}

HAL_StatusTypeDef readPage(uint16_t page, uint8_t *data, uint16_t offset, uint16_t size) {
    uint16_t MemAddress = (page << 8) | offset;
    HAL_StatusTypeDef res;
    res = HAL_I2C_Mem_Read(&hi2c1, M24C64_CHIP_ADDR, MemAddress, I2C_MEMADD_SIZE_16BIT, data, size, 1000);
    if (res != HAL_OK)
        return res;

    HAL_Delay(5);
    return res;
}


HAL_StatusTypeDef HAL_savePage(uint16_t page, uint8_t *data, uint16_t offset, uint16_t size) {
    uint8_t read[16]={0};
    bool notEqual = false;
    HAL_StatusTypeDef res;

    res = readPage(page, read, offset, size);

    for (uint16_t i = 0; i < size; i++)
        if (data[i] != read[i]) {
            notEqual = true;
            break;
        }

    if (notEqual) {
        uint16_t memAddress = (page << 8) | offset;
        res = HAL_I2C_Mem_Write(&hi2c1, M24C64_CHIP_ADDR, memAddress, I2C_MEMADD_SIZE_16BIT, data, size, 50);
    }
    HAL_Delay(6);

    return res;
}


unsigned long getULFromEeprom(uint8_t page) {
	//uint8_t size = sizeof(unsigned long);
	uint8_t buffer[4] = { 0 };
	unsigned long readValue = 0;
	readPage(page, buffer, 0, 4);
	for (int i = 0; i < 4; i++) {
		readValue |= (buffer[i] << ((i) * 8));
	}
	return readValue;
}
