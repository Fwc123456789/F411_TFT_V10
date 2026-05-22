#ifndef __APP_TASKS_H__
#define __APP_TASKS_H__

#include "main.h"
#include "app_data.h"

/* 按键事件类型 */
typedef enum {
    KEY_EVENT_NONE = 0,
    KEY_EVENT_SHORT_PRESS,
    KEY_EVENT_LONG_PRESS
} Key_Event_Type_t;

typedef struct {
    uint8_t key_id;
    Key_Event_Type_t event;
} Key_Event_t;

/* 按键ID */
#define KEY_ID_1  1
#define KEY_ID_2  2
#define KEY_ID_3  3
#define KEY_ID_4  4

/* UI页面状态 */
typedef enum {
    UI_PAGE_HOME = 0,
    UI_PAGE_TEMP_HUMI,
    UI_PAGE_ADC,
    UI_PAGE_WIFI,
    UI_PAGE_SYSTEM_INFO
} UI_Page_t;

/* DS18B20状态机 */
typedef enum {
    DS18B20_STATE_IDLE = 0,
    DS18B20_STATE_START_CONVERT,
    DS18B20_STATE_WAIT_CONVERT,
    DS18B20_STATE_READ_SCRATCHPAD,
    DS18B20_STATE_PROCESS_DATA,
    DS18B20_STATE_ERROR
} DS18B20_State_t;

/* ESP8266状态机 */
typedef enum {
    ESP_STATE_POWER_ON = 0,
    ESP_STATE_AT_TEST,
    ESP_STATE_ECHO_OFF,
    ESP_STATE_SET_WIFI_MODE,
    ESP_STATE_CONNECT_WIFI,
    ESP_STATE_CONNECT_MQTT,
    ESP_STATE_MQTT_READY,
    ESP_STATE_MQTT_PUBLISH,
    ESP_STATE_IDLE,
    ESP_STATE_ERROR_RETRY
} ESP8266_State_t;

/* 任务函数 */
void TFT_Update_Task(void);
void Key_Scan_Task(void);
void DHT11_Read_Task(void);
void DS18B20_Task(void);
void ESP8266_Poll_Task(void);
void UartLog_Flush_Task(void);
void ADC_Task(void);
void Buzzer_Task(void);
void MQTT_Task(void);
void UI_Task(void);

/* 按键事件接口 */
uint8_t Key_GetEvent(uint8_t *key_id, Key_Event_t *event);
void Key_Init_Task(void);

/* TFT任务初始化 */
void TFT_Init_Task(void);

/* ESP8266任务初始化 */
void ESP8266_Init_Task(void);

/* UART日志初始化 */
void UartLog_Init(void);

/* Buzzer初始化 */
void Buzzer_Init_Task(void);

extern volatile uint8_t tft_spi_dma_busy;

void Log_WriteChar(char c);
void Buzzer_Beep(uint16_t duration_10ms);

#endif
