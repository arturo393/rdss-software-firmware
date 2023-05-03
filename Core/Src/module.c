/*
 * ltel.c
 *
 *  Created on: 27-09-2022
 *      Author: sigmadev
 */

#include <module.h>

void pa_init(Function_t funcion, Id_t id, Module_pa_t *module) {
	module->function = funcion;
	module->id = id;
	module->att = 0;
	module->gain = 0;
	module->pin = 0;
	module->pout = 0;
	module->temperature = 0;
	module->enable = false;
	module->calc_en = true;
	pa_sample_timer3_init();
	pa_off();
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
	frame[index++] = (uint8_t) line_voltage;
	frame[index++] = (uint8_t) line_voltage >> 8;
	frame[index++] = (uint8_t) line_current;
	frame[index++] = (uint8_t) line_current >> 8;
	frame[index++] = (uint8_t) vladRev23Id;
	frame[index++] = (uint8_t) vladRev23Id >> 8;
	frame[index++] = (uint8_t) line_current;
	frame[index++] = (uint8_t) line_current >> 8;
	frame[index++] = (uint8_t) downlink_agc_value;
	frame[index++] = (uint8_t) vlad->level152m_real;
	frame[index++] = (uint8_t) uplink_agc_value;
	frame[index++] = (uint8_t) vlad->level172m_real;
	return index;
}

void module_init(Module_pa_t *module, Function_t funcion, Id_t id) {

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
	/*enable timer 3*/
	SET_BIT(TIM3->CR1, TIM_CR1_CEN);
	SET_BIT(TIM3->DIER, TIM_DIER_UIE);
	NVIC_EnableIRQ(TIM3_IRQn);
	CLEAR_BIT(TIM3->SR, TIM_SR_UIF);
}

void module_pa_state_update(Module_pa_t *pa) {
	if (pa->enable == ON) {
		if (pa->temperature_out > MAX_TEMPERATURE)
			pa_off();
		if (pa->temperature_out < SAFE_TEMPERATURE) {
			pa_on();
			if (pa->vswr > MAX_VSWR)
				pa_off();
			if (pa->vswr < MAX_VSWR)
				pa_on();
		}
	}
	if (pa->enable == OFF)
		pa_off();
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
