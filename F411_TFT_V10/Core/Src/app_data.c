#include "app_data.h"

App_Data_t g_app_data;
volatile uint32_t g_tick_10ms = 0;

void App_Data_Init(void)
{
    g_app_data.dht11_temp = 0.0f;
    g_app_data.dht11_humi = 0.0f;
    g_app_data.ds18b20_temp = 0.0f;
    g_app_data.dht11_valid = 0;
    g_app_data.ds18b20_valid = 0;
    g_app_data.adc_valid = 0;
    g_app_data.wifi_connected = 0;
    g_app_data.mqtt_connected = 0;
}
