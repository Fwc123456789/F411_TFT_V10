#include "TFT.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

 
/* at least 240*2 and can be divided wholely by 240*240*2 */
#define ST7789_BUF_SIZE (240 * 2)
uint8_t ST7789_Buf[ST7789_BUF_SIZE];
 
 // ��TFT.h������
#define USE_DMA_DOUBLE_BUFFER 1
#define DMA_BUFFER_SIZE 1024

// ��TFT.c���޸�
static uint8_t dma_buffer1[DMA_BUFFER_SIZE];
static uint8_t dma_buffer2[DMA_BUFFER_SIZE];
static volatile uint8_t active_buffer = 0;
volatile uint8_t tft_spi_dma_busy = 0;

void ST7789_DMA_Complete_Callback(void)
{
		tft_spi_dma_busy = 0;
	
    active_buffer = !active_buffer;
}

void ST7789_SendMultiByte_DMA(uint8_t* data, uint16_t len)
{
    while (tft_spi_dma_busy) {
        // �ȴ�ǰһ��DMA�������
        // ����������ִ����������
    }
    
    tft_spi_dma_busy = 1;
    
    if (len > DMA_BUFFER_SIZE) {
        // �����ݷֿ鴫��
        uint16_t remaining = len;
        uint16_t offset = 0;
        
        while (remaining > 0) {
            uint16_t chunk_size = (remaining > DMA_BUFFER_SIZE) ? DMA_BUFFER_SIZE : remaining;
            
            // �������ݵ���ǰ������
            memcpy(active_buffer ? dma_buffer1 : dma_buffer2, data + offset, chunk_size);
            
            ST7789_CS_LOW();
            ST7789_DC_HIGH();
            
            HAL_SPI_Transmit_DMA(ST7789_SPI, active_buffer ? dma_buffer1 : dma_buffer2, chunk_size);
            
            // �ȴ��������
            while (HAL_SPI_GetState(ST7789_SPI) == HAL_SPI_STATE_BUSY_TX);
            
            ST7789_CS_HIGH();
            
            offset += chunk_size;
            remaining -= chunk_size;
        }
    } else {
        // С����ֱ�Ӵ���
        memcpy(active_buffer ? dma_buffer1 : dma_buffer2, data, len);
        
        ST7789_CS_LOW();
        ST7789_DC_HIGH();
        
        HAL_SPI_Transmit_DMA(ST7789_SPI, active_buffer ? dma_buffer1 : dma_buffer2, len);
        
        // �ȴ��������
        while (HAL_SPI_GetState(ST7789_SPI) == HAL_SPI_STATE_BUSY_TX);
        
        ST7789_CS_HIGH();
    }
    
    tft_spi_dma_busy = 0;
    active_buffer = !active_buffer;
}

// �޸�ST7789_SendMultiByte����
void ST7789_SendMultiByte(uint8_t* dat, uint16_t len)
{
    if (len > 64) { // ���ڴ�����ʹ��DMA
        ST7789_SendMultiByte_DMA(dat, len);
    } else { // С����ʹ����������
        ST7789_CS_LOW();
        ST7789_DC_HIGH();
        HAL_SPI_Transmit(ST7789_SPI, dat, len, HAL_MAX_DELAY);
        ST7789_CS_HIGH();
    }
}
 
 
void ST7789_SendByte(uint8_t dat, ST7789_DCType DC)
{
   ST7789_CS_LOW();
 
   (DC == ST7789_DATA) ? ST7789_DC_HIGH() : ST7789_DC_LOW();
 
   HAL_SPI_Transmit_DMA(ST7789_SPI, &dat, 1);
   while (HAL_SPI_GetState(ST7789_SPI) == HAL_SPI_STATE_BUSY_TX);
 
   ST7789_CS_HIGH();
}
 
void ST7789_SendHalfWord(uint16_t dat)
{
   uint8_t da[2];
   ST7789_CS_LOW();
   ST7789_DC_HIGH();
 
   da[0] = dat >> 8;
   da[1] = dat & 0xFF;
 
   HAL_SPI_Transmit_DMA(ST7789_SPI, da, 2);
   while (HAL_SPI_GetState(ST7789_SPI) == HAL_SPI_STATE_BUSY_TX);
 
   ST7789_CS_HIGH();
}
 
