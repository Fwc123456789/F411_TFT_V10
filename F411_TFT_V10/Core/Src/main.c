/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "TFT.h"
#include <stdio.h>
#include <string.h>
#include <math.h>  // 对于fabs函数
#include "DHT11.h"
#include "ds18b20.h"
#include "Key.h"



int fputc(int ch,FILE *f)
 
{
 
//采用轮询方式发送1字节数据，超时时间设置为无限等待
 
HAL_UART_Transmit(&huart1,(uint8_t *)&ch,1,HAL_MAX_DELAY);
 
return ch;
 
}

int fgetc(FILE *f)
 
{
 
uint8_t ch;
 
// 采用轮询方式接收 1字节数据，超时时间设置为无限等待
 
HAL_UART_Receive( &huart1,(uint8_t*)&ch,1, HAL_MAX_DELAY );
 
return ch;
 
}

/**
  * @brief  AT指令
  * @param  无
  * @retval 无
  */
void Send_AT_Commands_Simple(void)
{
    printf("AT\r\n");
    HAL_Delay(1000);
    
    
    printf("ATE0\r\n");  //
    HAL_Delay(500);
    
    printf("AT+CWMODE=1\r\n");
    HAL_Delay(500);
    
    printf("AT+CWJAP=\"2003\",\"12121212\"\r\n");
    HAL_Delay(5000);  // WiFi连接需要较长时间
    
    printf("AT+MQTTUSERCFG=0,1,\"clientld\",\"admin\",\"admin\",0,0,\"\"\r\n");
    HAL_Delay(1000);
    
    printf("AT+MQTTCONN=0,\"44.232.241.40\",1883,1\r\n");
    HAL_Delay(2000);
    
    printf("AT+MQTTSUB=0,\"8266_RX\",1\r\n");
    HAL_Delay(1000);
    
    // 第9条指令
    printf("AT+MQTTPUB=0,\"8266_TX\",\"{\\\"TEMP\\\\1\\\":18}\",1,0\r\n");
    HAL_Delay(1000);
		
}
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
// 菜单状态枚举定义
typedef enum {
  MENU_MAIN,      // 主菜单
  MENU_TEMP_HUMI, // 温湿度监测
  MENU_ADC,       // ADC采集
  MENU_WIFI,      // WiFi通信
  MENU_README     // 系统信息
} MenuState;

// 传感器数据结构体
typedef struct {
    uint16_t dht11_temperature;  // DHT11温度值
    uint16_t dht11_humidity;     // DHT11湿度值
    float ds18b20_temperature;   // DS18B20温度值
    uint32_t timestamp;          // 时间戳
} SensorData;

// 图表参数定义
#define MAX_HISTORY 100    // 最大历史数据点数
#define GRAPH_MARGIN 10    // 图表边距

// 显示区域定义
#define MENU_AREA_TOP 30      // 菜单区域顶部
#define MENU_AREA_BOTTOM 220  // 菜单区域底部
#define INFO_AREA_TOP 221     // 信息区域顶部
#define INFO_AREA_BOTTOM 319  // 信息区域底部

// 菜单项数量
#define MENU_ITEMS_COUNT 4
// 全局变量中断用
uint8_t count = 0,sensor_read_pending,duqu,shangchuan1,shangchuan2,shangchuan3;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
// 显示区域定义
#define MENU_AREA_TOP 30
#define MENU_AREA_BOTTOM 220
#define INFO_AREA_TOP 221
#define INFO_AREA_BOTTOM 319

// 菜单项数量
#define MENU_ITEMS_COUNT 4
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
// ADC相关变量
uint16_t adc_values[3];  // 存储三个通道的ADC原始值
float channel1_voltage;  // 通道1的电压值
float internal_temp;     // 内部温度值
float vrefint_voltage;   // Vrefint电压值

// 简化的温度显示函数
void ShowTemperature(float temperature, uint16_t x, uint16_t y, FontDef font, uint16_t color, uint16_t bgcolor)
{
    char temp_str[20];
    snprintf(temp_str, sizeof(temp_str), "%.1f C", temperature);
    
    // 清除旧内容
    uint16_t str_width = strlen(temp_str) * font.width;
    ST7789_FillRect(x, y, x + str_width, y + font.height, bgcolor);
    
    // 显示新值
    ST7789_ShowString(x, y, temp_str, font, color, bgcolor);
}


