/*
 * led.c
 *
 *  Created on: Sep 26, 2022
 *      Author: sigmadev
 */
#include "led.h"

LED_t * ledInit() {
	LED_t *led;
	led = malloc(sizeof(LED_t));
	/*CURRENT NORMAL LED PA4 (A)*/
	SET_BIT(GPIOA->MODER, GPIO_MODER_MODE4_0);
	CLEAR_BIT(GPIOA->MODER, GPIO_MODER_MODE4_1);
	led_reset(led);

	return led;
}
void led_off(void) {

}

void blinkKALed(LED_t *l) {
	if (HAL_GetTick() - l->kaCounter > LED_KA_STATE_TIMEOUT)
		l->kaCounter = HAL_GetTick();
	else {
		if (HAL_GetTick() - l->kaCounter > LED_KA_ON_TIMEOUT)
			SYS_RP_LED_OFF();
		else
			SYS_RP_LED_ON();
	}
}

void led_reset(LED_t *l) {
//	l->chCounter = 0;
//	l->clCounter = 0;
//	l->cnCounter = 0;
	l->kaCounter = HAL_GetTick();
//	l->sysrpCounter = 0;
//	l->thCounter = 0;
//	l->tokCounter = 0;
}