//void ST7789_SendMultiByte(uint8_t* dat, uint16_t len)
//{
//   ST7789_CS_LOW();
//   ST7789_DC_HIGH();
// 
//   HAL_SPI_Transmit_DMA(ST7789_SPI, dat, len);
//   while (HAL_SPI_GetState(ST7789_SPI) == HAL_SPI_STATE_BUSY_TX);
// 
//   ST7789_CS_HIGH();
//}
 
 
void ST7789_Init(void)
{
   ST7789_BL_HIGH();
   ST7789_CS_HIGH();
 
   ST7789_RST_HIGH();
   ST7789_RST_LOW();
	HAL_Delay(1);
	ST7789_RST_HIGH();
	HAL_Delay(120);
 
   ST7789_SendByte(0x11, ST7789_CMD);	
 
   ST7789_SendByte(0x3A, ST7789_CMD);			
   ST7789_SendByte(0x55, ST7789_DATA);   //   5 565  6 666
 
   ST7789_SendByte(0xB2, ST7789_CMD);			
   ST7789_SendByte(0x0C, ST7789_DATA);
   ST7789_SendByte(0x0C, ST7789_DATA); 
   ST7789_SendByte(0x00, ST7789_DATA); 
   ST7789_SendByte(0x33, ST7789_DATA); 
   ST7789_SendByte(0x33, ST7789_DATA); 			
 
   ST7789_SendByte(0xB7, ST7789_CMD);			
   ST7789_SendByte(0x35, ST7789_DATA);
 
   ST7789_SendByte(0xBB, ST7789_CMD);			
   ST7789_SendByte(0x32, ST7789_DATA); //Vcom=1.35V
                                          
   ST7789_SendByte(0xC2, ST7789_CMD);			
   ST7789_SendByte(0x01, ST7789_DATA);
 
   ST7789_SendByte(0xC3, ST7789_CMD);			
   ST7789_SendByte(0x19, ST7789_DATA); //GVDD=4.8V 
                                          
   ST7789_SendByte(0xC4, ST7789_CMD);			
   ST7789_SendByte(0x20, ST7789_DATA); //VDV, 0x20:0v
 
   ST7789_SendByte(0xC6, ST7789_CMD);			
   ST7789_SendByte(0x0F, ST7789_DATA); //0x0F:60Hz        	
 
   ST7789_SendByte(0xD0, ST7789_CMD);			
   ST7789_SendByte(0xA4, ST7789_DATA);
   ST7789_SendByte(0xA1, ST7789_DATA); 											  												  																								
         
   ST7789_SendByte(0xE0, ST7789_CMD);     
   ST7789_SendByte(0xD0, ST7789_DATA);   
   ST7789_SendByte(0x08, ST7789_DATA);   
   ST7789_SendByte(0x0E, ST7789_DATA);   
   ST7789_SendByte(0x09, ST7789_DATA);   
   ST7789_SendByte(0x09, ST7789_DATA);   
   ST7789_SendByte(0x05, ST7789_DATA);   
   ST7789_SendByte(0x31, ST7789_DATA);   
   ST7789_SendByte(0x33, ST7789_DATA);   
   ST7789_SendByte(0x48, ST7789_DATA);   
   ST7789_SendByte(0x17, ST7789_DATA);   
   ST7789_SendByte(0x14, ST7789_DATA);   
   ST7789_SendByte(0x15, ST7789_DATA);   
   ST7789_SendByte(0x31, ST7789_DATA);   
   ST7789_SendByte(0x34, ST7789_DATA);   
 
   ST7789_SendByte(0xE1, ST7789_CMD);     
   ST7789_SendByte(0xD0, ST7789_DATA);   
   ST7789_SendByte(0x08, ST7789_DATA);   
   ST7789_SendByte(0x0E, ST7789_DATA);   
   ST7789_SendByte(0x09, ST7789_DATA);   
   ST7789_SendByte(0x09, ST7789_DATA);   
   ST7789_SendByte(0x15, ST7789_DATA);   
   ST7789_SendByte(0x31, ST7789_DATA);   
   ST7789_SendByte(0x33, ST7789_DATA);   
   ST7789_SendByte(0x48, ST7789_DATA);   
   ST7789_SendByte(0x17, ST7789_DATA);   
   ST7789_SendByte(0x14, ST7789_DATA);   
   ST7789_SendByte(0x15, ST7789_DATA);   
   ST7789_SendByte(0x31, ST7789_DATA);   
   ST7789_SendByte(0x34, ST7789_DATA);   
 
   ST7789_SendByte(0x21, ST7789_CMD);
 
   ST7789_SendByte(0x36, ST7789_CMD); //MX, MY, RGB mode 
#if (ST7789_ROTATION == 0)
   ST7789_SendByte(0x00, ST7789_DATA);
#elif (ST7789_ROTATION == 90)
   ST7789_SendByte(0x60, ST7789_DATA);//90
#elif (ST7789_ROTATION == 180)
   ST7789_SendByte(0xC0, ST7789_DATA);//180
#elif (ST7789_ROTATION == 270)
   ST7789_SendByte(0xA0, ST7789_DATA);//270
#endif
 
   ST7789_SendByte(0x2A, ST7789_CMD); //Column Address Set
   ST7789_SendByte(0x00, ST7789_DATA);
   ST7789_SendByte(0x00, ST7789_DATA); //0
   ST7789_SendByte(0x00, ST7789_DATA);
   ST7789_SendByte(0xEF, ST7789_DATA); //239
 
//   ST7789_SendByte(0x2B, ST7789_CMD); //Row Address Set
//   ST7789_SendByte(0x00, ST7789_DATA);
//   ST7789_SendByte(0x00, ST7789_DATA); //0
//   ST7789_SendByte(0x00, ST7789_DATA);
//   ST7789_SendByte(0xEF, ST7789_DATA); //239
	 
	 // �޸�Ϊ240x320���е�ַ���ã�0~319��
ST7789_SendByte(0x2B, ST7789_CMD); //Row Address Set
ST7789_SendByte(0x00, ST7789_DATA);
ST7789_SendByte(0x00, ST7789_DATA); //��ʼ�У�0
ST7789_SendByte(0x01, ST7789_DATA); //�����и�λ��0x01 = 256��
ST7789_SendByte(0x3F, ST7789_DATA); //�����е�λ��0x3F = 63��256+63=319��
 
   ST7789_SendByte(0x29, ST7789_CMD);
}
 
