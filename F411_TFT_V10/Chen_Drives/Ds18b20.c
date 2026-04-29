#include "ds18b20.h"
#include "ONE.h"
void ds18b20_init()
{
	onewire_init();
}
float ds18b20_getTemp()
{
	uint16_t temp;
	float value;
	uint8_t lsb,msb;
	onewire_init();
	onewire_sendbyte(0xcc);
	onewire_sendbyte(0x44);
	HAL_Delay(750);
	onewire_init();
	onewire_sendbyte(0xcc);
	onewire_sendbyte(0xbe);
	lsb=onewire_readbyte();
	msb=onewire_readbyte();
	temp=msb;
	temp=(temp<<8)+lsb;
    if((temp&0xf800)==0xf800)
	{
		temp=(~temp)+1;
		value=temp*(-0.0625);
	}
	else
	{
		value=temp*0.0625;	
	}
	return value;
	
}

