/*
 *******************************************************************************
 *  [ssd1306.c]
 *  This module is making display pattern for SSD1306 OLED module (using I2C mode) .
 *
 *  This program is under the terms of the GPLv3.
 *  https://www.gnu.org/licenses/gpl-3.0.html
 *
 *  Copyright(c) 2019 Keshikan (www.keshikan.net)
 *******************************************************************************
 */

#include <stdio.h>
#include <stdlib.h>

#include "stm32h7xx_hal.h"
#include "st7735s_SPI.h"

#ifndef __ATTR_DTCMRAM
#define __ATTR_DTCMRAM   __attribute__ ((section(".DTCMRAM"))) __attribute__ ((aligned (4)))
#endif

#ifndef __ATTR_RAM_D3
#define __ATTR_RAM_D3	__attribute__ ((section(".RAM_D3"))) __attribute__ ((aligned (4)))
#endif

#ifndef __ATTR_RAM_D2
#define __ATTR_RAM_D2	__attribute__ ((section(".RAM_D2"))) __attribute__ ((aligned (4)))
#endif

#ifndef __ATTR_RAM_D1
#define __ATTR_RAM_D1	__attribute__ ((section(".RAM_D1"))) __attribute__ ((aligned (4)))
#endif

#ifndef __ATTR_RAM_D1_VRAM1
#define __ATTR_RAM_D1_VRAM1	__attribute__ ((section(".RAM_D1_VRAM1"))) __attribute__ ((aligned (4)))
#endif


//porting
extern SPI_HandleTypeDef SSD1306_SPI_HANDLER;

//uint16_t disp_memory_main[DISP_MEMORY_SIZE];
uint16_t disp_memory_buffer[DISP_MEMORY_SIZE] __ATTR_RAM_D1_VRAM1;
bool disp_transmitted=true;

//extern DMA_HandleTypeDef hdma_memtomem_dma2_stream0;

//semaphore
volatile bool display_updating_semaphore = false;


void displayError()
{
	while(1);
}



void dispBacklightOn()
{
	HAL_GPIO_WritePin(SSD1306_BK_GPIO, SSD1306_BK_PIN, GPIO_PIN_SET);
}

void dispBacklightOff()
{
	HAL_GPIO_WritePin(SSD1306_BK_GPIO, SSD1306_BK_PIN, GPIO_PIN_RESET);
}

void dispAssertCS()
{
#ifdef SPI_CS_POLAR_INVERT
	HAL_GPIO_WritePin(SSD1306_CS_GPIO, SSD1306_CS_PIN, GPIO_PIN_SET);
#else
	HAL_GPIO_WritePin(SSD1306_CS_GPIO, SSD1306_CS_PIN, GPIO_PIN_RESET);
#endif
}

void dispNegateCS()
{
#ifdef SPI_CS_POLAR_INVERT
	HAL_GPIO_WritePin(SSD1306_CS_GPIO, SSD1306_CS_PIN, GPIO_PIN_RESET);
#else
	HAL_GPIO_WritePin(SSD1306_CS_GPIO, SSD1306_CS_PIN, GPIO_PIN_SET);
#endif

}

void dispSelectDCtoData()
{
	HAL_GPIO_WritePin(SSD1306_DC_GPIO, SSD1306_DC_PIN, GPIO_PIN_SET);
}

void dispSelectDCtoCommand()
{
	HAL_GPIO_WritePin(SSD1306_DC_GPIO, SSD1306_DC_PIN, GPIO_PIN_RESET);
}

void dispSendReset()
{
	dispNegateCS();

	HAL_GPIO_WritePin(SSD1306_RESET_GPIO, SSD1306_RESET_PIN, GPIO_PIN_RESET);
	HAL_Delay(100);
	HAL_GPIO_WritePin(SSD1306_RESET_GPIO, SSD1306_RESET_PIN, GPIO_PIN_SET);
	HAL_Delay(100);

	dispSelectDCtoCommand();
	HAL_Delay(50);
}

void dispSendCommand16b(uint8_t dat)
{
	uint16_t dat16b = 0x00 + dat;
	//Command
	dispSelectDCtoCommand();
	asm("nop");asm("nop");asm("nop");asm("nop");

	//Chip Select
	dispAssertCS();
	asm("nop");asm("nop");asm("nop");asm("nop");

	HAL_SPI_Transmit(&SSD1306_SPI_HANDLER, (uint8_t *)(&dat16b), 1, 100);
}

void dispSendData16b(uint8_t dat1, uint8_t dat2)
{
	uint16_t dat16b = (dat1 << 8) + dat2;

	//Data
	dispSelectDCtoData();
	asm("nop");asm("nop");asm("nop");asm("nop");

	//Chip Select
	dispAssertCS();
	asm("nop");asm("nop");asm("nop");asm("nop");

	HAL_SPI_Transmit(&SSD1306_SPI_HANDLER, (uint8_t *)(&dat16b), 1, 100);

}