void ST7789_Address_Set(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2)
{
	if(ST7789_ROTATION==0)
	{
		ST7789_SendByte(0x2a, ST7789_CMD);//�е�ַ����
		ST7789_SendHalfWord(x1);
		ST7789_SendHalfWord(x2);
		ST7789_SendByte(0x2b, ST7789_CMD);//�е�ַ����
		ST7789_SendHalfWord(y1);
		ST7789_SendHalfWord(y2);
		ST7789_SendByte(0x2c, ST7789_CMD);//������д
	}
	else if(ST7789_ROTATION == 90)
	{
      
		ST7789_SendByte(0x2a, ST7789_CMD);//�е�ַ����
		ST7789_SendHalfWord(x1);
		ST7789_SendHalfWord(x2);
		ST7789_SendByte(0x2b, ST7789_CMD);//�е�ַ����
		ST7789_SendHalfWord(y1);
		ST7789_SendHalfWord(y2);
		ST7789_SendByte(0x2c, ST7789_CMD);//������д
	}
	else if(ST7789_ROTATION == 180)
	{
//      y1 += 80;
//	   y2 += 80;
		ST7789_SendByte(0x2a, ST7789_CMD);//�е�ַ����
		ST7789_SendHalfWord(x1);
		ST7789_SendHalfWord(x2);
		ST7789_SendByte(0x2b, ST7789_CMD);//�е�ַ����
		ST7789_SendHalfWord(y1);
		ST7789_SendHalfWord(y2);
		ST7789_SendByte(0x2c, ST7789_CMD);//������д
	}
	else if(ST7789_ROTATION == 270)
	{
//      x1 += 80;
//      x2 += 80;
		ST7789_SendByte(0x2a, ST7789_CMD);//�е�ַ����
		ST7789_SendHalfWord(x1);
		ST7789_SendHalfWord(x2);
		ST7789_SendByte(0x2b, ST7789_CMD);//�е�ַ����
		ST7789_SendHalfWord(y1);
		ST7789_SendHalfWord(y2);
		ST7789_SendByte(0x2c, ST7789_CMD);//������д
	}
   else
   {
   }
}
 
void ST7789_Clear(uint16_t color)
{
   uint16_t i,j;
   ST7789_Address_Set(0,0,ST7789_HEIGHT - 1,ST7789_WIDTH - 1);
 
   for ( i = 0; i < ST7789_BUF_SIZE; i += 2)
   {
      ST7789_Buf[i] = color >> 8;
      ST7789_Buf[i + 1] = color & 0xFF;
   }
   for ( i = 0; i < ST7789_HEIGHT * ST7789_WIDTH * 2 / ST7789_BUF_SIZE; i++)
   {
      ST7789_SendMultiByte(ST7789_Buf, ST7789_BUF_SIZE);
   }
}
 
