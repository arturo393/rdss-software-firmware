/*
 * led.c
 *
 *  Created on: Sep 26, 2022
 *      Author: sigmadev
 */
#include "led.h"

void ledInit(LED_t *led) {
	/*CURRENT NORMAL LED PA7 (A)*/
	SET_BIT(GPIOB->ODR, GPIO_ODR_ODR5);
	CLEAR_BIT(GPIOB->ODR, GPIO_ODR_ODR5);
	/*CURRENT NORMAL LED PB0 (B)*/
	SET_BIT(GPIOB->ODR, GPIO_ODR_ODR4);
	CLEAR_BIT(GPIOB->ODR, GPIO_ODR_ODR4);
	/*CURRENT NORMAL LED PB1 (SR)*/
	SET_BIT(GPIOB->ODR, GPIO_ODR_ODR3);
	CLEAR_BIT(GPIOB->ODR, GPIO_ODR_ODR3);
	led_reset(led);
}
void led_off(void) {

}

void blinkKALed(LED_t *l) {
	if (HAL_GetTick() - l->kaCounter > LED_KA_STATE_TIMEOUT) {
		l->kaCounter = HAL_GetTick();
		SYS_RP_LED_ON();
	} else if (HAL_GetTick() - l->kaCounter > LED_KA_ON_TIMEOUT)
		SYS_RP_LED_OFF();
}

void led_reset(LED_t *l) {
	l->chCounter = 0;
	l->clCounter = 0;
	l->cnCounter = 0;
	l->kaCounter = HAL_GetTick();
	l->sysrpCounter = 0;
	l->thCounter = 0;
	l->tokCounter = 0;
}

