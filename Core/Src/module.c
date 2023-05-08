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
	vlad->vin_real = 0;
	vlad->current_real = 0;
	vlad->uc_temperature.i = 0;
	vlad->remote_attenuation = 0;
	vlad->is_remote_attenuation = false;
	vlad->is_attenuation_updated = false;
	vlad->calc_en = false;
	vlad->function = VLADR;
	vlad->id = id;
	vlad->ticks = HAL_GetTick();
	return vlad;
}

uint8_t encodeVladToLtel(uint8_t *frame, Vlad_t *vlad) {
	uint8_t data_length = 12;
	uint8_t index = 0;
	uint16_t line_voltage = (uint16_t) vlad->vin_real * 10;
	uint16_t line_current = (uint16_t) vlad->current_real * 1000;
	uint8_t downlink_agc_value = (uint8_t) (vlad->agc152m_real * 10);
	uint8_t uplink_agc_value = (uint8_t) (vlad->agc172m_real * 10);
	uint8_t vladRev23Id = 0xff;
	frame[index++] = data_length;
	frame[index++] = (uint8_t) vladRev23Id;
	frame[index++] = (uint8_t) vladRev23Id >> 8;
	frame[index++] = (uint8_t) line_voltage;
	frame[index++] = (uint8_t) line_voltage >> 8;
	frame[index++] = (uint8_t) line_current;
	frame[index++] = (uint8_t) line_current >> 8;
	frame[index++] = (uint8_t) vlad->remote_attenuation;
	frame[index++] = (uint8_t) vlad->remote_attenuation >> 8;
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

void vladReset(Vlad_t *vlad) {
	vlad->level152m = 0;
	vlad->level172m = 0;
	vlad->agc152m = 0;
	vlad->agc172m = 0;
	vlad->ref152m = 0;
	vlad->tone_level = 0;
	vlad->vin = 0;
	vlad->v_5v = 0;
	vlad->current = 0;
	vlad->uc_temperature.i = 0;
}

uint8_t queryVladStatus(Vlad_t *vlad) {
	uint8_t slave_address = 0x08;
	uint8_t query = 0x10;
	uint8_t query_size = 23;
	uint8_t index = 0;
	uint8_t buffer[23];

	if (!i2c1MasterTransmit(slave_address, &query, sizeof(query), 200))
		return index;
	HAL_Delay(6);
	if (!i2c1MasterReceive(slave_address, buffer, query_size, 2000))
		return index;

	uint16_t *vlad_values[] = { &vlad->level152m, &vlad->level172m,
			&vlad->agc152m, &vlad->agc172m, &vlad->ref152m, &vlad->tone_level,
			&vlad->vin, &vlad->v_5v, &vlad->current, &vlad->remote_attenuation };
	size_t num_values = sizeof(vlad_values) / sizeof(vlad_values[0]);

	for (size_t i = 0; i < num_values; i++) {
		*vlad_values[i] = (uint16_t) buffer[index++];
		*vlad_values[i] |= (uint16_t) (buffer[index++] << 8);
	}

	vlad->uc_temperature.i = buffer[index++];
	vlad->uc_temperature.i |= buffer[index++] << 8;
	vlad->uc_temperature.i |= buffer[index++] << 16;
	vlad->uc_temperature.i |= buffer[index++] << 24;

	return index;
}

void updateVladData(Vlad_t *vlad) {
	const uint32_t vladReadInterval = VLAD_READ_TIMER;
	uint8_t query[2];
	uint8_t slave_address = 0x08;

	if (HAL_GetTick() - vlad->ticks > vladReadInterval) {
		if (queryVladStatus(vlad) > 0) {
			vlad->v_5v_real = (float) vlad->v_5v * ADC_V5V_FACTOR;
			vlad->vin_real = (float) vlad->vin * ADC_VOLTAGE_FACTOR;
			vlad->current_real =
					(float) vlad->current * ADC_LINE_CURRENT_FACTOR;
			vlad->agc152m_real = (int8_t) (MAX4003_AGC_SCOPE * (float) vlad->agc152m + MAX4003_AGC_FACTOR);
			vlad->agc172m_real = (int8_t) (MAX4003_AGC_SCOPE * (float) vlad->agc172m + MAX4003_AGC_FACTOR);
			vlad->level152m_real =  (int8_t) (MAX4003_AGC_SCOPE * (float) vlad->level152m + MAX4003_AGC_FACTOR);
			vlad->level172m_real =  (int8_t) (MAX4003_DBM_SCOPE * (float) vlad->level172m + MAX4003_DBM_FACTOR);
			vladReset(vlad);
		}
		vlad->ticks = HAL_GetTick();

		query[0] = SET_VLAD_ATTENUATION;
		query[1] = (HAL_GetTick() & 0xFF) % 31;
		i2c1MasterTransmit(slave_address, query, sizeof(query), 200);
	}
}
