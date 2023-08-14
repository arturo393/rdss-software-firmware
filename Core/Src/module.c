/*
 * ltel.c
 *
 *  Created on: 27-09-2022
 *      Author: Arturo Veras
 *      email: arturo@uqomm.com
 */

#include <module.h>


Vlad_t* vladInit(uint8_t id) {
	Vlad_t *vlad;
	vlad = malloc(sizeof(Vlad_t));

	vlad->tone_level = 0;
	vlad->v_5v_real = 0;
	vlad->inputVoltageReal = 0;
	vlad->currentReal = 0;
	vlad->ucTemperature = 0;
	vlad->baseCurrentReal = 0;
	vlad->remoteAttenuation = 0;
	vlad->v_5v_real = 0;
	vlad->inputVoltageReal = 0;
	vlad->currentReal = 0;
	vlad->agc152m_real = 0;
	vlad->agc172m_real = 0;
	vlad->level152m_real = 0;
	vlad->level172m_real = 0;
	vlad->baseCurrentReal = 0;
	vlad->remoteAttenuation = 0;
	vlad->rotarySwitchAttenuation = 0;
	vlad->isRemoteAttenuation = false;
	vlad->is_attenuation_updated = false;
	vlad->state = 0;
	vlad->calc_en = false;
	vlad->function = VLADR;
	vlad->id = id;
	vlad->lastUpdateTicks = HAL_GetTick();

	readPage(CAT24C02_PAGE0_START_ADDR, &(vlad->function), 3, 1);
	readPage(CAT24C02_PAGE0_START_ADDR, &(vlad->id), 4, 1);
	return vlad;
}


uint8_t encode16u(uint8_t *buffer) {

	uint16_t temp = (uint16_t) buffer[0] | (uint16_t) (buffer[1] << 8);

	return temp;
}

uint8_t decodeVladMeasurements(Vlad_t *vlad, uint8_t *buffer) {
	uint8_t bufferIndex = 0;
	typedef enum measurements {
		vin,
		v_5v,
		current,
		agc152m,
		ref152m,
		level152m,
		agc172m,
		ref172m,
		level172m,
		tono_level,
		ucTemperature,
		baseCurrent,
		num_variables
	} measurements_t;

	uint16_t measurement[num_variables];
	for (int i = 0; i < num_variables; i++) {
		measurement[i] = (uint16_t) buffer[bufferIndex++];
		measurement[i] |= (uint16_t) (buffer[bufferIndex++] << 8);
	}
	vlad->state = buffer[bufferIndex++];
	uint8_t temperature;
	temperature = (uint8_t) ((float) measurement[ucTemperature]
			- (float) (*TEMP30_CAL_ADDR)) * (110.0 - 30.0)
			/ (float) (*TEMP110_CAL_ADDR - *TEMP30_CAL_ADDR);
	vlad->ucTemperature = temperature + 30.0;
	vlad->v_5v_real = (float) measurement[v_5v] * ADC_V5V_FACTOR;
	vlad->inputVoltageReal = (float) measurement[vin] * ADC_VOLTAGE_FACTOR;
	vlad->currentReal = measurement[current] * ADC_CONSUMPTION_CURRENT_FACTOR;
	vlad->agc152m_real = (int8_t) (MAX4003_AGC_SCOPE * measurement[agc152m]
			+ MAX4003_AGC_FACTOR);
	vlad->agc172m_real = (int8_t) (MAX4003_AGC_SCOPE * measurement[agc172m]
			+ MAX4003_AGC_FACTOR);
	vlad->level152m_real = (int8_t) (MAX4003_DBM_SCOPE * measurement[level152m]
			+ MAX4003_DBM_FACTOR);
	vlad->level172m_real = (int8_t) (MAX4003_DBM_SCOPE * measurement[level172m]
			+ MAX4003_DBM_FACTOR);
	vlad->baseCurrentReal = (measurement[baseCurrent] * 1000 * VREF)
			/ (1 << (RESOLUTION - 0x00));

	return bufferIndex;
}