void ST7789_DrawPixel(uint16_t x,uint16_t y,uint16_t color)
{
   ST7789_Address_Set(x,y,x,y);//���ù��λ�� 
   ST7789_SendHalfWord(color);
}
 
void ST7789_DrawHLine(uint16_t xs, uint16_t xe,uint16_t y,uint16_t color)
{
   uint16_t i,j;
   ST7789_Address_Set(xs,y,xe,y);//���ù��λ�� 
 
 
   for ( i = 0; i < (xe - xs + 1) * 2; i += 2)
   {
      ST7789_Buf[i] = color >> 8;
      ST7789_Buf[i + 1] = color & 0xFF;
   }
   ST7789_SendMultiByte(ST7789_Buf, (xe - xs + 1) * 2);
}
 
 
 
//void ST7789_DrawVLine(uint16_t ys, uint16_t ye, uint16_t x, uint16_t color)
//{
//   uint16_t i,j;
//   ST7789_Address_Set(x,ys,x,ye);//���ù��λ�� 
// 
//   for ( i = 0; i < (ye - ys + 1) * 2; i += 2)
//   {
//      ST7789_Buf[i] = color >> 8;
//      ST7789_Buf[i + 1] = color & 0xFF;
//   }
//   ST7789_SendMultiByte(ST7789_Buf, (ye - ys + 1) * 2);
//}

//void ST7789_DrawHLine(uint16_t xs, uint16_t xe, uint16_t y, uint16_t color)
//{
//    // ȷ����������Ч��Χ��
//    if (y >= ST7789_HEIGHT) return;
//    if (xs >= ST7789_WIDTH) return;
//    if (xe >= ST7789_WIDTH) xe = ST7789_WIDTH - 1;
//    if (xs > xe) {
//        // ���������յ�
//        uint16_t temp = xs;
//        xs = xe;
//        xe = temp;
//    }
//    
//    uint16_t length = xe - xs + 1;
//    
//    // ȷ�����Ȳ���������������
//    if (length > (ST7789_BUF_SIZE / 2)) {
//        length = ST7789_BUF_SIZE / 2;
//        xe = xs + length - 1;
//    }
//    
//    ST7789_Address_Set(xs, y, xe, y);  // ���ù��λ��
//    
//    // ��仺����
//    for (uint16_t i = 0; i < length * 2; i += 2)
//    {
//        ST7789_Buf[i] = color >> 8;
//        ST7789_Buf[i + 1] = color & 0xFF;
//    }
//    
//    // ��������
//    ST7789_SendMultiByte(ST7789_Buf, length * 2);
//}

void ST7789_DrawVLine(uint16_t ys, uint16_t ye, uint16_t x, uint16_t color)
{
   uint16_t i, j;
   uint32_t length = ye - ys + 1;
   ST7789_Address_Set(x, ys, x, ye);   // ��������Ϊ��(x,ys)��(x,ye)

   // ȷ��������Ч
   if (ys > ye) return;
   if (x >= ST7789_WIDTH) return;
   if (ye >= ST7789_HEIGHT) ye = ST7789_HEIGHT - 1;

   // ׼����ɫ���ݣ�2�ֽ�/���أ�
   uint8_t colorData[2] = {color >> 8, color & 0xFF};

   // �ֿ鷢������
   for (i = 0; i < length; i += (ST7789_BUF_SIZE/2)) 
   {
      uint16_t chunkSize = (length - i) > (ST7789_BUF_SIZE/2) ? 
                          (ST7789_BUF_SIZE/2) : 
                          (length - i);
      
      // ��仺����
      for (j = 0; j < chunkSize; j++) 
      {
          ST7789_Buf[j * 2] = colorData[0];
          ST7789_Buf[j * 2 + 1] = colorData[1];
      }
      
      // �������ݿ�
      ST7789_SendMultiByte(ST7789_Buf, chunkSize * 2);
   }
}
 
void ST7789_FillRect(uint16_t xs, uint16_t ys, uint16_t xe, uint16_t ye, uint16_t color)
{
   uint16_t i,j;
   uint32_t depth;
   depth = (ye - ys + 1) * (xe - xs + 1) * 2;
   ST7789_Address_Set(xs, ys, xe, ye);//���ù��λ�� 
 
   if (depth < ST7789_BUF_SIZE)
   {
      for ( i = 0; i < depth; i += 2)
      {
         ST7789_Buf[i] = color >> 8;
         ST7789_Buf[i + 1] = color & 0xFF;
      }
      ST7789_SendMultiByte(ST7789_Buf, depth);
   }
   else
   {
      for ( i = 0; i < ST7789_BUF_SIZE; i += 2)
      {
         ST7789_Buf[i] = color >> 8;
         ST7789_Buf[i + 1] = color & 0xFF;
      }
 
      for ( i = 0; i < depth / ST7789_BUF_SIZE; i++)
      {
         ST7789_SendMultiByte(ST7789_Buf, ST7789_BUF_SIZE);
      }
      
      if (depth % ST7789_BUF_SIZE != 0)
      {
         ST7789_SendMultiByte(ST7789_Buf, (depth % ST7789_BUF_SIZE));
      }
      else
      {
         
      }
   }
}
 