// 显示ADC值函数
void ShowADCValue(uint16_t value, uint16_t x, uint16_t y, FontDef font, uint16_t color, uint16_t bgcolor)
{
    char value_str[20];
    snprintf(value_str, sizeof(value_str), "%d", value);
    
    // 清除旧内容
    uint16_t str_width = strlen(value_str) * font.width;
    ST7789_FillRect(x, y, x + str_width, y + font.height, bgcolor);
    
    // 显示新值
    ST7789_ShowString(x, y, value_str, font, color, bgcolor);
}

// 显示电压值函数
void ShowVoltage(float voltage, uint16_t x, uint16_t y, FontDef font, uint16_t color, uint16_t bgcolor)
{
    char voltage_str[20];
    snprintf(voltage_str, sizeof(voltage_str), "%.2f V", voltage);
    
    // 清除旧内容
    uint16_t str_width = strlen(voltage_str) * font.width;
    ST7789_FillRect(x, y, x + str_width, y + font.height, bgcolor);
    
    // 显示新值
    ST7789_ShowString(x, y, voltage_str, font, color, bgcolor);
}

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
SensorData history[MAX_HISTORY];  // 历史数据存储数组
uint16_t data_index = 0;           // 当前数据索引
uint8_t menu_selection = 0;        // 菜单选择索引
MenuState current_menu = MENU_MAIN; // 当前菜单状态

uint16_t dht11_temperature = 0;    // DHT11温度值
uint16_t dht11_humidity = 0;       // DHT11湿度值
float ds18b20_temperature = 0.0;   // DS18B20温度值

