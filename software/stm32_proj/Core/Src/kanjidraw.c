/*
 * kanjidraw.c
 *
 *  Created on: 2018/09/16
 *      Author: Keshikan
 */


#include "kanjidraw.h"
#include "sjis_font.h"

uint16_t kanji_color_foreground = 0xFFFF;
uint16_t kanji_color_background = 0x0000;


void kdSetForegroundColor(UG_COLOR col)
{
	kanji_color_foreground = col;
}

void kdSetBackgroundColor(UG_COLOR col)
{
	kanji_color_background = col;
}

//convert s-jis code to fontx pointer.
//cf.1 http://www.elm-chan.org/docs/dosv/fontx.html
//cf.2 https://sakura87.net/archives/2671
uint8_t* kdGetFontPtr(FontInfo* finfo)
{

	uint32_t block_cnt, code_sum = 0, ret_ptr;
	uint8_t* block_ptr;
	uint32_t start, end;

	// single byte character
	if(finfo->code < 0x100){
		finfo->width = sjis_1b_font[14];
		finfo->height = sjis_1b_font[15];
		finfo->size = (sjis_1b_font[14] + 7)/8 * sjis_1b_font[15];
		ret_ptr = 17 + finfo->code * finfo->size;
		return (uint8_t*)&sjis_1b_font[ret_ptr];
	}

	//double byte character
	block_ptr = (uint8_t*)&sjis_2b_font[18];
	block_cnt = sjis_2b_font[17];
	finfo->width = sjis_2b_font[14];
	finfo->height = sjis_2b_font[15];
	finfo->size = (sjis_2b_font[14] + 7)/8 * sjis_2b_font[15];

	while(block_cnt--){

		start = block_ptr[0] + (block_ptr[1] << 8);
		end = block_ptr[2] + (block_ptr[3] << 8);

		if(finfo->code >= start && finfo->code <= end){
			code_sum += finfo->code - start;
			ret_ptr = 18 + 4 * sjis_2b_font[17] + code_sum * finfo->size;
			return (uint8_t*)&sjis_2b_font[ret_ptr];
		}

		code_sum += end - start + 1;
		block_ptr += 4;

	}

	return 0;//error

}

//draw character(in finfo.code) to display
void kdDrawc(UG_S16 x, UG_S16 y, FontInfo* finfo)
{
	uint8_t* font = kdGetFontPtr(finfo);
	const uint32_t line_block = (finfo->width / 8) + ((finfo->width & 7) != 0);
	uint32_t font_pos = 0;

	UG_COLOR color;
	uint8_t is_dot_active;

	//error
	if(NULL == font){
		return;
	}

	for(uint32_t dh = 0; dh<finfo->height; dh++){			//height
		for(uint32_t dw = 0; dw<finfo->width; dw++){		//width

			uint32_t lbnum = dw / 8;	//current line number

			is_dot_active = (font[font_pos + lbnum] >> (7-(dw&7)) );
			color = (is_dot_active & 0x01) * kanji_color_foreground + (~is_dot_active & 0x01) * kanji_color_background;
			pset( x+dw, y+dh, color);

		}
		font_pos += line_block;

	}
}

//draw strings to display
void kdDrawStr(UG_S16 x, UG_S16 y, char* str)
{
	uint32_t byte_pos = 0, str_pos = 0;
	uint32_t cstate = 0;//0:ready, 1:wait_2byte,
	FontInfo finfo;
	bool is_max_strings = false;

		while('\0' != str[byte_pos] && !is_max_strings){

			switch(cstate){

			case 0:
				if( (str[byte_pos] >= 0x81 && str[byte_pos] <= 0x9f) || (str[byte_pos] >= 0xE0 && str[byte_pos] <= 0xFC) ){// when 2byte code
					cstate = 1;
					finfo.code = str[byte_pos] << 8;
				}else{// when 1byte code
					if(x+str_pos >= STR_WIDTH_LMT_1B){
						is_max_strings = true;
						break;
					}
					finfo.code = str[byte_pos];
					kdDrawc(x + str_pos, y, &finfo);
					str_pos += finfo.width;
				}
				break;

			case 1:
				if(x+str_pos >= STR_WIDTH_LMT_2B){
					is_max_strings = true;
					cstate = 0;
					break;
				}
				finfo.code += str[byte_pos];
				kdDrawc(x + str_pos, y, &finfo);
				str_pos += finfo.width;
				cstate = 0;
				break;

			default:
				cstate = 0;
				break;

			}
			byte_pos++;
		}
}
