#include "module.h"

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

Vlad_t* vladInit(Function_t function) {
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
	vlad->inputVoltageReal = 0;
	vlad->currentReal = 0;
	vlad->ucTemperature = 0;
	vlad->baseCurrentReal = 0;
	vlad->attenuation = 0;
	vlad->v_5v_real = 0;
	vlad->inputVoltageReal = 0;
	vlad->currentReal = 0;
	vlad->agc152m_real = 0;
	vlad->agc172m_real = 0;
	vlad->level152m_real = 0;
	vlad->level172m_real = 0;
	vlad->isRemoteAttenuation = false;
	vlad->is_attenuation_updated = false;
	vlad->state = 0;
	vlad->calc_en = false;
	vlad->function = function;
	vlad->id = 0;
	vlad->lastUpdateTicks = HAL_GetTick();

//	HAL_readPage(CAT24C02_PAGE0_START_ADDR, &(vlad->function), 3, 1);
//	HAL_readPage(CAT24C02_PAGE0_START_ADDR, &(vlad->id), 4, 1);
	return vlad;
}

Server_t* serverInit(Function_t function) {
	Server_t *server;
	server = malloc(sizeof(Server_t));

	if (server != NULL) {
        server->inputVoltage = 0;
        server->counsumptionCurrentReal = 0;
        server->lm75Temperature = 0;
        server->id = 0;  // assuming id can be zero initialized
        server->function = function;  // Use the function parameter passed in
        server->lastUpdateTicks = 0;
	}
	return server;
}

uint8_t encodeVladToLtel(uint8_t *frame, Vlad_t *vlad) {
	if(vlad == NULL)
		return 0;
	uint8_t data_length = 12;
	uint8_t index = 0;
	uint16_t line_voltage = (uint16_t) (vlad->inputVoltageReal * 10);
	uint16_t current = (vlad->currentReal * 1000);
	uint16_t baseCurrent = vlad->baseCurrentReal * 1000;
	uint16_t tunnelCurrent = 0;
	uint8_t downlink_agc_value = (uint8_t) (vlad->agc152m_real * 10);
	uint8_t uplink_agc_value = (uint8_t) (vlad->agc172m_real * 10);
	uint8_t vladRev23Id = 0xff;

	frame[index++] = data_length;
	frame[index++] = (uint8_t) line_voltage;
	frame[index++] = (uint8_t) (line_voltage >> 8);
	frame[index++] = (uint8_t) baseCurrent;
	frame[index++] = (uint8_t) (baseCurrent >> 8);
	frame[index++] = (uint8_t) tunnelCurrent;
	frame[index++] = (uint8_t) (tunnelCurrent >> 8);
	frame[index++] = (uint8_t) current;
	frame[index++] = (uint8_t) (current >> 8);
	frame[index++] = (uint8_t) uplink_agc_value;
	frame[index++] = (uint8_t) vlad->level152m_real;
	frame[index++] = (uint8_t) downlink_agc_value;
	frame[index++] = (uint8_t) vlad->level172m_real;
	return index;
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

uint8_t decodeVladMeasurements(Vlad_t *vlad, uint8_t *buffer) {
	uint8_t bufferIndex = 0;
	typedef enum measurements {
		level152m,
		level172m,
		agc152m,
		agc172m,
		ref152m,
		tone_level,
		vin,
		v_5v,
		current,
		baseCurrent,
		num_variables
	} measurements_t;

	uint16_t measurement[num_variables];
	for (int i = 0; i < num_variables; i++) {
		measurement[i] = (uint16_t) buffer[bufferIndex++];
		measurement[i] |= (uint16_t) (buffer[bufferIndex++] << 8);
	}
	vlad->state = buffer[bufferIndex++];

	vlad->isReverse = vlad->state & 0x01;
	vlad->isSmartTune = (vlad->state >> 1) & 0x01;
	vlad->isRemoteAttenuation = (vlad->state >> 2) & 0x01;
	vlad->attenuation = (vlad->state >> 3) & 0x0F;
	vlad->ucTemperature = buffer[bufferIndex++];

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