/**
 * xsize is in [0,239]
 */
void ST7789_DrawBitLine16BPP(uint16_t xs, uint16_t y, uint8_t const * p, uint16_t xsize)
{
   uint16_t i,j;
   ST7789_Address_Set(xs, y, xs + xsize - 1, y);//���ù��λ�� 
   for ( i = 0; i < xsize * 2; i+=2)
   {
      // ST7789_Buf[i] = *(p + i) >> 8;
      // ST7789_Buf[i + 1] = *(p + i + 1) & 0xFF;
      ST7789_Buf[i + 1] = *(p + i);
      ST7789_Buf[i] = *(p + i + 1);
   }
 
   ST7789_SendMultiByte(ST7789_Buf, xsize * 2);
}
 
 
 
void ST7789_DrawBitmap(uint16_t xs, uint16_t ys, uint16_t xsize, uint16_t ysize, uint8_t *p)
{
   uint16_t i,j;
   for ( i = 0; i < ysize; i++)
   {
      ST7789_DrawBitLine16BPP(xs, ys + i, p + i * xsize  * 2, xsize);
   }
}
 
void ST7789_ShowChar(uint16_t x, uint16_t y, uint8_t ch, FontDef font, uint16_t color, uint16_t bgcolor)
{
   uint32_t i, b, j;
   uint8_t data[2] = {0};
   ST7789_Address_Set(x, y, x + font.width - 1, y + font.height - 1);
 
   for (i = 0; i < font.height; i++) 
   {
		b = font.data[(ch - 32) * font.height + i];
		for (j = 0; j < font.width; j++) 
      {
			if ((b << j) & 0x8000) 
         {
				data[0] = color >> 8; 
            data[1] = color & 0xFF;
				ST7789_SendMultiByte(data, sizeof(data));
			}
			else 
         {
				data[0] = bgcolor >> 8; 
            data[1] = bgcolor & 0xFF;
				ST7789_SendMultiByte(data, sizeof(data));
			}
		}
	}
}
 
 
void ST7789_ShowString(uint16_t x, uint16_t y, const char *str, FontDef font, uint16_t color, uint16_t bgcolor)
{
   while (*str) 
   {
		if (x + font.width >= ST7789_WIDTH) 
      {
			x = 0;
			y += font.height;
			if (y + font.height >= ST7789_HEIGHT) 
         {
				break;
			}
			if (*str == ' ') 
         {
				str++;
				continue;
			}
		}
		ST7789_ShowChar(x, y, *str, font, color, bgcolor);
		x += font.width;
		str++;
	}
}


// ��ʾ����
void ST7789_ShowInt(uint16_t x, uint16_t y, int num, FontDef font, uint16_t color, uint16_t bgcolor)
{
    char buffer[12];  // �㹻�洢����
    snprintf(buffer, sizeof(buffer), "%d", num);
    ST7789_ShowString(x, y, buffer, font, color, bgcolor);
}

// ��ʾ������
void ST7789_ShowFloat(uint16_t x, uint16_t y, float num, uint8_t decimals, FontDef font, uint16_t color, uint16_t bgcolor)
{
    char buffer[20];  // �㹻�洢������
    char format[10];
    snprintf(format, sizeof(format), "%%.%df", decimals);
    snprintf(buffer, sizeof(buffer), format, num);
    ST7789_ShowString(x, y, buffer, font, color, bgcolor);
}

// ����ֱ�ߺ���
void ST7789_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color) {
    int16_t dx = abs(x2 - x1);
    int16_t dy = abs(y2 - y1);
    int16_t sx = (x1 < x2) ? 1 : -1;
    int16_t sy = (y1 < y2) ? 1 : -1;
    int16_t err = dx - dy;
    int16_t e2;
    
    while (1) {
        ST7789_DrawPixel(x1, y1, color);
        if (x1 == x2 && y1 == y2) break;
        e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
}
