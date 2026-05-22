#include "app_tasks.h"
#include "app_data.h"
#include "TFT.h"
#include "DHT11.h"
#include "Ds18b20.h"
#include "ONE.h"
#include "Key.h"
#include "adc.h"
#include "usart.h"
#include <stdio.h>
#include <string.h>

/* ================================================================
 *  全局变量
 * ================================================================ */

static Key_Event_t key_event_queue[8];
static uint8_t key_queue_wr = 0;
static uint8_t key_queue_rd = 0;

static UI_Page_t ui_page = UI_PAGE_HOME;
static uint8_t ui_need_full_redraw = 1;
static uint8_t ui_data_dirty = 0;

/* 上一次显示值，用于局部刷新 */
static float last_disp_dht11_temp = -999.0f;
static float last_disp_dht11_humi = -999.0f;
static float last_disp_ds18b20_temp = -999.0f;
static uint16_t last_disp_adc0 = 0xFFFF;
static float last_disp_volt = -999.0f;

/* ================================================================
 *  日志FIFO
 * ================================================================ */
#define LOG_FIFO_SIZE  1024
static char log_fifo[LOG_FIFO_SIZE];
static volatile uint16_t log_wr = 0;
static volatile uint16_t log_rd = 0;
static volatile uint8_t log_tx_busy = 0;

/* ================================================================
 *  ESP8266 状态机变量
 * ================================================================ */
static ESP8266_State_t esp_state = ESP_STATE_POWER_ON;
static uint32_t esp_cmd_tick = 0;
static uint8_t esp_retry_count = 0;
static uint32_t mqtt_publish_phase = 0;
static uint32_t mqtt_last_publish_tick = 0;

/* ESP8266 UART接收环形缓冲 */
#define ESP_RX_BUF_SIZE  512
static uint8_t esp_rx_buf[ESP_RX_BUF_SIZE];
static volatile uint16_t esp_rx_wr = 0;
static uint16_t esp_rx_rd = 0;

/* ================================================================
 *  蜂鸣器状态机
 * ================================================================ */
static uint8_t buzzer_on = 0;
static uint32_t buzzer_start_tick = 0;
static uint16_t buzzer_duration = 0;

/* ================================================================
 *  DS18B20 状态机
 * ================================================================ */
static DS18B20_State_t ds18b20_state = DS18B20_STATE_IDLE;
static uint32_t ds18b20_convert_start_tick = 0;
static float ds18b20_last_valid_temp = 0.0f;
static uint8_t ds18b20_lsb = 0;
static uint8_t ds18b20_msb = 0;

/* UI菜单选择 */
static uint8_t menu_selection = 0;

/* DS18B20 底层OneWire操作 (不阻塞) */
static void DS18B20_StartConvert(void)
{
    onewire_init();
    onewire_sendbyte(0xCC);  /* Skip ROM */
    onewire_sendbyte(0x44);  /* Convert T */
}

static uint8_t DS18B20_ReadScratchpad(uint8_t *lsb, uint8_t *msb)
{
    onewire_init();
    onewire_sendbyte(0xCC);  /* Skip ROM */
    onewire_sendbyte(0xBE);  /* Read Scratchpad */
    *lsb = onewire_readbyte();
    *msb = onewire_readbyte();
    return 0;
}

static float DS18B20_ProcessRawData(uint8_t lsb, uint8_t msb)
{
    uint16_t temp;
    float value;
    temp = ((uint16_t)msb << 8) | lsb;
    if ((temp & 0xF800) == 0xF800) {
        temp = (~temp) + 1;
        value = temp * (-0.0625f);
    } else {
        value = temp * 0.0625f;
    }
    return value;
}

/* ================================================================
 *  按键事件队列
 * ================================================================ */
static void Key_PushEvent(uint8_t key_id, Key_Event_Type_t event_type)
{
    uint8_t next = (key_queue_wr + 1) % 8;
    if (next != key_queue_rd) {
        key_event_queue[key_queue_wr].event = event_type;
        key_event_queue[key_queue_wr].key_id = key_id;
        key_queue_wr = next;
    }
}

