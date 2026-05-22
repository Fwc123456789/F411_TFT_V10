# F411_TFT_V10 — STM32F411 智能家居环境监测系统

基于 STM32F411CEU6 的 HAL 裸机工程，采用 **10ms 分时调度 + 状态机** 架构。

## 硬件平台

| 外设 | 型号/配置 |
|------|-----------|
| MCU | STM32F411CEU6 |
| TFT | ST7789V 320x240, SPI 50MHz + DMA |
| 温湿度 | DHT11 |
| 温度 | DS18B20 (OneWire) |
| Wi-Fi | ESP8266 (UART AT) |
| 按键 | 4键 (PB12/13/14/15) |
| ADC | PA1, 内部温度, VREFINT |
| 蜂鸣器 | PC13 |

## 架构特点

- **10ms 系统节拍**：基于 HAL_GetTick 软件节拍
- **9 任务分时调度**：TFT(20ms)、按键(10ms)、DHT11(2s)、DS18B20(1s)、ESP8266(10ms)、MQTT(2s)、ADC(100ms)、蜂鸣器(10ms)、日志(10ms)
- **DS18B20 非阻塞状态机**：替代 HAL_Delay(750ms)
- **ESP8266 AT 状态机**：替代 ~12秒阻塞初始化
- **TFT DMA 非阻塞刷新**：SPI 50MHz + DMA，busy 标志防重叠
- **UART 日志 FIFO**：printf 异步输出
- **按键事件队列 + UI 状态机**
- **CubeMX 兼容**：所有生成代码未被破坏

## 工程结构

```
F411_TFT_V10/
├── Core/
│   ├── Inc/          # 头文件 (CubeMX + app_*.h)
│   └── Src/          # 源文件 (CubeMX + app_*.c)
├── Chen_Drives/      # 用户驱动 (TFT, DHT11, DS18B20, Key, OneWire, Font)
├── Drivers/          # HAL库 + CMSIS
├── MDK-ARM/          # Keil MDK 工程
└── 重构说明文档.md    # 详细重构说明
```

## 编译

Keil MDK-ARM 打开 `MDK-ARM/F411_TFT_V2.uvprojx` 编译即可。

详见 `重构说明文档.md`。