void dispClear(uint16_t* vram)
{
	uint32_t i;

	for(i=0; i< DISP_MEMORY_SIZE; i++){
		vram[i] = 0x0000;
	}
}

void displayInit()
{

	//Backlight off
	dispBacklightOff();

	//Clear VRAM
//	dispClear(disp_memory_main);
	dispClear(disp_memory_buffer);
	dispSendReset();


	//Initialize
	dispSendCommand16b(0x11);//Sleep exit
	HAL_Delay(130);
dispSendCommand16b(0x28);//disp off
	HAL_Delay(10);


	//Set color mode
	dispSendCommand16b(0x3A);
#ifdef COLOR_RGB444
	dispSendData16b(0x03,0x00);//RGB444
#else
	dispSendData16b(0x05,0x00);//RGB565
#endif



	//Porch Setting
	dispSendCommand16b(0xB2);
	dispSendData16b(0x0C,0x0C);
	dispSendData16b(0x00,0x33);
	dispSendData16b(0x33,0x00);

	//Memory Data Access Control MADCTL
	dispSendCommand16b(0x36);
	dispSendData16b(0b01100000, 0x00);

	//GateControl: add
	dispSendCommand16b(0xB7);
	dispSendData16b(0x35,0x00);


	//VCOM setting
	dispSendCommand16b(0xBB);
//	dispSendData16b(0x19, 0x00);
	dispSendData16b(0x1A, 0x00);

	//VRH
	dispSendCommand16b(0xC3);
	dispSendData16b(0x12, 0x00);

	//Power Control
	dispSendCommand16b(0xD0);
	dispSendData16b(0xA4, 0xA1);

	//Frame rate
	dispSendCommand16b(0xC6);
	dispSendData16b(0x0F, 0x00);

	//Display Brightness
	dispSendCommand16b(0x51);
	dispSendData16b(0xFF, 0x00);

	dispSendCommand16b(0x21);//display inversion


	//positive gamma: add
	dispSendCommand16b(0xE0);
	dispSendData16b(0x00, 0x03);
	dispSendData16b(0x07, 0x08);
	dispSendData16b(0x07, 0x15);
	dispSendData16b(0x2A, 0x44);
	dispSendData16b(0x42, 0x0A);
	dispSendData16b(0x17, 0x18);
	dispSendData16b(0x25, 0x27);

	//negative gamma: add
	dispSendCommand16b(0xE1);
	dispSendData16b(0x00, 0x03);
	dispSendData16b(0x08, 0x07);
	dispSendData16b(0x07, 0x23);
	dispSendData16b(0x2A, 0x43);
	dispSendData16b(0x42, 0x09);
	dispSendData16b(0x18, 0x17);
	dispSendData16b(0x25, 0x27);

	//Display on
	dispSendCommand16b(0x29);
	HAL_Delay(200);

	//set cursor
	dispSendCommand16b(0x2A);
	dispSendData16b(0x00, 0x00);
	dispSendData16b(0x01, 0x3F);//319px

	dispSendCommand16b(0x2B);
	dispSendData16b(0x00, 35+0x00);
	dispSendData16b(0x00, 35+0xA9);//169px

	dispSendCommand16b(0x2C);


	//Data sending
	dispSelectDCtoData();
	HAL_Delay(1);


	//clear
	for(uint32_t i=0; i<DISP_MEMORY_SIZE; i++){
		dispSendData16b(0x00, 0x00);
	}


	displayUpdate();

}


#ifdef MIDI_BARGRAPH_IS_DOT

//x=0-127, y=0-16
void displayDrawDot(uint16_t x, uint16_t y, uint16_t col)
{

	uint32_t pos = x + (y*VRAM_X_SIZE);

	disp_memory_buffer[pos*2] = (col & 0xFF00) >> 8;
	disp_memory_buffer[pos*2 + 1] = col & 0x00FF;

}



//ch:0-15
//dat:0-63
inline void displaySetDotGraph(uint8_t ch, uint8_t dat)
{
	uint16_t start_x = ch*8;
	uint16_t start_y = 79-dat;
	uint32_t vram_pos = start_x + (start_y*VRAM_X_SIZE);

	for(uint32_t i=0; i<7; i++){
		disp_memory_buffer[vram_pos*2] = (COL_FOREGROUND_565 & 0xFF00) >> 8;
		disp_memory_buffer[vram_pos*2 + 1] = COL_FOREGROUND_565 & 0x00FF;
		vram_pos++;
	}
}