uint8_t Key_GetEvent(uint8_t *key_id, Key_Event_t *event)
{
    if (key_queue_rd == key_queue_wr) return 0;
    *key_id = key_event_queue[key_queue_rd].key_id;
    *event = key_event_queue[key_queue_rd];
    key_queue_rd = (key_queue_rd + 1) % 8;
    return 1;
}

/* ================================================================
 *  日志函数
 * ================================================================ */
void UartLog_Init(void)
{
    log_wr = 0;
    log_rd = 0;
    log_tx_busy = 0;
}

/* 替换fputc: printf最终调用此函数写入FIFO */
static void Log_PutChar(char c)
{
    uint16_t next = (log_wr + 1) % LOG_FIFO_SIZE;
    if (next != log_rd) {
        log_fifo[log_wr] = c;
        log_wr = next;
    }
}

/* ================================================================
 *  初始化函数
 * ================================================================ */
void TFT_Init_Task(void)
{
    tft_spi_dma_busy = 0;
}

void Key_Init_Task(void)
{
    key_queue_wr = 0;
    key_queue_rd = 0;
}

void ESP8266_Init_Task(void)
{
    esp_state = ESP_STATE_POWER_ON;
    esp_cmd_tick = 0;
    esp_retry_count = 0;
    mqtt_publish_phase = 0;
    mqtt_last_publish_tick = 0;
    esp_rx_wr = 0;
    esp_rx_rd = 0;
}

void Buzzer_Init_Task(void)
{
    buzzer_on = 0;
    buzzer_start_tick = 0;
    buzzer_duration = 0;
}

/* ================================================================
 *  TFT更新任务 - 非阻塞DMA刷新
 * ================================================================ */
