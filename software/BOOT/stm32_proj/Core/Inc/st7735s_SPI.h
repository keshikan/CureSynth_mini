/*
 *******************************************************************************
 *  [st7735s_SPI.h]
 *  This module is making display pattern for SSD1306 OLED module (using I2C mode) .
 *
 *  This program is under the terms of the GPLv3.
 *  https://www.gnu.org/licenses/gpl-3.0.html
 *
 *  Copyright(c) 2019 Keshikan (www.keshikan.net)
 *******************************************************************************
 */

#ifndef SSD1306_H_
#define SSD1306_H_

#include <stdbool.h>
#include <stdint.h>

//setting
#define COLOR_RGB444
//#define COLOR_RGB565

//12bit packing method normal is METHOD_A
#define METHOD_A
//#define METHOD_B

//SPI CS logic
//#define SPI_CS_POLAR_INVERT

//convert color
#define CONV888TO565(col) (((col&0xF80000)>>8) + ((col&0xFC00)>>5) + ((col&0xF8)>>3))
#define CONV565TO888(col) (((col&0b1111100000000000)<<8) + ((col&0b0000011111100000)<<5) + ((col&0b0000000000011111)<<3))
#define CONV565TO444(col) (((col&0b1111000000000000)>>4) + ((col&0b0000011110000000)>>3) + ((col&0b0000000000011110)>>1))

//setting
#define SSD1306_ADDR (0x78)
#define SSD1306_SPI_HANDLER hspi3
#define SSD1306_RESET_GPIO GPIOB
#define SSD1306_RESET_PIN GPIO_PIN_4
#define SSD1306_CS_GPIO GPIOG
#define SSD1306_CS_PIN GPIO_PIN_10
#define SSD1306_DC_GPIO GPIOG
#define SSD1306_DC_PIN GPIO_PIN_9
#define SSD1306_BK_GPIO GPIOB
#define SSD1306_BK_PIN GPIO_PIN_5

//display memory
//note: X and Y may be even value for RGB444 mode.
#define VRAM_X_SIZE (320)
#define VRAM_Y_SIZE (170)

//display memory size (byte)
#ifdef COLOR_RGB444
	#define DISP_MEMORY_SIZE ((VRAM_X_SIZE * VRAM_Y_SIZE * 3) >> 2)
#else
	#define DISP_MEMORY_SIZE (VRAM_X_SIZE * VRAM_Y_SIZE)
#endif

//color define
//#define COL_BACKGROUND (0x213a60)
//#define COL_FOREGROUND (0x8ebcfc)
#define COL_BACKGROUND (0x000000)
#define COL_FOREGROUND (0xFFFFFF)

//#define COL_BACKGROUND_565 (0x0883)
//#define COL_FOREGROUND_565 (0x5CFF)

#define COL_BACKGROUND_565 CONV888TO565(COL_BACKGROUND)
#define COL_FOREGROUND_565 CONV888TO565(COL_FOREGROUND)


//bargraph display types
//#define MIDI_BARGRAPH_IS_DOT

//display device type
#define DISP_TYPE_2

//display memory
extern uint16_t disp_memory_buffer[DISP_MEMORY_SIZE];

//table
extern uint8_t disp_settings[];
extern bool disp_transmitted;

//public func
extern void displayInit();
extern void dispSendReset();
//extern void dispSendCommand(uint8_t dat);
//extern void dispSendData(uint8_t dat);
//extern void displayDrawDot(uint16_t x, uint16_t y, uint16_t col);
extern void displayDMAMain();
extern void displayUpdate();
extern void displayWipe();


extern void dispBacklightOn();
extern void dispBacklightOff();

//16b
extern void dispSendCommand16b(uint8_t dat);
extern void dispSendData16b(uint8_t dat1, uint8_t dat2);
extern void displayDrawDot16b(uint16_t x, uint16_t y, uint16_t col);

//display semaphore
extern void displaySemaphoreClear();
extern void displayWaitDispalyTransfer();
extern bool displayisUpdating();

#endif /* SSD1306_H_ */

