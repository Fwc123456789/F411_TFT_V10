#include "app_scheduler.h"
#include "app_data.h"
#include "app_tasks.h"
#include "TFT.h"
#include "DHT11.h"
#include "Ds18b20.h"
#include "Key.h"
#include "tim.h"

/* 软件10ms节拍更新 */
static void App_Tick_Update(void)
{
    static uint32_t last_ms = 0;
    uint32_t now_ms = HAL_GetTick();

    if (now_ms - last_ms >= 10) {
        last_ms += 10;
        g_tick_10ms++;
    }
}

/* 应用初始化 */
void App_Init(void)
{
    App_Data_Init();

    ST7789_Init();
    ST7789_Clear(BLACK);

    DHT11_Init();
    Key_Init();

    TFT_Init_Task();
    Key_Init_Task();
    ESP8266_Init_Task();
    UartLog_Init();
    Buzzer_Init_Task();
    ds18b20_init();

    HAL_TIM_Base_Start(&htim9);
    HAL_TIM_Base_Start_IT(&htim11);
}

/* 分时任务调度器 */
void App_Scheduler_Run(void)
{
    static uint32_t last_tft = 0;
    static uint32_t last_key = 0;
    static uint32_t last_dht11 = 0;
    static uint32_t last_ds18b20 = 0;
    static uint32_t last_esp = 0;
    static uint32_t last_log = 0;
    static uint32_t last_adc = 0;
    static uint32_t last_buzzer = 0;
    static uint32_t last_mqtt = 0;
    static uint32_t last_ui = 0;

    uint32_t now = g_tick_10ms;

    /* TFT刷新: 20ms一次 */
    if (now - last_tft >= 2) {
        TFT_Update_Task();
        last_tft = now;
    }

    /* 按键扫描: 10ms一次 */
    if (now - last_key >= 1) {
        Key_Scan_Task();
        last_key = now;
    }

    /* UI处理: 10ms一次 */
    if (now - last_ui >= 1) {
        UI_Task();
        last_ui = now;
    }

    /* DHT11: 每2秒一次 */
    if (now - last_dht11 >= 200) {
        DHT11_Read_Task();
        last_dht11 = now;
    }

    /* DS18B20: 每1秒调度一次，内部状态机非阻塞 */
    if (now - last_ds18b20 >= 100) {
        DS18B20_Task();
        last_ds18b20 = now;
    }

    /* ESP8266: 每10ms一次 */
    if (now - last_esp >= 1) {
        ESP8266_Poll_Task();
        last_esp = now;
    }

    /* UART日志: 每10ms刷新 */
    if (now - last_log >= 1) {
        UartLog_Flush_Task();
        last_log = now;
    }

    /* ADC: 每100ms一次 */
    if (now - last_adc >= 10) {
        ADC_Task();
        last_adc = now;
    }

    /* 蜂鸣器: 每10ms一次 */
    if (now - last_buzzer >= 1) {
        Buzzer_Task();
        last_buzzer = now;
    }

    /* MQTT: 每2秒一次 */
    if (now - last_mqtt >= 200) {
        MQTT_Task();
        last_mqtt = now;
    }
}

/* 主循环入口 */
void App_Run(void)
{
#if APP_USE_SOFT_10MS_TICK
    App_Tick_Update();
#endif
    App_Scheduler_Run();
}