void TFT_Update_Task(void)
{
    if (tft_spi_dma_busy) {
        return;  /* DMA忙，本轮跳过 */
    }

    if (!ui_need_full_redraw && !ui_data_dirty) {
        return;  /* 无需刷新 */
    }

    if (ui_need_full_redraw) {
        ui_need_full_redraw = 0;
        ui_data_dirty = 0;

        switch (ui_page) {
        case UI_PAGE_HOME: {
            uint16_t c0, c1, c2, c3;
            c0 = (menu_selection == 0) ? RED : WHITE;
            c1 = (menu_selection == 1) ? RED : WHITE;
            c2 = (menu_selection == 2) ? RED : WHITE;
            c3 = (menu_selection == 3) ? RED : WHITE;
            ST7789_Clear(BLACK);
            ST7789_ShowString(70, 10, "Main Menu", Font_16x26, WHITE, BLACK);
            ST7789_ShowString(50, 60, "1. Temp & Humi", Font_11x18, c0, BLACK);
            ST7789_ShowString(50, 90, "2. ADC", Font_11x18, c1, BLACK);
            ST7789_ShowString(50, 120, "3. WiFi Config", Font_11x18, c2, BLACK);
            ST7789_ShowString(50, 150, "4. System Info", Font_11x18, c3, BLACK);
            ST7789_ShowString(30, 190, "KEY4/3:Select KEY1:OK", Font_7x10, YELLOW, BLACK);
            break;
        }

        case UI_PAGE_TEMP_HUMI:
            ST7789_Clear(BLACK);
            ST7789_ShowString(50, 10, "Temp & Humi", Font_16x26, WHITE, BLACK);
            ST7789_ShowString(30, 290, "KEY2:Back to Menu", Font_7x10, YELLOW, BLACK);
            ui_data_dirty = 1;  /* 标记数据区域需刷新 */
            break;

        case UI_PAGE_ADC:
            ST7789_Clear(BLACK);
            ST7789_ShowString(50, 10, "ADC", Font_16x26, WHITE, BLACK);
            ST7789_ShowString(10, 70, "ADC1 (PA1):", Font_11x18, WHITE, BLACK);
            ST7789_ShowString(10, 100, "Voltage:", Font_11x18, WHITE, BLACK);
            ST7789_ShowString(10, 130, "Temp Sensor:", Font_11x18, WHITE, BLACK);
            ST7789_ShowString(10, 160, "Temp Value:", Font_11x18, WHITE, BLACK);
            ST7789_ShowString(10, 190, "Vrefint:", Font_11x18, WHITE, BLACK);
            ST7789_ShowString(10, 220, "Vrefint V:", Font_11x18, WHITE, BLACK);
            ST7789_ShowString(30, 290, "KEY2:Back to Menu", Font_7x10, YELLOW, BLACK);
            ui_data_dirty = 1;
            break;

        case UI_PAGE_WIFI:
            ST7789_Clear(BLACK);
            ST7789_ShowString(50, 10, "WiFi Config", Font_16x26, WHITE, BLACK);
            ST7789_ShowString(5, 50, "SSID: 2003", Font_11x18, WHITE, BLACK);
            ST7789_ShowString(5, 80, "PASS: 12121212", Font_11x18, WHITE, BLACK);
            ST7789_ShowString(5, 110, "IP: 44.232.241.40", Font_11x18, WHITE, BLACK);
            ST7789_ShowString(30, 290, "KEY2:Back to Menu", Font_7x10, YELLOW, BLACK);
            break;

        case UI_PAGE_SYSTEM_INFO:
            ST7789_Clear(BLACK);
            ST7789_ShowString(50, 10, "System Info", Font_16x26, WHITE, BLACK);
            ST7789_ShowString(5, 50, "STM32F4", Font_11x18, WHITE, BLACK);
            ST7789_ShowString(5, 80, "Version: 2.0 Release", Font_11x18, WHITE, BLACK);
            ST7789_ShowString(5, 140, "- Temperature", Font_11x18, WHITE, BLACK);
            ST7789_ShowString(5, 170, "- Humidity", Font_11x18, WHITE, BLACK);
            ST7789_ShowString(5, 200, "- ADC Data", Font_11x18, WHITE, BLACK);
            ST7789_ShowString(30, 290, "KEY2:Back to Menu", Font_7x10, YELLOW, BLACK);
            break;
        }
        last_disp_dht11_temp = -999.0f;
        last_disp_dht11_humi = -999.0f;
        last_disp_ds18b20_temp = -999.0f;
        last_disp_adc0 = 0xFFFF;
        last_disp_volt = -999.0f;
    }

    /* 局部数据刷新 */
    if (ui_data_dirty) {
        ui_data_dirty = 0;

        if (ui_page == UI_PAGE_TEMP_HUMI) {
            char buf[24];
            if (last_disp_dht11_temp != g_app_data.dht11_temp ||
                last_disp_dht11_humi != g_app_data.dht11_humi ||
                last_disp_ds18b20_temp != g_app_data.ds18b20_temp) {

                ST7789_FillRect(10, 160, 230, 220, BLACK);

                snprintf(buf, sizeof(buf), "DHT11 Temp: %.1f C", g_app_data.dht11_temp);
                ST7789_ShowString(15, 165, buf, Font_7x10, g_app_data.dht11_valid ? RED : GRAY, BLACK);

                snprintf(buf, sizeof(buf), "DHT11 Humi: %.0f %%", g_app_data.dht11_humi);
                ST7789_ShowString(15, 180, buf, Font_7x10, g_app_data.dht11_valid ? BLUE : GRAY, BLACK);

                snprintf(buf, sizeof(buf), "DS18B20: %.1f C", g_app_data.ds18b20_temp);
                ST7789_ShowString(15, 195, buf, Font_7x10, g_app_data.ds18b20_valid ? GREEN : GRAY, BLACK);

                last_disp_dht11_temp = g_app_data.dht11_temp;
                last_disp_dht11_humi = g_app_data.dht11_humi;
                last_disp_ds18b20_temp = g_app_data.ds18b20_temp;
            }
        } else if (ui_page == UI_PAGE_ADC) {
            char buf[24];
            if (last_disp_adc0 != g_app_data.adc_values[0] ||
                last_disp_volt != g_app_data.adc_ch1_voltage ||
                !g_app_data.adc_valid) {

                ST7789_FillRect(150, 70, 230, 250, BLACK);

                snprintf(buf, sizeof(buf), "%d", g_app_data.adc_values[0]);
                ST7789_ShowString(150, 70, buf, Font_11x18, g_app_data.adc_valid ? WHITE : GRAY, BLACK);

                snprintf(buf, sizeof(buf), "%.2f V", g_app_data.adc_ch1_voltage);
                ST7789_ShowString(150, 100, buf, Font_11x18, g_app_data.adc_valid ? WHITE : GRAY, BLACK);

                snprintf(buf, sizeof(buf), "%d", g_app_data.adc_values[1]);
                ST7789_ShowString(150, 130, buf, Font_11x18, GRAY, BLACK);

                snprintf(buf, sizeof(buf), "%.1f C", g_app_data.adc_internal_temp);
                ST7789_ShowString(150, 160, buf, Font_11x18, GRAY, BLACK);

                snprintf(buf, sizeof(buf), "%d", g_app_data.adc_values[2]);
                ST7789_ShowString(150, 190, buf, Font_11x18, GRAY, BLACK);

                snprintf(buf, sizeof(buf), "%.2f V", g_app_data.adc_vrefint_voltage);
                ST7789_ShowString(150, 220, buf, Font_11x18, GRAY, BLACK);

                last_disp_adc0 = g_app_data.adc_values[0];
                last_disp_volt = g_app_data.adc_ch1_voltage;
            }
        }
    }
}

