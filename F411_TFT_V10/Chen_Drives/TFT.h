#ifndef ST7789_H
#define ST7789_H		
 
 
#include "main.h"
#include "gpio.h"
#include "spi.h"
#include "FO.h"
 
#define ST7789_ROTATION 0
#define ST7789_SPI &hspi1
 
#define ST7789_BUFFER
 
 
#define ST7789_WIDTH 320   // x方向 (水平)
#define ST7789_HEIGHT 240  // y方向 (垂直)
 
 
#define ST7789_RST_LOW()       HAL_GPIO_WritePin(RES_GPIO_Port,RES_Pin,GPIO_PIN_RESET)
#define ST7789_RST_HIGH()      HAL_GPIO_WritePin(RES_GPIO_Port,RES_Pin,GPIO_PIN_SET)
 
#define ST7789_CS_LOW()        HAL_GPIO_WritePin(CS_GPIO_Port,CS_Pin,GPIO_PIN_RESET)
#define ST7789_CS_HIGH()       HAL_GPIO_WritePin(CS_GPIO_Port,CS_Pin,GPIO_PIN_SET)
 
#define ST7789_DC_LOW()        HAL_GPIO_WritePin(DC_GPIO_Port,DC_Pin,GPIO_PIN_RESET)
#define ST7789_DC_HIGH()       HAL_GPIO_WritePin(DC_GPIO_Port,DC_Pin,GPIO_PIN_SET)
 
#define ST7789_BL_LOW()        HAL_GPIO_WritePin(BL_GPIO_Port,BL_Pin,GPIO_PIN_RESET)
#define ST7789_BL_HIGH()       HAL_GPIO_WritePin(BL_GPIO_Port,BL_Pin,GPIO_PIN_SET)
 
 
#define WHITE         	 0xFFFF
#define BLACK         	 0x0000	  
#define BLUE           	 0x001F  
#define BRED             0XF81F
#define GRED 			 0XFFE0
#define GBLUE			 0X07FF
#define RED           	 0xF800
#define MAGENTA       	 0xF81F
#define GREEN         	 0x07E0
#define CYAN          	 0x7FFF
#define YELLOW        	 0xFFE0
#define BROWN 			 0XBC40
#define BRRED 			 0XFC07
#define GRAY  			 0X8430
 
 
 
typedef enum
{
   ST7789_CMD,
   ST7789_DATA,
}ST7789_DCType;
 
typedef struct
{
   ST7789_DCType DC;
   uint8_t data;
}ST7789_InitSequenceType;
 
 
typedef struct {
   uint16_t XSize;
   uint16_t YSize;
   uint16_t BytesPerLine;
   uint16_t BitsPerPixel;
   const uint8_t * pData;
 } GUI_BITMAP;
 
 
void ST7789_Clear(uint16_t Color);
void ST7789_SendByte(uint8_t dat, ST7789_DCType DC);
void ST7789_SendHalfWord(uint16_t dat);
void ST7789_SendMultiByte(uint8_t* dat, uint16_t len);
void ST7789_Init(void);
void ST7789_DrawPixel(uint16_t x,uint16_t y,uint16_t color);
void ST7789_DrawHLine(uint16_t xs, uint16_t xe,uint16_t y,uint16_t color);
void ST7789_DrawVLine(uint16_t ys, uint16_t ye,uint16_t x,uint16_t color);
void ST7789_FillRect(uint16_t xs, uint16_t ys, uint16_t xe, uint16_t ye, uint16_t color);
void ST7789_DrawBitmap(uint16_t xs, uint16_t ys, uint16_t xsize, uint16_t ysize, uint8_t *p);
void ST7789_ShowChar(uint16_t x, uint16_t y, uint8_t ch, FontDef font, uint16_t color, uint16_t bgcolor);
void ST7789_ShowString(uint16_t x, uint16_t y, const char *str, FontDef font, uint16_t color, uint16_t bgcolor);
 
 
 // 在 ST7789.h 中添加这些声明
void ST7789_ShowInt(uint16_t x, uint16_t y, int num, FontDef font, uint16_t color, uint16_t bgcolor);
void ST7789_ShowFloat(uint16_t x, uint16_t y, float num, uint8_t decimals, FontDef font, uint16_t color, uint16_t bgcolor);
 void ST7789_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
 void ST7789_Address_Set(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2);
 
#endif
 
 