//ch:0-15
//dat:0-63
inline void displaySetBarGraph(uint8_t ch, uint8_t dat)
{
	uint16_t start_x = ch*8;
	uint16_t start_y = 79-dat;
	uint32_t vram_pos = start_x + (start_y*VRAM_X_SIZE);

	for(uint32_t idx_y=start_y; idx_y<79; idx_y++){
		for(uint32_t i=0; i<7; i++){
			disp_memory_buffer[vram_pos*2] = (COL_FOREGROUND_565 & 0xFF00) >> 8;
			disp_memory_buffer[vram_pos*2 + 1] = COL_FOREGROUND_565 & 0x00FF;
			vram_pos++;
		}
		vram_pos += (VRAM_X_SIZE-7);
	}
}

void displayWipe()
{
	for(uint32_t i=0; i<DISP_MEMORY_SIZE; i++){
			disp_memory_buffer[i] = 0;
	}
}

#define DISP_BAR_MULTIPLY (1.0f)
#define DISP_PEAK_DOWN_SPEED (1)
#define DISP_PEAK_DOWN_DELAY (1)

uint32_t peak_down_cnt = 0;
//call from 10ms timer
void displayPeakDownValueCounter()
{
	peak_down_cnt++;
	peak_down_cnt &= 0xFFFFFFFFFFFFFFFF;
}

#else

#endif


// 16bit*3 = 12bit*4
//4pixel with 48bit alignment
//| RRRRGGGG BBBBRRRR | GGGGBBBB RRRRGGGG | BBBBRRRR GGGGBBBB |

#ifdef COLOR_RGB444
void displayDrawDot16b(uint16_t x, uint16_t y, uint16_t col565)
{

#ifdef METHOD_A

	uint32_t pos_linear = x + (y*VRAM_X_SIZE);
	uint32_t pos_16bit = (pos_linear * 3) >> 2;
	uint32_t pos_4bit =  pos_linear & 0x03;
	uint16_t col444 = CONV565TO444(col565);

	uint16_t temp;
	switch(pos_4bit){

		case 0x00:
			temp = disp_memory_buffer[pos_16bit] & 0x000F;
			disp_memory_buffer[pos_16bit] = (col444 << 4) + temp;
			break;

		case 0x01:
			temp = disp_memory_buffer[pos_16bit] & 0xFFF0;
			disp_memory_buffer[pos_16bit] = (col444 >> 8) + (temp);

			temp = disp_memory_buffer[pos_16bit+1] & 0x00FF;
			disp_memory_buffer[pos_16bit+1] = ((col444 & 0x00FF) << 8) + (temp);
			break;

		case 0x02:
			temp = disp_memory_buffer[pos_16bit] & 0xFF00;
			disp_memory_buffer[pos_16bit] = (col444 >> 4) + (temp);

			temp = disp_memory_buffer[pos_16bit+1] & 0x0FFF;
			disp_memory_buffer[pos_16bit+1] = ((col444 & 0x000F) << 12) + (temp);
			break;

		case 0x03:
			temp = disp_memory_buffer[pos_16bit] & 0xF000;
			disp_memory_buffer[pos_16bit] = col444 + (temp);
			break;

		default:
			break;
	}

#else
	uint32_t pos_bit = (x + (y*VRAM_X_SIZE)) * 12;
	uint16_t pos_16bit, pos_16bit_remain, memory_color, memory_other;
	uint16_t col444 = CONV565TO444(col565);


	for(uint32_t idx=0; idx<3; idx++){
		pos_16bit = pos_bit >> 4;
		pos_16bit_remain =  pos_bit & 0x0F;

		memory_color = (col444 & (0xF00 >> (idx*4))) >> (8-idx*4);
		memory_color = memory_color << (12 - pos_16bit_remain);
		memory_other = disp_memory_buffer[pos_16bit] & ~(0xF000 >> pos_16bit_remain);
		disp_memory_buffer[pos_16bit] = memory_other + memory_color;

		pos_bit += 4;
	}
#endif



}

#else
void displayDrawDot16b(uint16_t x, uint16_t y, uint16_t col565)
{
	uint32_t pos = x + (y*VRAM_X_SIZE);

	disp_memory_buffer[pos] = col565;
}

#endif


void displayWipe()
{
	for(uint32_t i=0; i<DISP_MEMORY_SIZE; i++){
			disp_memory_buffer[i] = 0;
	}
}

void displayUpdate()
{
	if(!display_updating_semaphore){
		display_updating_semaphore = true;
		HAL_SPI_Transmit_DMA(&SSD1306_SPI_HANDLER,(uint8_t *)disp_memory_buffer, DISP_MEMORY_SIZE);
	}
}

void displaySemaphoreClear()
{
	display_updating_semaphore = false;
}

void displayWaitDispalyTransfer()
{
	while(1)
	{
		if(!display_updating_semaphore){
			break;
		}
	}

}

bool displayisUpdating()
{
	return display_updating_semaphore;
}