/* ================================================================
 *  按键扫描任务
 * ================================================================ */
void Key_Scan_Task(void)
{
    static uint8_t last_keynum = 0;

    if (KeyNum != 0) {
        uint8_t k = KeyNum;
        KeyNum = 0;
        Key_PushEvent(k, KEY_EVENT_SHORT_PRESS);
    }
    last_keynum = KeyNum;
}

/* ================================================================
 *  DHT11读取任务
 * ================================================================ */
void DHT11_Read_Task(void)
{
    uint16_t temp_raw, humi_raw;
    if (DHT11_Read_Data(&temp_raw, &humi_raw) == 0) {
        g_app_data.dht11_temp = (float)temp_raw;
        g_app_data.dht11_humi = (float)humi_raw;
        g_app_data.dht11_valid = 1;
    } else {
        g_app_data.dht11_valid = 0;
    }
    if (ui_page == UI_PAGE_TEMP_HUMI || ui_page == UI_PAGE_HOME) {
        ui_data_dirty = 1;
    }
}

/* ================================================================
 *  DS18B20非阻塞状态机任务
 * ================================================================ */
void DS18B20_Task(void)
{
    switch (ds18b20_state) {
    case DS18B20_STATE_IDLE:
        DS18B20_StartConvert();
        ds18b20_convert_start_tick = g_tick_10ms;
        ds18b20_state = DS18B20_STATE_WAIT_CONVERT;
        break;

    case DS18B20_STATE_WAIT_CONVERT:
        if (g_tick_10ms - ds18b20_convert_start_tick >= 75) {
            DS18B20_ReadScratchpad(&ds18b20_lsb, &ds18b20_msb);
            ds18b20_state = DS18B20_STATE_PROCESS_DATA;
            /* 立即处理，无需等待下次调度 */
            goto process_data;
        }
        break;

    case DS18B20_STATE_ERROR:
    default:
        g_app_data.ds18b20_valid = 0;
        ds18b20_state = DS18B20_STATE_IDLE;
        break;
    }
    return;

process_data: {
        float t = DS18B20_ProcessRawData(ds18b20_lsb, ds18b20_msb);
        if (t > -55.0f && t < 125.0f) {
            g_app_data.ds18b20_temp = t;
            g_app_data.ds18b20_valid = 1;
            ds18b20_last_valid_temp = t;
        } else {
            g_app_data.ds18b20_temp = ds18b20_last_valid_temp;
            g_app_data.ds18b20_valid = 0;
        }
        ds18b20_state = DS18B20_STATE_IDLE;
        if (ui_page == UI_PAGE_TEMP_HUMI || ui_page == UI_PAGE_HOME) {
            ui_data_dirty = 1;
        }
    }
}

/* ================================================================
 *  ESP8266轮询任务 - AT状态机
 * ================================================================ */
