/*
 * led.c
 *
 *  Created on: Sep 26, 2022
 *      Author: sigmadev
 */
#include "led.h"

void ledInit(LED_t *led) {
	/*CURRENT NORMAL LED PA7 (A)*/
	SET_BIT(GPIOA->MODER, GPIO_MODER_MODE7_0);
	CLEAR_BIT(GPIOA->MODER, GPIO_MODER_MODE7_1);
	/*CURRENT NORMAL LED PB0 (B)*/
	SET_BIT(GPIOB->MODER, GPIO_MODER_MODE0_0);
	CLEAR_BIT(GPIOB->MODER, GPIO_MODER_MODE0_1);
	/*CURRENT NORMAL LED PB1 (SR)*/
	SET_BIT(GPIOB->MODER, GPIO_MODER_MODE1_0);
	CLEAR_BIT(GPIOB->MODER, GPIO_MODER_MODE1_1);
	led_reset(led);
}
void led_off(void) {

}

void led_enable_kalive(LED_t *l) {
	if (HAL_GetTick() - l->kaCounter > LED_KA_STATE_TIMEOUT)
		l->kaCounter = HAL_GetTick();
	else {
		if (HAL_GetTick() - l->kaCounter > LED_KA_ON_TIMEOUT)
			SYS_RP_LED_OFF();
		else
			SYS_RP_LED_ON();
	}
}
void led_i2c_toggle(LED_t *l) {
	if (READ_BIT(GPIOB->ODR, GPIO_ODR_OD1))
		i2c1_irq_led_off();
	else
		i2c1_irq_led_on();
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

