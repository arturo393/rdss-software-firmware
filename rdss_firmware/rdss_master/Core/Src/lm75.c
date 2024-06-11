#include "lm75.h"


HAL_StatusTypeDef lm75Init(I2C_HandleTypeDef *i2c) {
	uint8_t cmd[2];
	uint8_t len;
	cmd[0] = LM75_Conf;
	cmd[1] = 0x0;
	len = sizeof(cmd);
	return (HAL_I2C_Master_Transmit(i2c, LM75_ADDR << 1, cmd, len,
	HAL_MAX_DELAY));
}

uint8_t lm75Read(I2C_HandleTypeDef *i2c) {
	uint8_t cmd[2];
	HAL_StatusTypeDef res;
	cmd[0] = LM75_Temp;
	res = HAL_I2C_Master_Transmit(i2c, LM75_ADDR << 1, cmd, 1,
	HAL_MAX_DELAY);
	if (res != HAL_OK)
		return (0);
	HAL_Delay(1);
	HAL_I2C_Master_Receive(i2c, LM75_ADDR << 1 | 1, cmd, 2,
	HAL_MAX_DELAY);
	if (res != HAL_OK)
		return (0);
	return (((cmd[0] << 8) | cmd[1]) / 256.0f);
}