void ESP8266_Poll_Task(void)
{
    switch (esp_state) {
    case ESP_STATE_POWER_ON:
        esp_cmd_tick = g_tick_10ms;
        esp_state = ESP_STATE_AT_TEST;
        break;

    case ESP_STATE_AT_TEST:
        if (g_tick_10ms - esp_cmd_tick >= 50) {
            Log_PutChar('A'); Log_PutChar('T'); Log_PutChar('\r'); Log_PutChar('\n');
            esp_cmd_tick = g_tick_10ms;
            esp_state = ESP_STATE_ECHO_OFF;
        }
        break;

    case ESP_STATE_ECHO_OFF:
        if (g_tick_10ms - esp_cmd_tick >= 100) {
            Log_PutChar('A'); Log_PutChar('T'); Log_PutChar('E');
            Log_PutChar('0'); Log_PutChar('\r'); Log_PutChar('\n');
            esp_cmd_tick = g_tick_10ms;
            esp_state = ESP_STATE_SET_WIFI_MODE;
        }
        break;

    case ESP_STATE_SET_WIFI_MODE:
        if (g_tick_10ms - esp_cmd_tick >= 50) {
            Log_PutChar('A'); Log_PutChar('T'); Log_PutChar('+');
            Log_PutChar('C'); Log_PutChar('W'); Log_PutChar('M');
            Log_PutChar('O'); Log_PutChar('D'); Log_PutChar('E');
            Log_PutChar('='); Log_PutChar('1'); Log_PutChar('\r'); Log_PutChar('\n');
            esp_cmd_tick = g_tick_10ms;
            esp_state = ESP_STATE_CONNECT_WIFI;
        }
        break;

    case ESP_STATE_CONNECT_WIFI:
        if (g_tick_10ms - esp_cmd_tick >= 500) {
            Log_PutChar('A'); Log_PutChar('T'); Log_PutChar('+');
            Log_PutChar('C'); Log_PutChar('W'); Log_PutChar('J');
            Log_PutChar('A'); Log_PutChar('P'); Log_PutChar('=');
            Log_PutChar('"'); Log_PutChar('2'); Log_PutChar('0');
            Log_PutChar('0'); Log_PutChar('3'); Log_PutChar('"');
            Log_PutChar(','); Log_PutChar('"');
            Log_PutChar('1'); Log_PutChar('2'); Log_PutChar('1');
            Log_PutChar('2'); Log_PutChar('1'); Log_PutChar('2');
            Log_PutChar('1'); Log_PutChar('2'); Log_PutChar('"');
            Log_PutChar('\r'); Log_PutChar('\n');
            esp_cmd_tick = g_tick_10ms;
            esp_state = ESP_STATE_CONNECT_MQTT;
        }
        break;

    case ESP_STATE_CONNECT_MQTT:
        if (g_tick_10ms - esp_cmd_tick >= 500) {
            Log_PutChar('A'); Log_PutChar('T'); Log_PutChar('+');
            Log_PutChar('M'); Log_PutChar('Q'); Log_PutChar('T');
            Log_PutChar('T'); Log_PutChar('U'); Log_PutChar('S');
            Log_PutChar('E'); Log_PutChar('R'); Log_PutChar('C');
            Log_PutChar('F'); Log_PutChar('G'); Log_PutChar('=');
            Log_PutChar('0'); Log_PutChar(','); Log_PutChar('1');
            Log_PutChar(','); Log_PutChar('"'); Log_PutChar('c');
            Log_PutChar('l'); Log_PutChar('i'); Log_PutChar('e');
            Log_PutChar('n'); Log_PutChar('t'); Log_PutChar('l');
            Log_PutChar('d'); Log_PutChar('"'); Log_PutChar(',');
            Log_PutChar('"'); Log_PutChar('a'); Log_PutChar('d');
            Log_PutChar('m'); Log_PutChar('i'); Log_PutChar('n');
            Log_PutChar('"'); Log_PutChar(','); Log_PutChar('"');
            Log_PutChar('a'); Log_PutChar('d'); Log_PutChar('m');
            Log_PutChar('i'); Log_PutChar('n'); Log_PutChar('"');
            Log_PutChar(','); Log_PutChar('0'); Log_PutChar(',');
            Log_PutChar('0'); Log_PutChar(','); Log_PutChar('"');
            Log_PutChar('"'); Log_PutChar('\r'); Log_PutChar('\n');
            esp_cmd_tick = g_tick_10ms;
            esp_state = ESP_STATE_MQTT_READY;
        }
        break;

    case ESP_STATE_MQTT_READY:
        if (g_tick_10ms - esp_cmd_tick >= 200) {
            Log_PutChar('A'); Log_PutChar('T'); Log_PutChar('+');
            Log_PutChar('M'); Log_PutChar('Q'); Log_PutChar('T');
            Log_PutChar('T'); Log_PutChar('C'); Log_PutChar('O');
            Log_PutChar('N'); Log_PutChar('N'); Log_PutChar('=');
            Log_PutChar('0'); Log_PutChar(','); Log_PutChar('"');
            Log_PutChar('4'); Log_PutChar('4'); Log_PutChar('.');
            Log_PutChar('2'); Log_PutChar('3'); Log_PutChar('2');
            Log_PutChar('.'); Log_PutChar('2'); Log_PutChar('4');
            Log_PutChar('1'); Log_PutChar('.'); Log_PutChar('4');
            Log_PutChar('0'); Log_PutChar('"'); Log_PutChar(',');
            Log_PutChar('1'); Log_PutChar('8'); Log_PutChar('8');
            Log_PutChar('3'); Log_PutChar(','); Log_PutChar('1');
            Log_PutChar('\r'); Log_PutChar('\n');
            esp_cmd_tick = g_tick_10ms;
            esp_state = ESP_STATE_MQTT_PUBLISH;
        }
        break;

    case ESP_STATE_MQTT_PUBLISH:
        /* MQTT发布由MQTT_Task处理 */
        esp_state = ESP_STATE_IDLE;
        break;

    case ESP_STATE_IDLE:
        break;

    case ESP_STATE_ERROR_RETRY:
    default:
        if (g_tick_10ms - esp_cmd_tick >= 500) {
            esp_retry_count++;
            if (esp_retry_count > 5) {
                esp_retry_count = 0;
                esp_state = ESP_STATE_POWER_ON;
            } else {
                esp_state = ESP_STATE_IDLE;
            }
        }
        break;
    }
}

