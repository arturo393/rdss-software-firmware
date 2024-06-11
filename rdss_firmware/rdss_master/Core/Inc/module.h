#include "main.h"
#include "stdbool.h"
#include "math.h"
#include "stdlib.h"
#include "eeprom.h"

#ifndef INC_LTEL_H_
#define INC_LTEL_H_


#define MAX_TEMPERATURE 100
#define SAFE_TEMPERATURE 50
#define MAX_VSWR 1.7
#define SAFE_VSWR 1.0

static const float ADC_CONSUMPTION_CURRENT_FACTOR = 0.06472492f;
static const float ADC_LINE_CURRENT_FACTOR = 0.0010989f;
static const float ADC_VOLTAGE_FACTOR = 0.01387755f;
static const float ADC_V5V_FACTOR = 0.00161246f;

#define VREF 5 // Reference voltage in volts
#define RESOLUTION 12 // ADC resolution in bits

#define MAX4003_VOLTAGE_MAX 4.2f
#define MAX4003_VOLTAGE_MIN 0.5f

#define MAX4003_DBM_MAX 0
#define MAX4003_DBM_MIN -30
#define MAX4003_AGC_MIN 30
#define MAX4003_AGC_MAX 0
#define MAX4003_ADC_MAX  1888.0f
#define MAX4003_ADC_MIN   487.0f

#define MAX4003_DBM_SCOPE ( MAX4003_DBM_MAX -  MAX4003_DBM_MIN) / (MAX4003_ADC_MAX - MAX4003_ADC_MIN)
#define MAX4003_DBM_FACTOR   (MAX4003_DBM_MAX - MAX4003_ADC_MAX * MAX4003_DBM_SCOPE)

#define MAX4003_AGC_SCOPE ( MAX4003_AGC_MAX -  MAX4003_AGC_MIN) / (4095.0f)
#define MAX4003_AGC_FACTOR   (MAX4003_AGC_MAX - 4095.0f * MAX4003_AGC_SCOPE)
#define MAX4003_VOLTAGE_SCOPE ( MAX4003_VOLTAGE_MAX -  MAX4003_VOLTAGE_MIN) / (4095.0f)
#define MAX4003_VOLTAGE_FACTOR MAX4003_VOLTAGE_MAX - 4096.0f * MAX4003_VOLTAGE_SCOPE
#define MAX4003_IS_CALIBRATED 0xAA
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
	UHF_TONE,
	SNIFFER
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
	float inputVoltageReal;
	uint16_t currentReal;
	uint8_t ucTemperature;
	uint16_t baseCurrentReal;
	uint8_t attenuation;
	bool isRemoteAttenuation;
	bool is_attenuation_updated;
	bool isSmartTune;
	bool isReverse;
	bool isOverCurrent;
	Id_t id;
	Function_t function;
	bool calc_en;
	uint32_t lastUpdateTicks;
	uint8_t state;
} Vlad_t;

typedef struct SERVER_MODULE {
	uint16_t inputVoltage;
	uint16_t consumptionCurrent;
	float inputVoltageReal;
	float counsumptionCurrentReal;
	float vrefintVoltage;
	uint16_t lm75Temperature;
	uint16_t ucTemperature;
	Id_t id;
	Function_t function;
	uint32_t lastUpdateTicks;
} Server_t;


void module_init(Module_pa_t*,Function_t,Id_t);
void module_calc_parameters(Module_pa_t m,uint16_t* media_array);
void pa_sample_timer3_init();
void module_pa_state_update(Module_pa_t *pa);
void toneUhfInit(Function_t funcion, Id_t id, Tone_uhf_t *uhf);
Vlad_t* vladInit(Function_t id);
Server_t* serverInit(Function_t function);
uint8_t encodeVladToLtel(uint8_t *frame, Vlad_t *vlad);
uint8_t decodeVladMeasurements(Vlad_t *vlad, uint8_t *buffer);
#endif /* INC_LTEL_H_ */
