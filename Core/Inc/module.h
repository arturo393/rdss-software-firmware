/*
 * ltel.h
 *
 *  Created on: 27-09-2022
 *      Author: sigmadev
 */

#include "main.h"
#include "stdbool.h"
#include "math.h"


#ifndef INC_LTEL_H_
#define INC_LTEL_H_

#define MAX_TEMPERATURE 100
#define SAFE_TEMPERATURE 50
#define MAX_VSWR 1.7
#define SAFE_VSWR 1.0

//#define pa_on() SET_BIT(GPIOA->ODR,GPIO_ODR_OD3)
//#define pa_off() CLEAR_BIT(GPIOA->ODR,GPIO_ODR_OD3)
//#define pa_state()  READ_BIT(GPIOA->ODR,GPIO_ODR_OD3) ? 1 : 0

typedef enum MODULE_FUNCTION {
	SERVER,
	QUAD_BAND,
	PSU,
	TETRA,
	ULADR,
	VLADR,
	BDA,
	LOW_NOISE_AMPLIFIER,
	POWER_AMPLIFIER,
	UHF_TONE
} Function_t;

typedef enum MODULE_ID {
	ID0 = 0x00, ID1 = 0x01, ID2 = 0x02, ID8 = 0x08, ID9 = 0X09
} Id_t;

typedef enum MODULE_S{
	OFF,
	ON
}State_t;
typedef struct PA_MODULE {
	uint8_t att;
	uint8_t gain;
	int8_t pout;
	int8_t pr;
	uint8_t voltage;
	int8_t pin;
	uint16_t current;
	State_t enable;
	float  temperature;
	float temperature_out;
	float vswr;
	Id_t id;
	Function_t function;
	bool calc_en;
}  Module_pa_t;

typedef struct TONE_UHF_MODULE {
	unsigned long FreqOut;
	unsigned long FreqBase;
	unsigned long ON_OFF;
	uint8_t PdBm;
	Id_t id;
	Function_t function;
}  Tone_uhf_t;

union floatConverter {
	uint32_t i;
	float f;
};

typedef struct VLAD_MODULE {
	uint16_t agc152m;
	uint16_t ref152m;
	uint16_t level152m;  // downlink 150 mhz
	uint16_t agc172m;
	uint16_t level172m; //uplink 170 mhz
	uint16_t tone_level;
	uint16_t v_5v;
	uint16_t vin;
	uint16_t current;
	int8_t agc152m_real;
	int8_t agc172m_real;
	int8_t level152m_real;
	int8_t level172m_real;
	float v_5v_real;
	float lineVoltagereal;
	float lineCurrentReal;
	union floatConverter ucTemperature;
	union floatConverter lineCurrent;
	uint8_t remoteAttenuation;
	uint8_t rotarySwitchAttenuation;
	bool isRemoteAttenuation;
	bool is_attenuation_updated;
	Id_t id;
	Function_t function;
	bool calc_en;
	uint32_t lastUpdateTicks;
	uint8_t state;
} Vlad_t;

void module_init(Module_pa_t*,Function_t,Id_t);
void module_calc_parameters(Module_pa_t m,uint16_t* media_array);
void pa_sample_timer3_init();
void module_pa_state_update(Module_pa_t *pa);
void toneUhfInit(Function_t funcion, Id_t id, Tone_uhf_t *uhf);
Vlad_t* vladInit(uint8_t id);

#endif /* INC_LTEL_H_ */