/* ================================================================
 *  MQTT任务 - 分时上传
 * ================================================================ */
void MQTT_Task(void)
{
    if (esp_state != ESP_STATE_IDLE) return;

    switch (mqtt_publish_phase) {
    case 0:
        if (g_app_data.dht11_valid) {
            char buf[80];
            int len = snprintf(buf, sizeof(buf),
                "AT+MQTTPUB=0,\"DHT11-TEMP\",\"{\\\"TEMP\\\\1\\\":%.0f}\",1,0\r\n",
                g_app_data.dht11_temp);
            for (int i = 0; i < len; i++) Log_PutChar(buf[i]);
        }
        mqtt_publish_phase = 1;
        break;
    case 1:
        if (g_app_data.dht11_valid) {
            char buf[80];
            int len = snprintf(buf, sizeof(buf),
                "AT+MQTTPUB=0,\"DHT11-HUMI\",\"{\\\"HUMI\\\\1\\\":%.0f}\",1,0\r\n",
                g_app_data.dht11_humi);
            for (int i = 0; i < len; i++) Log_PutChar(buf[i]);
        }
        mqtt_publish_phase = 2;
        break;
    case 2:
        if (g_app_data.ds18b20_valid) {
            char buf[80];
            int len = snprintf(buf, sizeof(buf),
                "AT+MQTTPUB=0,\"DS18B20\",\"{\\\"TEMP\\\\2\\\":%.1f}\",1,0\r\n",
                g_app_data.ds18b20_temp);
            for (int i = 0; i < len; i++) Log_PutChar(buf[i]);
        }
        mqtt_publish_phase = 0;
        break;
    }
}

/* ================================================================
 *  UART日志刷新任务
 * ================================================================ */
void UartLog_Flush_Task(void)
{
    if (log_tx_busy) {
        /* 检查上一次DMA/IT发送是否完成 */
        if (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_TC)) {
            log_tx_busy = 0;
            __HAL_UART_CLEAR_FLAG(&huart1, UART_FLAG_TC);
        } else {
            return;
        }
    }

    if (log_rd == log_wr) return;

    /* 每次发送一个字符，确保不阻塞 */
    uint8_t ch = (uint8_t)log_fifo[log_rd];
    log_rd = (log_rd + 1) % LOG_FIFO_SIZE;

    log_tx_busy = 1;
    HAL_UART_Transmit_IT(&huart1, &ch, 1);
}