uint32_t frame_count = 0;          // 帧计数器
uint32_t last_frame_time = 0;      // 上一帧时间
float fps = 0;                     // 帧率
float cpu_usage = 0;               // CPU使用率
uint32_t idle_time = 0;            // 空闲时间
uint32_t last_idle_check = 0;      // 上一次空闲检查时间
uint8_t need_redraw = 1;           // 需要重绘标志
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
// 函数声明
void DrawMainMenu(void);
void DrawTempHumiScreen(void);
void DrawADCScreen(void);
void DrawWiFiScreen(void);
void DrawReadmeScreen(void);
void UpdateCPUAndFPS(void);
void DrawGraphGrid(void);
void UpdateGraph(void);
void DisplayCurrentValues(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/**
  * @brief  绘制主菜单界面
  * @param  无
  * @retval 无
  */
void DrawMainMenu(void) {
  ST7789_Clear(BLACK);  // 清屏，黑色背景
  
  // 绘制菜单标题
  ST7789_ShowString(70, 10, "Main Menu", Font_16x26, WHITE, BLACK);
  
  // 绘制菜单项 - 竖排显示
  ST7789_ShowString(50, 60, "1. Temp & Humi", Font_11x18, 
                   menu_selection == 0 ? RED : WHITE, BLACK);
  ST7789_ShowString(50, 90, "2. ADC", Font_11x18, 
                   menu_selection == 1 ? RED : WHITE, BLACK);
  ST7789_ShowString(50, 120, "3. WiFi Config", Font_11x18, 
                   menu_selection == 2 ? RED : WHITE, BLACK);
  ST7789_ShowString(50, 150, "4. System Info", Font_11x18, 
                   menu_selection == 3 ? RED : WHITE, BLACK);
  
  // 绘制操作提示
  ST7789_ShowString(30, 190, "KEY4/3:Select KEY1:OK", Font_7x10, YELLOW, BLACK);
  
  // 更新CPU和FPS信息
  UpdateCPUAndFPS();
}

/**
  * @brief  绘制温湿度监测界面
  * @param  无
  * @retval 无
  */
void DrawTempHumiScreen(void) {
	  // 清空历史数据数组
  memset(history, 0, sizeof(history));
  data_index = 0;
	
  ST7789_Clear(BLACK);  // 清屏，黑色背景
  
  // 显示标题
  ST7789_ShowString(50, 10, "Temp & Humi", Font_16x26, WHITE, BLACK);
  
  // 绘制图表区域
  DrawGraphGrid();
  
  // 显示当前数值
  DisplayCurrentValues();
  
  // 返回提示
  ST7789_ShowString(30, 290, "KEY2:Back to Menu", Font_7x10, YELLOW, BLACK);
  
  // 更新CPU和FPS信息
  UpdateCPUAndFPS();
}

/**
  * @brief  绘制ADC采集界面
  * @param  无
  * @retval 无
  */
void DrawADCScreen(void) {
  ST7789_Clear(BLACK);  // 清屏，黑色背景
  
  // 显示标题
  ST7789_ShowString(50, 10, "ADC", Font_16x26, WHITE, BLACK);
    
    // 显示ADC通道标签
    ST7789_ShowString(10, 70, "ADC1 (PA1):", Font_11x18, WHITE, BLACK);
    ST7789_ShowString(10, 100, "ADC1 Voltage:", Font_11x18, WHITE, BLACK);
    ST7789_ShowString(10, 130, "Temp Sensor:", Font_11x18, WHITE, BLACK);
    ST7789_ShowString(10, 160, "Temp Value:", Font_11x18, WHITE, BLACK);
    ST7789_ShowString(10, 190, "Vrefint:", Font_11x18, WHITE, BLACK);
    ST7789_ShowString(10, 220, "Vrefint Voltage:", Font_11x18, WHITE, BLACK);
	
	 
  
  // 返回提示
  ST7789_ShowString(30, 290, "KEY2:Back to Menu", Font_7x10, YELLOW, BLACK);
  
  // 更新CPU和FPS信息
  UpdateCPUAndFPS();
}


/**
  * @brief  绘制WiFi通信界面（显示基础信息）
  * @param  无
  * @retval 无
  * @note   显示WiFi名称、密码和IP地址，使用英文字符串
  */
void DrawWiFiScreen(void) {
    ST7789_Clear(BLACK);  // 清屏，黑色背景
    
    // 显示标题
    ST7789_ShowString(50, 10, "WiFi Config", Font_16x26, WHITE, BLACK);
    
    // 显示WiFi名称（SSID）
    ST7789_ShowString(5, 50, "SSID: 2003", Font_11x18, WHITE, BLACK);
    // 中文注释：WiFi网络名称
    
    // 显示WiFi密码
    ST7789_ShowString(5, 80, "PASS: 12121212", Font_11x18, WHITE, BLACK);
    // 中文注释：WiFi连接密码
    
    // 显示IP地址
    ST7789_ShowString(5, 110, "IP: 44.232.241.40", Font_11x18, WHITE, BLACK);
    // 中文注释：服务器IP地址
    
    // 显示功能提示（保留原有内容）
    ST7789_ShowString(5, 140, "WiFi Function", Font_11x18, WHITE, BLACK);
    ST7789_ShowString(5, 170, "Connectivity Module", Font_11x18, WHITE, BLACK);
    ST7789_ShowString(5, 200, "Wireless Data", Font_11x18, WHITE, BLACK);
    
    // 返回提示
    ST7789_ShowString(30, 290, "KEY2:Back to Menu", Font_7x10, YELLOW, BLACK);
    
    // 更新CPU和FPS信息
    UpdateCPUAndFPS();
}

/**
  * @brief  绘制系统信息界面
  * @param  无
  * @retval 无
  */
void DrawReadmeScreen(void) {
  ST7789_Clear(BLACK);  // 清屏，黑色背景
  
  // 显示标题
  ST7789_ShowString(50, 10, "System Info", Font_16x26, WHITE, BLACK);
  
  // 显示系统信息
  ST7789_ShowString(5, 50, "STM32F4", Font_11x18, WHITE, BLACK);
  ST7789_ShowString(5, 80, "Version: 2.0 Release", Font_11x18, WHITE, BLACK);
  ST7789_ShowString(5, 110, "Core Features:", Font_11x18, WHITE, BLACK);
  ST7789_ShowString(15, 140, "- Temperature", Font_11x18, WHITE, BLACK);
  ST7789_ShowString(15, 170, "- Humidity", Font_11x18, WHITE, BLACK);
  ST7789_ShowString(15, 200, "- ADC Data", Font_11x18, WHITE, BLACK);
  ST7789_ShowString(15, 230, "- Real-time", Font_11x18, WHITE, BLACK);
  
  // 显示项目描述
  ST7789_ShowString(5, 260, "Multi-function monitoring system", Font_7x10, YELLOW, BLACK);
  ST7789_ShowString(5, 275, "with graphical interface", Font_7x10, YELLOW, BLACK);
	
  // 返回提示
  ST7789_ShowString(30, 290, "KEY2:Back to Menu", Font_7x10, YELLOW, BLACK);
  
  // 更新CPU和FPS信息
  UpdateCPUAndFPS();
}

/**
  * @brief  更新CPU使用率和帧率信息
  * @param  无
  * @retval 无
  */
void UpdateCPUAndFPS(void) {
  // 计算FPS
  uint32_t current_time = HAL_GetTick();
  frame_count++;
  
  if (current_time - last_frame_time >= 1000) {
    fps = frame_count * 1000.0 / (current_time - last_frame_time);
    frame_count = 0;
    last_frame_time = current_time;
  }
  
  // 计算CPU使用率（简化版）
  if (current_time - last_idle_check >= 1000) {
    cpu_usage = 100.0 - (idle_time * 100.0 / (current_time - last_idle_check));
    idle_time = 0;
    last_idle_check = current_time;
  } else {
    idle_time++;
  }
  
  // 限制CPU使用率在0-100之间
  if (cpu_usage > 100.0) cpu_usage = 100.0;
  if (cpu_usage < 0.0) cpu_usage = 0.0;
  
  // 显示FPS和CPU使用率
  char info_str[30];
  snprintf(info_str, sizeof(info_str), "CPU:%.1f%% FPS:%.1f", cpu_usage, fps);
  ST7789_ShowString(100, 310, info_str, Font_7x10, WHITE, BLACK);
}

/**
  * @brief  绘制图表网格
  * @param  无
  * @retval 无
  */
void DrawGraphGrid(void) {
  // 清空图表区域
  ST7789_FillRect(10, 40, 230, 150, BLACK);
  
  // 绘制边框
  ST7789_DrawHLine(10, 230, 40, WHITE);
  ST7789_DrawHLine(10, 230, 150, WHITE);
  ST7789_DrawVLine(40, 150, 10, WHITE);
  ST7789_DrawVLine(40, 150, 230, WHITE);
  
  // 绘制网格线
  for (int i = 1; i < 5; i++) {
    uint16_t y = 40 + i * (110 / 5);
    ST7789_DrawHLine(15, 225, y, GRAY);
  }
  
  // 绘制刻度标签
  ST7789_ShowString(5, 35, "50", Font_7x10, WHITE, BLACK);
  ST7789_ShowString(5, 145, "00", Font_7x10, WHITE, BLACK);
  ST7789_ShowString(210, 145, "Time", Font_7x10, WHITE, BLACK);
}

/**
  * @brief  更新图表数据
  * @param  无
  * @retval 无
  */
void UpdateGraph(void) {
  static uint16_t last_x = 0;
  static uint16_t last_dht11_temp_y = 0;
  static uint16_t last_dht11_humi_y = 0;
  static uint16_t last_ds18b20_temp_y = 0;
	static uint8_t first_point = 1;  // 添加首次绘制标志
	
	 // 如果是新开始绘制，重置上一次的点
  if (data_index == 0) {
    last_x = 0;
    last_dht11_temp_y = 0;
    last_dht11_humi_y = 0;
    last_ds18b20_temp_y = 0;
    first_point = 1;
  }
  
  // 计算新数据点在图表中的位置
  uint16_t x = 10 + (data_index * 220) / MAX_HISTORY;
  uint16_t dht11_temp_y = 150 - (dht11_temperature * 110 / 50);
  uint16_t dht11_humi_y = 150 - (dht11_humidity * 110 / 100);
  uint16_t ds18b20_temp_y = 150 - ((uint16_t)ds18b20_temperature * 110 / 50);
  
 // 如果是第一个点，只绘制点不绘制线
  if (first_point) {
    // 绘制DHT11温度点 (红色)
    ST7789_DrawPixel(x, dht11_temp_y, RED);
    // 绘制DHT11湿度点 (蓝色)
    ST7789_DrawPixel(x, dht11_humi_y, BLUE);
    // 绘制DS18B20温度点 (绿色)
    ST7789_DrawPixel(x, ds18b20_temp_y, GREEN);
    
    first_point = 0;
  } else {
    // 绘制DHT11温度曲线 (红色)
    ST7789_DrawLine(last_x, last_dht11_temp_y, x, dht11_temp_y, RED);
    // 绘制DHT11湿度曲线 (蓝色)
    ST7789_DrawLine(last_x, last_dht11_humi_y, x, dht11_humi_y, BLUE);
    // 绘制DS18B20温度曲线 (绿色)
    ST7789_DrawLine(last_x, last_ds18b20_temp_y, x, ds18b20_temp_y, GREEN);
  }
  
  // 更新上一次的点
  last_x = x;
  last_dht11_temp_y = dht11_temp_y;
  last_dht11_humi_y = dht11_humi_y;
  last_ds18b20_temp_y = ds18b20_temp_y;
  
  // 如果数据已满，滚动图表
  if (data_index >= MAX_HISTORY - 1) {
    // 移动所有数据，为新数据腾出空间
    for (int i = 0; i < MAX_HISTORY - 1; i++) {
      history[i] = history[i+1];
    }
    data_index = MAX_HISTORY - 1;
    
    // 重新绘制整个图表
    DrawGraphGrid();
    for (int i = 1; i < data_index; i++) {
      uint16_t x1 = 10 + ((i-1) * 220) / MAX_HISTORY;
      uint16_t x2 = 10 + (i * 220) / MAX_HISTORY;
      
      // DHT11温度
      uint16_t dht11_temp_y1 = 150 - (history[i-1].dht11_temperature * 110 / 50);
      uint16_t dht11_temp_y2 = 150 - (history[i].dht11_temperature * 110 / 50);
      
      // DHT11湿度
      uint16_t dht11_humi_y1 = 150 - (history[i-1].dht11_humidity * 110 / 100);
      uint16_t dht11_humi_y2 = 150 - (history[i].dht11_humidity * 110 / 100);
      
      // DS18B20温度
      uint16_t ds18b20_temp_y1 = 150 - ((uint16_t)history[i-1].ds18b20_temperature * 110 / 50);
      uint16_t ds18b20_temp_y2 = 150 - ((uint16_t)history[i].ds18b20_temperature * 110 / 50);
      
      // 绘制线段
      ST7789_DrawLine(x1, dht11_temp_y1, x2, dht11_temp_y2, RED);
      ST7789_DrawLine(x1, dht11_humi_y1, x2, dht11_humi_y2, BLUE);
      ST7789_DrawLine(x1, ds18b20_temp_y1, x2, ds18b20_temp_y2, GREEN);
    }
  }
}

/**
  * @brief  显示当前传感器数值
  * @param  极无
  * @retval 无
  */
// 添加局部刷新功能
void ST7789_UpdateArea(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t* data)
{
    ST7789_Address_Set(x1, y1, x2, y2);
    uint32_t area_size = (x2 - x1 + 1) * (y2 - y1 + 1) * 2;
    ST7789_SendMultiByte(data, area_size);
}

// 修改显示函数，只更新变化部分
void DisplayCurrentValues(void)
{
    static uint16_t last_dht11_temp = 0;
    static uint16_t last_dht11_humi = 0;
    static float last_ds18b20_temp = 0.0;
    
    // 只有当值发生变化时才更新显示
    if (last_dht11_temp != dht11_temperature || 
        last_dht11_humi != dht11_humidity || 
        fabs(last_ds18b20_temp - ds18b20_temperature) > 0.1) {
        
        // 清空数值显示区域
        ST7789_FillRect(10, 160, 230, 220, BLACK);
        
        // 显示DHT11温度
        ST7789_ShowString(15, 165, "DHT11 Temp:", Font_7x10, WHITE, BLACK);
        char dht11_temp_str[10];
        snprintf(dht11_temp_str, sizeof(dht11_temp_str), "%.1f C", (float)dht11_temperature);
        ST7789_ShowString(120, 165, dht11_temp_str, Font_7x10, RED, BLACK);
        
        // 显示DHT11湿度
        ST7789_ShowString(15, 180, "DHT11 Humi:", Font_7x10, WHITE, BLACK);
        char dht11_humi_str[10];
        snprintf(dht11_humi_str, sizeof(dht11_humi_str), "%d %%", dht11_humidity);
        ST7789_ShowString(120, 180, dht11_humi_str, Font_7x10, BLUE, BLACK);
        
        // 显示DS18B20温度
        ST7789_ShowString(15, 195, "DS18B20 Temp:", Font_7x10, WHITE, BLACK);
        char ds18b20_temp_str[10];
        snprintf(ds18b20_temp_str, sizeof(ds18b20_temp_str), "%.1f C", ds18b20_temperature);
        ST7789_ShowString(120, 195, ds18b20_temp_str, Font_7x10, GREEN, BLACK);
        
        // 更新上一次的值
        last_dht11_temp = dht11_temperature;
        last_dht11_humi = dht11_humidity;
        last_ds18b20_temp = ds18b20_temperature;
    }
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
	

	
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_SPI1_Init();
  MX_TIM10_Init();
  MX_TIM9_Init();
  MX_TIM11_Init();
  MX_ADC1_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */

 ST7789_Init();        // 初始化TFT显示屏
  ST7789_Clear(BLACK);  // 清屏，黑色背景

  DHT11_Init();         // 初始化DHT11传感器
  Key_Init();           // 初始化按键
	
		Send_AT_Commands_Simple();
		
  HAL_TIM_Base_Start(&htim9);
  HAL_TIM_Base_Start_IT(&htim11);

  // 初始化菜单
  DrawMainMenu();
	
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  // 按键处理
    if (KeyNum == 4) 
			{ // 上键 - 选择上一个菜单项
				if (current_menu == MENU_MAIN) 
					{
						menu_selection = (menu_selection > 0) ? menu_selection - 1 : MENU_ITEMS_COUNT - 1;
						need_redraw = 1;
					}
				KeyNum = 0;
			} 
			else if (KeyNum == 3) 
				{ // 下键 - 选择下一个菜单项
				if (current_menu == MENU_MAIN) 
					{
						menu_selection = (menu_selection < MENU_ITEMS_COUNT - 1) ? menu_selection + 1 : 0;
						need_redraw = 1;
					}
				KeyNum = 0;
			} 
			else if (KeyNum == 1) 
				{ // 确认键 - 进入选中的菜单项
				if (current_menu == MENU_MAIN) 
					{
					switch (menu_selection) 
						{
						case 0: current_menu = MENU_TEMP_HUMI; break;
						case 1: current_menu = MENU_ADC; break;
						case 2: current_menu = MENU_WIFI; break;
						case 3: current_menu = MENU_README; break;
						}
						need_redraw = 1;
					}
				KeyNum = 0;
				} 
				else if (KeyNum == 2)
					{ // 返回键 - 返回主菜单
					if (current_menu != MENU_MAIN) 
						{
							current_menu = MENU_MAIN;
							need_redraw = 1;
						}
					KeyNum = 0;
					}
    
    // 根据当前菜单状态显示相应内容
    if (need_redraw) 
			{
      switch (current_menu) 
				{
					case MENU_MAIN:
						DrawMainMenu();
						break;
					case MENU_TEMP_HUMI:
						DrawTempHumiScreen();
						break;
					case MENU_ADC:
						DrawADCScreen();
						break;
					case MENU_WIFI:
						DrawWiFiScreen();
						break;
					case MENU_README:
						DrawReadmeScreen();
						break;
				}
      need_redraw = 0;
			}
    
    // 在温湿度界面中更新数据
    if (current_menu == MENU_TEMP_HUMI) 
			{
      // 读取传感器数据
				if (DHT11_Read_Data(&dht11_temperature, &dht11_humidity) == 0) 
					{
						ds18b20_temperature = ds18b20_getTemp();
        
        // 存储历史数据
						history[data_index].dht11_temperature = dht11_temperature;
						history[data_index].dht11_humidity = dht11_humidity;
						history[data_index].ds18b20_temperature = ds18b20_temperature;
						history[data_index].timestamp = HAL_GetTick();
        
        // 更新图表
						UpdateGraph();
        
        // 更新数值显示
						DisplayCurrentValues();
        
        // 增加数据索引
						data_index = (data_index + 1) % MAX_HISTORY;
					}
				}

		
		// 传感器读取
		if(duqu)
		{
			duqu=0;
			DHT11_Read_Data(&dht11_temperature, &dht11_humidity);
			ds18b20_temperature = ds18b20_getTemp();
		}
		if(shangchuan1 == 1)
		{
			shangchuan1=0;
					printf("AT+MQTTPUB=0,\"DHT11-TEMP\",\"{\\\"TEMP\\\\1\\\":%d}\",1,0\r\n", dht11_temperature);

		}
		
		if(shangchuan2 == 1)
		{
			shangchuan2=0;

					printf("AT+MQTTPUB=0,\"DHT11-HUMI\",\"{\\\"TEMP\\\\1\\\":%d}\",1,0\r\n", dht11_humidity);

		}
		
		if(shangchuan3 == 1)
		{
			shangchuan3=0;

					printf("AT+MQTTPUB=0,\"DS18B20\",\"{\\\"TEMP\\\\2\\\":%.1f}\",1,0\r\n", ds18b20_temperature);
		}
		
		
    if (sensor_read_pending && current_menu == MENU_TEMP_HUMI)
    {
        sensor_read_pending = 0;
        
        // 读取传感器数据
        if (DHT11_Read_Data(&dht11_temperature, &dht11_humidity) == 0)
        {
            ds18b20_temperature = ds18b20_getTemp();
            
            // 存储历史数据
            history[data_index].dht11_temperature = dht11_temperature;
            history[data_index].dht11_humidity = dht11_humidity;
            history[data_index].ds18b20_temperature = ds18b20_temperature;
            history[data_index].timestamp = HAL_GetTick();
            
            // 更新图表
            UpdateGraph();
            
            // 更新数值显示
            DisplayCurrentValues();
            
            // 增加数据索引
            data_index = (data_index + 1) % MAX_HISTORY;
				}
				
				    

			}
		if(sensor_read_pending && current_menu == MENU_ADC)
{
	sensor_read_pending = 0;
		
			 // 重新配置ADC为单通道，并读取每个通道
    ADC_ChannelConfTypeDef sConfig = {0};
    
    // 读取通道1 (PA1)
    sConfig.Channel = ADC_CHANNEL_1;
    sConfig.Rank = 1;
    sConfig.SamplingTime = ADC_SAMPLETIME_480CYCLES;
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);
    HAL_ADC_Start(&hadc1);
    if (HAL_ADC_PollForConversion(&hadc1, 100) == HAL_OK) {
        adc_values[0] = HAL_ADC_GetValue(&hadc1);
    }
    HAL_ADC_Stop(&hadc1);
    
    // 读取温度传感器通道
    sConfig.Channel = ADC_CHANNEL_TEMPSENSOR;
    sConfig.Rank = 1;
    sConfig.SamplingTime = ADC_SAMPLETIME_480CYCLES;
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);
    HAL_ADC_Start(&hadc1);
    if (HAL_ADC_PollForConversion(&hadc1, 100) == HAL_OK) {
        adc_values[1] = HAL_ADC_GetValue(&hadc1);
    }
    HAL_ADC_Stop(&hadc1);
    
    // 读取VREFINT通道
    sConfig.Channel = ADC_CHANNEL_VREFINT;
    sConfig.Rank = 1;
    sConfig.SamplingTime = ADC_SAMPLETIME_480CYCLES;
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);
    HAL_ADC_Start(&hadc1);
    if (HAL_ADC_PollForConversion(&hadc1, 100) == HAL_OK) {
        adc_values[2] = HAL_ADC_GetValue(&hadc1);
    }
    HAL_ADC_Stop(&hadc1);
    
    // 计算电压值
    // 使用VREFINT校准参考电压（假设VDDA为3.3V，但实际可能不准确）
    // 如果您有准确的VDDA，可以替换3.3f
    channel1_voltage = (adc_values[0] * 3.3f) / 4095.0f;
    float temp_voltage = (adc_values[1] * 3.3f) / 4095.0f;
    vrefint_voltage = (adc_values[2] * 3.3f) / 4095.0f;
    
    // 计算内部温度
    // V25 = 0.76V, Avg_Slope = 2.5mV/°C
    internal_temp = ((temp_voltage - 0.76f) / 0.0025f) + 25.0f;
    
    // 显示ADC原始值
    ShowADCValue(adc_values[0], 150, 70, Font_11x18, WHITE, BLACK);      // 通道1原始值
    ShowVoltage(channel1_voltage, 150, 100, Font_11x18, WHITE, BLACK);  // 通道1电压
    ShowADCValue(adc_values[1], 150, 130, Font_11x18, WHITE, BLACK);     // 温度传感器原始值
    ShowTemperature(internal_temp, 150, 160, Font_11x18, WHITE, BLACK); // 内部温度
    ShowADCValue(adc_values[2], 150, 190, Font_11x18, WHITE, BLACK);     // Vrefint原始值
    ShowVoltage(vrefint_voltage, 150, 250, Font_11x18, WHITE, BLACK);    // Vrefint电压0
		// 短暂延迟
		    HAL_Delay(10);
		
}
    // 更新CPU和FPS信息
    UpdateCPUAndFPS();
    
    // 短暂延迟，控制刷新率
    HAL_Delay(10);
    idle_time++;

  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 100;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
