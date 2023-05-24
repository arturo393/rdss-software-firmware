/*
 * ltel.c
 *
 *  Created on: 27-09-2022
 *      Author: sigmadev
 */

#include <module.h>

void pa_init(Function_t funcion, Id_t id, PA_t *module) {
	module->function = funcion;
	module->id = id;
	module->att = 0;
	module->gain = 0;
	module->pin = 0;
	module->pout = 0;
	module->temperature = 0;
	module->isEnable = false;
	module->calc_en = true;
	pa_sample_timer3_init();
	powerAmplifierOff();
	/* PA3  PA_HAB as output - ENABLE - DISABLE PA */
	SET_BIT(GPIOA->MODER, GPIO_MODER_MODE3_0);
	CLEAR_BIT(GPIOA->MODER, GPIO_MODER_MODE3_1);
}

void toneUhfInit(Function_t funcion, Id_t id, Tone_uhf_t *uhf) {
	uhf->ON_OFF = 0;
	uhf->FreqBase = 0;
	uhf->FreqOut = 0;
	uhf->PdBm = 0;
	uhf->function = funcion;
	uhf->id = id;
}

Vlad_t* vladInit(uint8_t id) {
	Vlad_t *vlad;
	vlad = malloc(sizeof(Vlad_t));
	vlad->agc152m = 0;
	vlad->ref152m = 0;
	vlad->level152m = 0;  // downlink 150 mhz
	vlad->agc172m = 0;
	vlad->level172m = 0; //uplink 170 mhz
	vlad->tone_level = 0;
	vlad->v_5v = 0;
	vlad->vin = 0;
	vlad->current = 0;
	vlad->v_5v_real = 0;
	vlad->lineVoltageReal = 0;
	vlad->lineCurrentReal = 0;
	vlad->ucTemperature.i = 0;
	vlad->lineCurrent.i = 0;
	vlad->remoteAttenuation = 0;
	vlad->v_5v_real = 0;
	vlad->lineVoltageReal = 0;
	vlad->lineCurrentReal = 0;
	vlad->agc152m_real = 0;
	vlad->agc172m_real = 0;
	vlad->level152m_real = 0;
	vlad->level172m_real = 0;
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

uint8_t encodeVladToLtel(uint8_t *frame, Vlad_t *vlad) {
	uint8_t data_length = 12;
	uint8_t index = 0;
	uint16_t line_voltage = (uint16_t) (vlad->lineVoltageReal * 10);
	uint16_t line_current = (uint16_t) (vlad->lineCurrent.f * 1000);
	uint8_t downlink_agc_value = (uint8_t) (vlad->agc152m_real * 10);
	uint8_t uplink_agc_value = (uint8_t) (vlad->agc172m_real * 10);
	uint8_t vladRev23Id = 0xff;

	frame[index++] = data_length;
	frame[index++] = (uint8_t) vladRev23Id;
	frame[index++] = (uint8_t) vlad->state;
	frame[index++] = (uint8_t) line_voltage;
	frame[index++] = (uint8_t) (line_voltage >> 8);
	frame[index++] = (uint8_t) line_current;
	frame[index++] = (uint8_t) (line_current >> 8);
	frame[index++] = (uint8_t) vlad->tone_level;
	frame[index++] = (uint8_t) (vlad->tone_level >> 8);
	frame[index++] = (uint8_t) downlink_agc_value;
	frame[index++] = (uint8_t) vlad->level152m_real;
	frame[index++] = (uint8_t) uplink_agc_value;
	frame[index++] = (uint8_t) vlad->level172m_real;
	return index;
}

void pa_sample_timer3_init() {
	/*enable clock access to timer 2 */
	SET_BIT(RCC->APBENR1, RCC_APBENR1_TIM3EN);
	/*set preescaler value */
	TIM3->PSC = 6400 - 1; // 64 000 000 / 64 00 = 1 000 000
	/* set auto-reload */
	TIM3->ARR = 10000 - 1; // 1 000  000 /
	SET_BIT(TIM3->CR1, TIM_CR1_ARPE);
	/* clear counter */
	TIM3->CNT = 0;
	/*isEnable timer 3*/
	SET_BIT(TIM3->CR1, TIM_CR1_CEN);
	SET_BIT(TIM3->DIER, TIM_DIER_UIE);
	NVIC_EnableIRQ(TIM3_IRQn);
	CLEAR_BIT(TIM3->SR, TIM_SR_UIF);
}

void updatePAState(PA_t *modulePA) {
	const uint8_t maxTemp = 100;
	const uint8_t safeTemp = 50;
	const float maxVSWR = 1.7f;
	const float minVSWR = 1.0f;
	if (modulePA->isEnable == ON) {
		if (modulePA->outsideTemp > maxTemp) {
			powerAmplifierOff();
		} else if (modulePA->outsideTemp < safeTemp) {
			powerAmplifierOn();
			if (modulePA->vswr > maxVSWR) {
				powerAmplifierOff();
			} else if (modulePA->vswr < minVSWR) {
				powerAmplifierOn();
			}
		}

	} else if (modulePA->isEnable == OFF) {
		powerAmplifierOff();
	}
}

float current_calc(uint16_t _current) {
	return ADC_CONSUMPTION_CURRENT_FACTOR * _current / 4095.0f;
}

void resetVladData(Vlad_t *vlad) {
	vlad->level152m = 0;
	vlad->level172m = 0;
	vlad->agc152m = 0;
	vlad->agc172m = 0;
	vlad->ref152m = 0;
	vlad->tone_level = 0;
	vlad->vin = 0;
	vlad->v_5v = 0;
	vlad->current = 0;
	vlad->ucTemperature.i = 0;
	vlad->lineCurrent.i = 0;
	vlad->remoteAttenuation = 0;
	vlad->rotarySwitchAttenuation = 0;
	vlad->is_attenuation_updated = false;
	vlad->v_5v_real = 0;
	vlad->lineVoltageReal = 0;
	vlad->lineCurrentReal = 0;
	vlad->agc152m_real = 0;
	vlad->agc172m_real = 0;
	vlad->level152m_real = 0;
	vlad->level172m_real = 0;
	vlad->state = 0;
}

uint8_t readVladMeasurements(Vlad_t *vlad) {
	uint8_t slaveAddress = 0x08;
	uint8_t vladMeasurementsCmd[2] = {0x10,0x00};
	uint8_t query_size = 30;
	uint8_t bufferIndex = 0;
	uint8_t buffer[30];

	if (!i2c1MasterTransmit(slaveAddress, vladMeasurementsCmd, sizeof(vladMeasurementsCmd), 10))
		return bufferIndex;
	HAL_Delay(6);
	if (!i2c1MasterReceive(slaveAddress, buffer, query_size, 10))
		return bufferIndex;

	uint16_t *vladValues[] = { &vlad->level152m, &vlad->level172m,
			&vlad->agc152m, &vlad->agc172m, &vlad->ref152m, &vlad->tone_level,
			&vlad->vin, &vlad->v_5v, &vlad->current };

	size_t num_values = sizeof(vladValues) / sizeof(vladValues[0]);

	for (size_t i = 0; i < num_values; i++) {
		*vladValues[i] = (uint16_t) buffer[bufferIndex++];
		*vladValues[i] |= (uint16_t) (buffer[bufferIndex++] << 8);
	}

	vlad->remoteAttenuation = buffer[bufferIndex++];
	vlad->rotarySwitchAttenuation = buffer[bufferIndex++];
	vlad->state = buffer[bufferIndex++];
	vlad->lineCurrent.i =buffer[bufferIndex++];
	vlad->lineCurrent.i |=buffer[bufferIndex++]<<8;
	vlad->lineCurrent.i |=buffer[bufferIndex++]<<16;
	vlad->lineCurrent.i |=buffer[bufferIndex++]<<24;
	vlad->ucTemperature.i = buffer[bufferIndex++];
	vlad->ucTemperature.i |= buffer[bufferIndex++] << 8;
	vlad->ucTemperature.i |= buffer[bufferIndex++] << 16;
	vlad->ucTemperature.i |= buffer[bufferIndex++] << 24;
	return bufferIndex;
}

void updateVladMeasurements(Vlad_t *vlad) {
	const uint32_t vladReadIntervalMs  = VLAD_READ_TIMER;


	if (HAL_GetTick() - vlad->lastUpdateTicks > vladReadIntervalMs ) {
		if (readVladMeasurements(vlad) > 0) {
			vlad->v_5v_real = (float) vlad->v_5v * ADC_V5V_FACTOR;
			vlad->lineVoltageReal = (float) vlad->vin * ADC_VOLTAGE_FACTOR;
			vlad->lineCurrentReal =
					(float) vlad->current * ADC_LINE_CURRENT_FACTOR;
			vlad->agc152m_real = (int8_t) (MAX4003_AGC_SCOPE
					* (float) vlad->agc152m + MAX4003_AGC_FACTOR);
			vlad->agc172m_real = (int8_t) (MAX4003_AGC_SCOPE
					* (float) vlad->agc172m + MAX4003_AGC_FACTOR);
			vlad->level152m_real = (int8_t) (MAX4003_DBM_SCOPE
					* (float) vlad->level152m + MAX4003_DBM_FACTOR);
			vlad->level172m_real = (int8_t) (MAX4003_DBM_SCOPE
					* (float) vlad->level172m + MAX4003_DBM_FACTOR);

		} else {
			resetVladData(vlad);
		}
		vlad->lastUpdateTicks = HAL_GetTick();
	}
}
