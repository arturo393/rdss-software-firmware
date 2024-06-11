#include <eeprom.h>

void m24c64_page_read(uint8_t address, uint8_t page, uint8_t *data) {
	uint8_t buff[2] = { 0 };
	uint16_t MemAddress = page << PADDRPOSITION;

	buff[0] = MemAddress >> 8;
	buff[1] = MemAddress & 0xff;

	i2c1MasterByteTx(CHIP_ADDR, buff, 2);
	i2c1MasterFrameRx(CHIP_ADDR, data, 32);
}

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

uint8_t savePage(uint8_t page, uint8_t *data, uint8_t offset, uint8_t size) {
	uint8_t buff[16 + 1];
	uint8_t read[16];
	uint8_t i = 0;
	HAL_StatusTypeDef res = 0;
	res = readPage(page, read, offset, size);
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
		res = i2c1MasterTransmit(CHIP_ADDR, buff, size + 1, 50);
	}
	HAL_Delay(6);

	return res;
}

void m24c64_init_16uvalue(M24C64_ADDR_t addr, uint16_t value) {
	uint8_t buff[2];
	readPage(BASE_ADDR, buff, addr, 1);
	if (!(buff[0] == IS_READY)) {
		buff[0] = value >> 8;
		buff[1] = value & 0xff;
		savePage(BASE_ADDR, buff, addr + 1, 2);
	}
}

void saveU16(M24C64_ADDR_t addr, uint16_t value) {
	uint8_t buff[2];
	buff[0] = value >> 8;
	buff[1] = value & 0xff;
	savePage(BASE_ADDR, buff, addr + 1, 2);
	buff[0] = addr;
	savePage(BASE_ADDR, buff, addr, 1);
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
