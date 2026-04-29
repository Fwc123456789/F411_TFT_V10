#include "Key.h"                  // Device header

uint8_t KeyNum;

void Key_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
	
  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pins : PB12 PB13 PB14 PB15 */
  GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}


uint8_t b1o = 1;
uint8_t b1n;
uint8_t b2o = 1;
uint8_t b2n;
uint8_t b3o = 1;
uint8_t b3n;
uint8_t b4o = 1;
uint8_t b4n;

void key_scan()
{
//	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_SET);
	
	b1n = HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_12);
	b2n = HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_13);
	b3n = HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_14);
	b4n = HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_15);
	
	if(b1n == 0 && b1o == 1)//按键key1
	{
		TIM9->CNT = 0;
	}
	else if(b1n == 0 && b1o == 0)//按键key1一直
	{
		if(TIM9->CNT >= 10000)//key1长按
		{

		}
	}
	else if(b1n == 1 && b1o == 0)//按键key1松开
	{
		if(TIM9->CNT <= 10000)//key1短按
		{
				KeyNum = 1;
		}
	}
	
	
	if(b2n == 0 && b2o == 1)//按键key2
	{
		TIM9->CNT = 0;
	}
	else if(b2n == 0 && b2o == 0)//按键key2一直
	{
		if(TIM9->CNT >= 10000)//key2长按
		{

		}
	}
	else if(b2n == 1 && b2o == 0)//按键key2松开
	{
		if(TIM9->CNT <= 10000)//key2短按
		{
				KeyNum = 2;
		}
	}
	
	
	if(b3n == 0 && b3o == 1)//按键key3
	{
		TIM9->CNT = 0;
	}
	else if(b3n == 0 && b3o == 0)//key3一直
	{
		if(TIM9->CNT >= 10000)//key3长按
		{

		}
	}
	else if(b3n == 1 && b3o == 0)//key3松开
	{
		if(TIM9->CNT <= 10000)//key3短按
		{
				KeyNum = 3;
		}
	}
	
	
	if(b4n == 0 && b4o == 1)//按键key4
	{
		TIM9->CNT = 0;
	}
	else if(b4n == 0 && b4o == 0)//key4一直
	{
		if(TIM9->CNT >= 10000)//key4长按
		{

		}
	}
	else if(b4n == 1 && b4o == 0)//按键B2松开
	{
		if(TIM9->CNT <= 10000)//key4短按
		{
				KeyNum = 4;
		}
	}
	
	b1o = b1n;
	b2o = b2n;
	b3o = b3n;
	b4o = b4n;
	

}