// 使用定时器中断进行传感器读取
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    static uint8_t sensor_read_counter = 0,s5 = 0,s10 = 0,s11=0,s12=0;
    
    if (htim->Instance == TIM11) // 20ms定时器
    {
        key_scan();
        count++;
        
        if (count == 50) // 1秒
        {
            count = 0;
					  s5++;
					s10++;
					s11++;
					s12++;
					sensor_read_counter++;
            HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
        }
        
        // 每2秒读取一次传感器（减少读取频率）

        if (sensor_read_counter >= 2) // 2秒
        {
            sensor_read_counter = 0;
//					printf("AT+MQTTPUB=0,\"8266_TX\",\"{\\\"TEMP\\\\1\\\":18}\",1,0\r\n");


            // 设置标志，在主循环中读取传感器
            sensor_read_pending = 1;
        }
				
        if (s5 >= 5) // 5秒
        {
            s5 = 0;

//					printf("AT+MQTTPUB=0,\"8266_TX\",\"{\\\"TEMP\\\\1\\\":%d}\",1,0\r\n", dht11_temperature);
//					printf("AT+MQTTPUB=0,\"8266_TX\",\"{\\\"HUMI\\\\1\\\":%d}\",1,0\r\n", dht11_humidity);
//					printf("AT+MQTTPUB=0,\"8266_TX\",\"{\\\"DS18B20\\\\2\\\":%.1f}\",1,0\r\n", ds18b20_temperature);

            // 设置标志，在主循环中读取传感器
            duqu = 1;

        }
				        if (s10 >= 10) // 5秒
        {
            s10 = 0;
            // 设置标志，在主环中读取传感器
            shangchuan1 = 1;
        }
				
				        if (s11 >= 11) // 5秒
        {
            s11 = 0;
            // 设置标志，在主循环中读取传感器
            shangchuan2 = 1;
        }
				
				        if (s12 >= 12) // 5秒
        {
            s12 = 0;
            // 设置标志，在主循环中读取传感器
            shangchuan3 = 1;
        }
    }
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