/* ================================================================
 *  ADC采集任务
 * ================================================================ */
void ADC_Task(void)
{
    if (ui_page != UI_PAGE_ADC) return;

    /* 重新配置并读取通道1 */
    ADC_ChannelConfTypeDef sConfig = {0};
    sConfig.Channel = ADC_CHANNEL_1;
    sConfig.Rank = 1;
    sConfig.SamplingTime = ADC_SAMPLETIME_480CYCLES;
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);
    HAL_ADC_Start(&hadc1);
    if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK) {
        g_app_data.adc_values[0] = HAL_ADC_GetValue(&hadc1);
    }
    HAL_ADC_Stop(&hadc1);

    sConfig.Channel = ADC_CHANNEL_TEMPSENSOR;
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);
    HAL_ADC_Start(&hadc1);
    if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK) {
        g_app_data.adc_values[1] = HAL_ADC_GetValue(&hadc1);
    }
    HAL_ADC_Stop(&hadc1);

    sConfig.Channel = ADC_CHANNEL_VREFINT;
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);
    HAL_ADC_Start(&hadc1);
    if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK) {
        g_app_data.adc_values[2] = HAL_ADC_GetValue(&hadc1);
    }
    HAL_ADC_Stop(&hadc1);

    g_app_data.adc_ch1_voltage = (g_app_data.adc_values[0] * 3.3f) / 4095.0f;
    float temp_voltage = (g_app_data.adc_values[1] * 3.3f) / 4095.0f;
    g_app_data.adc_internal_temp = ((temp_voltage - 0.76f) / 0.0025f) + 25.0f;
    g_app_data.adc_vrefint_voltage = (g_app_data.adc_values[2] * 3.3f) / 4095.0f;
    g_app_data.adc_valid = 1;

    ui_data_dirty = 1;
}

/* ================================================================
 *  蜂鸣器任务
 * ================================================================ */
void Buzzer_Task(void)
{
    if (!buzzer_on) return;

    if (g_tick_10ms - buzzer_start_tick >= buzzer_duration) {
        buzzer_on = 0;
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
    }
}

/* 启动蜂鸣器 */
void Buzzer_Beep(uint16_t duration_10ms)
{
    buzzer_on = 1;
    buzzer_duration = duration_10ms;
    buzzer_start_tick = g_tick_10ms;
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
}

/* ================================================================
 *  UI状态机任务
 * ================================================================ */
void UI_Task(void)
{
    uint8_t key_id;
    Key_Event_t event;

    if (Key_GetEvent(&key_id, &event)) {
        if (event.event == KEY_EVENT_SHORT_PRESS) {
            switch (key_id) {
            case KEY_ID_1: /* 确认键 - 进入选中菜单 */
                if (ui_page == UI_PAGE_HOME) {
                    switch (menu_selection) {
                    case 0: ui_page = UI_PAGE_TEMP_HUMI; break;
                    case 1: ui_page = UI_PAGE_ADC; break;
                    case 2: ui_page = UI_PAGE_WIFI; break;
                    case 3: ui_page = UI_PAGE_SYSTEM_INFO; break;
                    }
                    ui_need_full_redraw = 1;
                }
                break;

            case KEY_ID_2: /* 返回键 */
                if (ui_page != UI_PAGE_HOME) {
                    ui_page = UI_PAGE_HOME;
                    ui_need_full_redraw = 1;
                }
                break;

            case KEY_ID_3: /* 下键 - 菜单选择 */
                if (ui_page == UI_PAGE_HOME) {
                    menu_selection = (menu_selection < 3) ? menu_selection + 1 : 0;
                    ui_need_full_redraw = 1;
                }
                break;

            case KEY_ID_4: /* 上键 - 菜单选择 */
                if (ui_page == UI_PAGE_HOME) {
                    menu_selection = (menu_selection > 0) ? menu_selection - 1 : 3;
                    ui_need_full_redraw = 1;
                }
                break;
            }
        }
    }
}

/* 日志FIFO写入接口 (供main.c的fputc调用) */
void Log_WriteChar(char c)
{
    Log_PutChar(c);
}
