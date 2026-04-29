#include "ONE.h"
void HAL_Delay_us(uint32_t nus)
{
	uint32_t ticks;
	uint32_t told,tnow,tcnt=0;
	uint32_t reload=SysTick->LOAD;
	ticks=nus*100;//100MHz
	told=SysTick->VAL;
	while(1)
	{
		tnow=SysTick->VAL;
		if(tnow!=told)
		{
			if(tnow<told)tcnt+=told-tnow;
			else tcnt+=reload-tnow+told;
			told=tnow;
			if(tcnt>=ticks)break;
		}
	}
}
 
void onewire_hold()
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Mode=GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pin=GPIO_PIN_8;
	GPIO_InitStruct.Pull=GPIO_PULLUP;
	GPIO_InitStruct.Speed=GPIO_SPEED_HIGH;
	HAL_GPIO_Init(GPIOB,&GPIO_InitStruct);
}
void onewire_init()
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
 
	/* important : modified to fit your pin clock source*/
	__HAL_RCC_GPIOC_CLK_ENABLE();
	
	GPIO_InitStruct.Mode=GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pin=GPIO_PIN_8;
	GPIO_InitStruct.Pull=GPIO_NOPULL;
	GPIO_InitStruct.Speed=GPIO_SPEED_HIGH;
	HAL_GPIO_Init(GPIOB,&GPIO_InitStruct);
	
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_8,GPIO_PIN_RESET);
	HAL_Delay_us(500);
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_8,GPIO_PIN_SET);
	HAL_Delay_us(40);
	HAL_GPIO_DeInit(GPIOB,GPIO_PIN_8);
	HAL_Delay_us(500);
}
 
void onewire_write(uint8_t dat)
{
	onewire_hold();
	if(dat==0)
	{
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_8,GPIO_PIN_RESET);
		HAL_Delay_us(60);
		HAL_GPIO_DeInit(GPIOB,GPIO_PIN_8);
		HAL_Delay_us(1);
	}
	else
	{
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_8,GPIO_PIN_RESET);
		HAL_Delay_us(1);
		HAL_GPIO_DeInit(GPIOB,GPIO_PIN_8);
		HAL_Delay_us(60);
	}
}
 
uint8_t onewire_read()
{
	uint8_t t;
	onewire_hold();
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_8,GPIO_PIN_RESET);
	HAL_Delay_us(1);
	HAL_GPIO_DeInit(GPIOB,GPIO_PIN_8);
	HAL_Delay_us(10);
	t=HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_8);
	HAL_Delay_us(50);
	return t;
}
 
void onewire_sendbyte(uint8_t dat)
{
	uint8_t i;
	for(i=0;i<8;i++)
	{
		if((dat&0x01)==1)
			onewire_write(1);
		else
			onewire_write(0);
		dat=dat>>1;
	}
}
uint8_t onewire_readbyte(void)    // read one byte
{        
    uint8_t i,j,dat;
    dat=0;
	for (i=1;i<=8;i++) 
	{
        j=onewire_read();
        dat=(j<<7)|(dat>>1);
    }						    
    return dat;
}

