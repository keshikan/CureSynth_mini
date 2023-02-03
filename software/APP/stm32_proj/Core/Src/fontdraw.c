/*
 * fontdraw.c
 *
 *  Created on: 2022/02/09
 *      Author: Keshikan
 */


#include "fontdraw.h"
#include "orig_font.h"

uint16_t orig_color_foreground = 0xFFFF;
uint16_t orig_color_background = 0x0000;



void fdSetForegroundColor(UG_COLOR col)
{
	orig_color_foreground = col;
}

void fdSetBackgroundColor(UG_COLOR col)
{
	orig_color_background = col;
}


void fdDrawc(UG_S16 x, UG_S16 y, char c)
{
	uint8_t font_pos;

	UG_COLOR color;
	uint8_t is_dot_active;

	if (0x7F <= c){
		//do nothing

	}else{

		//recognize as space
		if(0x20 >= c){
			c = 0x20;
		}

		font_pos = (uint8_t)(c - ORIG_FONT_OFFSET);

		for(volatile uint32_t dh = 0; dh < ORIG_FONT_HEIGHT; dh++){		//height
			for(volatile uint32_t dw = 0; dw <ORIG_FONT_WIDTH; dw++){	//width
				is_dot_active = ((original_font_narrow[font_pos][dh]) & (0x80 >> dw)) >> (7-dw);
				color = (is_dot_active & 0x01) * orig_color_foreground + (~is_dot_active & 0x01) * orig_color_background;
				pset( x+dw, y+dh, color);
			}
		}
	}
}

void fdDrawc_font2(UG_S16 x, UG_S16 y, char c)
{
	uint8_t font_pos;

	UG_COLOR color;
	uint8_t is_dot_active;

	if (0x7F <= c){
		//do nothing

	}else{

		//recognize as space
		if(0x20 >= c){
			c = 0x20;
		}

		font_pos = (uint8_t)(c - ORIG_FONT_OFFSET);

		for(volatile uint32_t dh = 0; dh < ORIG_FONT_HEIGHT; dh++){		//height
			for(volatile uint32_t dw = 3; dw <ORIG_FONT_WIDTH; dw++){	//width
				is_dot_active = ((original_font2[font_pos][dh]) & (0x80 >> dw)) >> (7-dw);
				color = (is_dot_active & 0x01) * orig_color_foreground + (~is_dot_active & 0x01) * orig_color_background;
				pset( x+dw-3, y+dh, color);
			}
		}
	}
}

//draw strings to display
void fdDrawStr(UG_S16 x, UG_S16 y, char* str)
{
	volatile uint32_t str_byte_pos = 0;

	uint16_t char_xpos = x;
	uint16_t char_ypos = y;

	while('\0' != str[str_byte_pos]){

		if(char_xpos + ORIG_FONT_WIDTH - 1 > FD_WIDTH_LMT){
			y += ORIG_FONT_HEIGHT;
			char_xpos = x;
		}

		if(char_ypos + ORIG_FONT_HEIGHT - 1 > FD_HEIGHT_LMT){
			break;
		}

		fdDrawc(char_xpos, y, str[str_byte_pos]);
		str_byte_pos++;
		char_xpos += ORIG_FONT_WIDTH;

	}
}

//draw strings to display
void fdDrawStr2(UG_S16 x, UG_S16 y, char* str)
{
	volatile uint32_t str_byte_pos = 0;

	uint16_t char_xpos = x;
	uint16_t char_ypos = y;

	while('\0' != str[str_byte_pos]){

		if(char_xpos + ORIG_FONT_WIDTH -3 - 1 > FD_WIDTH_LMT){
			y += ORIG_FONT_HEIGHT;
			char_xpos = x;
		}

		if(char_ypos + ORIG_FONT_HEIGHT - 1 > FD_HEIGHT_LMT){
			break;
		}

		fdDrawc_font2(char_xpos, y, str[str_byte_pos]);
		str_byte_pos++;
		char_xpos += ORIG_FONT_WIDTH-3;

	}
}

