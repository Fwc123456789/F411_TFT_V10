#ifndef ONEWIRE_H
#define ONEWIRE_H
#include "main.h"
void onewire_init();
void onewire_write(uint8_t dat);
uint8_t onewire_read();
uint8_t onewire_readbyte(void);
void onewire_sendbyte(uint8_t dat);
#endif

