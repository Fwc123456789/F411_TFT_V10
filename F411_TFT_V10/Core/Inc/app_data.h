#ifndef __APP_DATA_H__
#define __APP_DATA_H__

#include "main.h"
#include <stdint.h>

/* 全局系统数据结构 */
typedef struct {
    float dht11_temp;
    float dht11_humi;
    float ds18b20_temp;
    uint16_t adc_values[3];
    float adc_ch1_voltage;
    float adc_internal_temp;
    float adc_vrefint_voltage;
    uint8_t dht11_valid;
    uint8_t ds18b20_valid;
    uint8_t adc_valid;
    uint8_t wifi_connected;
    uint8_t mqtt_connected;
} App_Data_t;

extern App_Data_t g_app_data;
extern volatile uint32_t g_tick_10ms;

void App_Data_Init(void);

#endif
