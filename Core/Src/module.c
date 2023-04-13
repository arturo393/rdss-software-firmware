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

void vladInit(Function_t funcion, Id_t id, Vlad_t *vlad) {
	vlad->agc150m = 0;
	vlad->ref150m = 0;
	vlad->level150m = 0;  // downlink 150 mhz
	vlad->agc170m = 0;
	vlad->ref170m = 0;
	vlad->level170m = 0; //uplink 170 mhz
	vlad->tone_level = 0;
	vlad->v_5v = 0;
	vlad->vin = 0;
	vlad->current = 0;
	vlad->v_5v_real = 0;
	vlad->vin_real = 0;
	vlad->current_real = 0;
	vlad->uc_temperature = 0;
	vlad->remote_attenuation = 0;
	vlad->is_remote_attenuation = false;
	vlad->is_attenuation_updated = false;
	vlad->calc_en = false;
	vlad->function = funcion;
	vlad->id = id;
}

void encodeVLAD(uint8_t *frame, Id_t id) {
	uint8_t crc_frame[2];
	uint16_t crc;
	uint8_t data_length = 12;

	frame[0] = LTEL_START_MARK;
	frame[1] = VLADR;
	frame[2] = id;
	frame[3] = 0x11;
	frame[4] = 0x00;
	frame[5] = data_length;
	frame[6] = 0x00;
	frame[7] = rand() % 10;
	frame[8] = rand() % 256;
	frame[9] = rand() % 256;
	frame[10] = rand() % 256;
	frame[11] = rand() % 256;
	frame[12] = rand() % 256;
	frame[13] = rand() % 256;
	frame[14] = rand() % 256;
	frame[15] = rand() % 256;
	frame[16] = rand() % 256;
	frame[17] = rand() % 256;

	crc = crc_get(&(frame[1]), 17);
	memcpy(crc_frame, &crc, 2);
	frame[18] = crc_frame[0];
	frame[19] = crc_frame[1];
	frame[20] = LTEL_END_MARK;
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

