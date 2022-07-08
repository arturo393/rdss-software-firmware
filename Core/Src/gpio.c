#include "gpio.h"


/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
void MX_GPIO_Init(void)
{
	  GPIO_InitTypeDef GPIO_InitStruct = {0};

	  /* GPIO Ports Clock Enable */
	  __HAL_RCC_GPIOD_CLK_ENABLE();
	  __HAL_RCC_GPIOA_CLK_ENABLE();
	  __HAL_RCC_GPIOB_CLK_ENABLE();

	  /*Configure GPIO pin Output Level */
	  HAL_GPIO_WritePin(GPIOB, LORA_NSS_Pin|LORA_RST_Pin|LORA_DIO3_Pin|LORA_DIO1_Pin
	                          |LORA_BUSSY_Pin|LED2_Pin|LED3_Pin|LED1_Pin
	                          |RS485_DE_Pin|BUZZER_Pin, GPIO_PIN_RESET);

	  /*Configure GPIO pins : LORA_NSS_Pin LORA_RST_Pin LORA_DIO3_Pin LORA_DIO1_Pin
	                           LORA_BUSSY_Pin PB3 PB4 LED_Pin
	                           RS485_DE_Pin BUZZER_Pin */
	  GPIO_InitStruct.Pin = LORA_NSS_Pin|LORA_RST_Pin|LORA_DIO3_Pin|LORA_DIO1_Pin
	                          |LORA_BUSSY_Pin|LED2_Pin|LED3_Pin|LED1_Pin
	                          |RS485_DE_Pin|BUZZER_Pin;
	  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);


}
