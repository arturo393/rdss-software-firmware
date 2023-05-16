/*
 * ltel.c
 *
 *  Created on: 27-09-2022
 *      Author: sigmadev
 */


#include <eeprom.h>

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
	//SET_BIT(GPIOA->MODER, GPIO_MODER_MODE3_0);
	//CLEAR_BIT(GPIOA->MODER, GPIO_MODER_MODE3_1);
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
	vlad->lineVoltagereal = 0;
	vlad->lineCurrentReal = 0;
	vlad->ucTemperature.i = 0;
	vlad->lineCurrent.i = 0;
	vlad->remoteAttenuation = 0;
	vlad->v_5v_real = 0;
	vlad->lineVoltagereal = 0;
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

	HAL_readPage(CAT24C02_PAGE0_START_ADDR, &(vlad->function), 3, 1);
	HAL_readPage(CAT24C02_PAGE0_START_ADDR, &(vlad->id), 4, 1);
	return vlad;
}


void module_init(Module_pa_t *module, Function_t funcion, Id_t id) {

}

void pa_sample_timer3_init() {
	/*enable clock access to timer 2 */
	//SET_BIT(RCC->APBENR1, RCC_APBENR1_TIM3EN);
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

