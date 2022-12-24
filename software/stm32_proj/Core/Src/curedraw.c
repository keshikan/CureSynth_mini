/*
 * curedraw.c
 *
 *  Created on: Feb 5, 2022
 *      Author: Keshikan
 *
 *      Encoding: Shift-JIS
 */


#include "curedraw.h"
#include "logo.h"
#include "stm32h7xx_hal.h"
#include <stdio.h>

#include "curesynth_setting.h"

#include "fontdraw.h"

#include <stdlib.h>

#define DISP_DIVIDED_Y_POS (21)
#define DISP_MEMORY_DIVIDED_HALFWORD ((VRAM_X_SIZE * DISP_DIVIDED_Y_POS * 2 * 3) >> 2)

//uint16_t disp_memory_background[DISP_MEMORY_SIZE];

uint16_t disp_memory_background_top[DISP_MEMORY_DIVIDED_HALFWORD];
uint16_t disp_memory_background_bottom[DISP_MEMORY_SIZE - DISP_MEMORY_DIVIDED_HALFWORD];

uint16_t curedraw_color_foreground = 0xFFFF;
uint16_t curedraw_color_background = 0x0000;
uint16_t curedraw_color_basecolor = 0x0000;

uint16_t* curedraw_color_level_fore = (uint16_t*)(level2);
uint16_t* curedraw_color_level_back = (uint16_t*)(level2_back);



//smoother
DispInfo dispinfo_reference, dispinfo_drawing;
uint8_t volume_ref[2] = {0, 0};
uint8_t volume_drawing[2] = {0, 0};

//16px, 24byte
uint8_t rgb444_16dot_box_pattern_on[24];
uint8_t rgb444_16dot_box_pattern_off[24];


//setting
DispDrawingSetting draw_setting;


typedef enum{
	BAR_COLOR_ON, BAR_COLOR_OFF
}BARGRAPH_COLOR_TYPE;

void pset(UG_S16 x, UG_S16 y, UG_COLOR col)
{
//	displayDrawDot16b(x, y, col);

	displayDrawDot16b(x*2, y*2, col);
	displayDrawDot16b(x*2+1, y*2, col);
	displayDrawDot16b(x*2, y*2+1, col);
	displayDrawDot16b(x*2+1, y*2+1, col);
}

void pset_tiny(UG_S16 x, UG_S16 y, UG_COLOR col)
{
	displayDrawDot16b(x, y, col);
}


void psetDirectBackgroundImageBottom(UG_S16 x, UG_S16 y, UG_COLOR col)
{
	x *= 2;
	y *= 2;

	for(uint32_t offset_y=0; offset_y<2; offset_y++){
		for(uint32_t offset_x=0; offset_x<2; offset_x++){


			uint32_t pos_linear = (x + offset_x) + ((y + offset_y)*VRAM_X_SIZE);
			uint32_t pos_16bit = (pos_linear * 3) >> 2;
			uint32_t pos_4bit =  pos_linear & 0x03;
			uint16_t col444 = CONV565TO444(col);

			uint16_t temp;
			switch(pos_4bit){

				case 0x00:
					temp = disp_memory_background_bottom[pos_16bit] & 0x000F;
					disp_memory_background_bottom[pos_16bit] = (col444 << 4) + temp;
					break;

				case 0x01:
					temp = disp_memory_background_bottom[pos_16bit] & 0xFFF0;
					disp_memory_background_bottom[pos_16bit] = (col444 >> 8) + (temp);

					temp = disp_memory_background_bottom[pos_16bit+1] & 0x00FF;
					disp_memory_background_bottom[pos_16bit+1] = ((col444 & 0x00FF) << 8) + (temp);
					break;

				case 0x02:
					temp = disp_memory_background_bottom[pos_16bit] & 0xFF00;
					disp_memory_background_bottom[pos_16bit] = (col444 >> 4) + (temp);

					temp = disp_memory_background_bottom[pos_16bit+1] & 0x0FFF;
					disp_memory_background_bottom[pos_16bit+1] = ((col444 & 0x000F) << 12) + (temp);
					break;

				case 0x03:
					temp = disp_memory_background_bottom[pos_16bit] & 0xF000;
					disp_memory_background_bottom[pos_16bit] = col444 + (temp);
					break;

				default:
					break;
			}
		}
	}

}

void displaySetColorNormal()
{
	fdSetBackgroundColor(curedraw_color_background);
	fdSetForegroundColor(curedraw_color_foreground);
}

//ch:0-15
//dat:0-63
inline void displaySetDotGraph(uint8_t ch, uint8_t dat)
{
	uint16_t start_x = ch*8;
	uint16_t start_y = 84-dat;

	UG_DrawLine(start_x,  start_y, start_x+6, start_y, curedraw_color_foreground);

}


#define BARGRAPH_VRAM_START_POS (20640)
#define BARGRAPH_VRAM_BOX_WIDTH_BYTE (24)
#define BARGRAPH_VRAM_LINE_WIDTH_BYTE (480)

//ch:0-15
//dat:0-15
inline void displaySetDotGraphRough(uint8_t ch, uint8_t dat, BARGRAPH_COLOR_TYPE ctype)
{
//	uint16_t start_x = ch*8;
//	uint16_t start_y = 82-dat*4;
//
//		if(ctype == BAR_COLOR_ON){
//			UG_FillFrame(start_x, start_y, start_x+6, start_y + 2, curedraw_color_foreground);
//		}else{
//			UG_FillFrame(start_x, start_y, start_x+6, start_y + 2, curedraw_color_background);
//		}



	uint32_t vram_8bit_pos = BARGRAPH_VRAM_START_POS + ch * BARGRAPH_VRAM_BOX_WIDTH_BYTE + BARGRAPH_VRAM_LINE_WIDTH_BYTE * (15-dat) * 8;
	uint8_t* color_pattern_ptr;

	if(ctype == BAR_COLOR_ON){
		color_pattern_ptr = rgb444_16dot_box_pattern_on;
	}else{
		color_pattern_ptr = rgb444_16dot_box_pattern_off;
	}

	for(uint32_t idx_height = 0; idx_height < 6; idx_height++)
	{
		for(uint32_t idx_line = 0; idx_line < 12; idx_line++){
			((uint8_t*)disp_memory_buffer)[vram_8bit_pos] = color_pattern_ptr[idx_line*2+1];
			((uint8_t*)disp_memory_buffer)[vram_8bit_pos+1] = color_pattern_ptr[idx_line*2];// invert msb and lsb
			vram_8bit_pos+=2;
		}
		vram_8bit_pos += BARGRAPH_VRAM_LINE_WIDTH_BYTE - BARGRAPH_VRAM_BOX_WIDTH_BYTE;
	}



}


#define BARGRAPH_TYPEB_VRAM_START_POS (20832)
#define BARGRAPH_TYPEB_VRAM_BOX_WIDTH_BYTE (12)
#define BARGRAPH_TYPEB_VRAM_LINE_WIDTH_BYTE (480)

//ch:0-15
//dat:0-15
inline void displaySetDotGraphRough_typeB(uint8_t ch, uint8_t dat, BARGRAPH_COLOR_TYPE ctype)
{

	uint32_t vram_8bit_pos = BARGRAPH_TYPEB_VRAM_START_POS + ch * BARGRAPH_TYPEB_VRAM_BOX_WIDTH_BYTE + BARGRAPH_TYPEB_VRAM_LINE_WIDTH_BYTE * (15-dat) * 8;
	uint8_t* color_pattern_ptr;

	if(ctype == BAR_COLOR_ON){
		color_pattern_ptr = rgb444_16dot_box_pattern_on;
	}else{
		color_pattern_ptr = rgb444_16dot_box_pattern_off;
	}

	for(uint32_t idx_height = 0; idx_height < 6; idx_height++)
	{
		for(uint32_t idx_line = 6; idx_line < 12; idx_line++){
			((uint8_t*)disp_memory_buffer)[vram_8bit_pos] = color_pattern_ptr[idx_line*2+1];
			((uint8_t*)disp_memory_buffer)[vram_8bit_pos+1] = color_pattern_ptr[idx_line*2];// invert msb and lsb
			vram_8bit_pos+=2;
		}
		vram_8bit_pos += BARGRAPH_TYPEB_VRAM_LINE_WIDTH_BYTE - BARGRAPH_TYPEB_VRAM_BOX_WIDTH_BYTE;
	}



}



//ch:0-15
//dat:0-63
inline void displaySetBarGraph(uint8_t ch, uint8_t dat)
{
	uint16_t start_x = ch*8;
	uint16_t start_y = 84-dat;

	UG_FillFrame(start_x, start_y, start_x+6, 84, curedraw_color_foreground);

}

//ch:0-15
//dat:0-15
inline void displaySetBarGraphRough(uint8_t ch, uint8_t dat, BARGRAPH_COLOR_TYPE ctype)
{

	for(uint32_t i=0; i<=dat; i++)
	{
		displaySetDotGraphRough(ch, i, ctype);
	}

}

//ch:0-15
//dat:0-15
inline void displaySetBarGraphRough_typeB(uint8_t ch, uint8_t dat, BARGRAPH_COLOR_TYPE ctype)
{

	for(uint32_t i=0; i<=dat; i++)
	{
		displaySetDotGraphRough_typeB(ch, i, ctype);
	}

}



#define DISP_BAR_MULTIPLY (1.2f)
#define DISP_PEAK_DOWN_SPEED (1)//must greater equal than 1
#define DISP_PEAK_DOWN_DELAY (6)

uint32_t peak_down_cnt = 0;
//call from 10ms timer
void cdPeakDownValueCounter()
{
	peak_down_cnt++;
	peak_down_cnt &= 0xFFFFFFFFFFFFFFFF;
}


void displaySetData(uint8_t dat[16])
{

	static uint8_t peak_data[16];
	static uint32_t peak_delay_counter = 0;


	//Draw bargraph
	for(uint32_t i=0; i<16; i++){
		dat[i] = __USAT(dat[i] * DISP_BAR_MULTIPLY, 6);
		displaySetBarGraph(i, dat[i]);
	}

	//Draw peak meter
	for(uint32_t i=0; i<16; i++){
		if( peak_data[i] < dat[i] ){
			peak_data[i] = dat[i];
		}
		displaySetDotGraph(i, peak_data[i]);
	}


	if(DISP_PEAK_DOWN_DELAY < peak_delay_counter){
		for(uint32_t i=0; i<16; i++){
			if( peak_data[i] > DISP_PEAK_DOWN_SPEED){
				peak_data[i] -= DISP_PEAK_DOWN_SPEED;
			}else{
				peak_data[i] = 0;
			}
		}
		peak_delay_counter = 0;
	}else{
		peak_delay_counter++;
	}

}

void displaySetDataRough(uint8_t dat[16])
{

	static uint8_t peak_data[16];
	static uint32_t peak_delay_counter = 0;

	uint8_t data_temporary;


	//Draw bargraph
	for(uint32_t i=0; i<16; i++){

		//Draw bargraph
		data_temporary = __USAT((dat[i] >> 2) * DISP_BAR_MULTIPLY , 4);
		displaySetBarGraphRough(i, data_temporary, BAR_COLOR_ON);

		//Draw peak meter
		if( peak_data[i] < data_temporary ){
			peak_data[i] = data_temporary;
		}
		displaySetDotGraphRough(i, peak_data[i], BAR_COLOR_ON);
	}


	if(DISP_PEAK_DOWN_DELAY < peak_delay_counter){
		for(uint32_t i=0; i<16; i++){
			if( peak_data[i] > DISP_PEAK_DOWN_SPEED){
				peak_data[i] -= DISP_PEAK_DOWN_SPEED;
			}else{
				peak_data[i] = 0;
			}
		}
		peak_delay_counter = 0;
	}else{
		peak_delay_counter++;
	}

}

void displaySetDataRough_typeB(uint8_t dat[16])
{

	static uint8_t peak_data[16];
	static uint32_t peak_delay_counter = 0;

	uint8_t data_temporary;


	//Draw bargraph
	for(uint32_t i=0; i<16; i++){

		//Draw bargraph
		data_temporary = __USAT((dat[i] >> 2) * DISP_BAR_MULTIPLY , 4);
		displaySetBarGraphRough_typeB(i, data_temporary, BAR_COLOR_ON);

		//Draw peak meter
		if( peak_data[i] < data_temporary ){
			peak_data[i] = data_temporary;
		}
		displaySetDotGraphRough_typeB(i, peak_data[i], BAR_COLOR_ON);
	}


	if(DISP_PEAK_DOWN_DELAY < peak_delay_counter){
		for(uint32_t i=0; i<16; i++){
			if( peak_data[i] > DISP_PEAK_DOWN_SPEED){
				peak_data[i] -= DISP_PEAK_DOWN_SPEED;
			}else{
				peak_data[i] = 0;
			}
		}
		peak_delay_counter = 0;
	}else{
		peak_delay_counter++;
	}

}

bool isBlackKey(uint8_t note_num)
{
	switch((note_num%12)){
		case 1:
		case 3:
		case 6:
		case 8:
		case 10:
			return true;

		default:
			break;
	}

	return false;
}

uint16_t pianoroll_ch_color[CHNUM]
							= {
									C_DEEP_SKY_BLUE, C_HOT_PINK, C_YELLOW, C_GREEN,
									C_LIGHT_BLUE, C_DARK_ORANGE, C_PALE_GOLDEN_ROD, C_LIGHT_SEA_GREEN,
									C_ROYAL_BLUE, C_LIGHT_PINK, C_DEEP_PINK, C_LIGHT_GREEN,
									C_RED, C_AQUA, C_PEACH_PUFF, C_LIGHT_CYAN
							};
void displaySetDataRough_PianoRoll(DispInfo *dp)
{

	uint32_t buf_addr_halfword;
	const uint32_t buf_row_halfword = (320*12)/16;
	const uint32_t pianoarea_row_halfword = (256*12)/16;

    //scrolling buffer
	buf_addr_halfword = (320*167*12)/16;

	for(uint32_t idx_row=0; idx_row<120; idx_row++){
		for(uint32_t idx_col=0; idx_col<pianoarea_row_halfword; idx_col++){
			disp_memory_buffer[buf_addr_halfword + buf_row_halfword*2 + idx_col] = disp_memory_buffer[buf_addr_halfword + idx_col];
		}
		buf_addr_halfword -= buf_row_halfword;
	}

	//delete 1 line
	UG_DrawLine(0, 20, 127, 20, curedraw_color_basecolor);
	UG_DrawLine(0, 24, 127, 24, C_BLACK);

	//draw base line
	static uint32_t baseline_counter = 0;
	if(15 == baseline_counter){
		UG_DrawLine(0, 24, 127, 24, CONV888TO565(0x303030));
		baseline_counter = 0;
	}
	baseline_counter++;

	//draw note
	for(uint32_t i=0; i<CHNUM; i++){

		for(uint32_t idx_note=0; idx_note<64; idx_note++){

			if( (((uint64_t)1 << idx_note) & dp->note_stat_lower64[i]) )
			{
				UG_DrawPixel(idx_note, 24, pianoroll_ch_color[i]);
			}

			if( (((uint64_t)1 << idx_note) & dp->note_stat_upper64[i]) )
			{
				UG_DrawPixel(idx_note+64, 24, pianoroll_ch_color[i]);
			}

		}
	}

	cdSetBackgroundBottom();

	//draw keyboard hammering effect
	for(uint32_t i=0; i<CHNUM; i++){

		for(uint32_t idx_note=0; idx_note<64; idx_note++){

			if( (((uint64_t)1 << idx_note) & dp->note_stat_lower64[i]) )
			{
				if(isBlackKey(idx_note)){
					UG_DrawLine(idx_note, 21, idx_note, 22, C_RED);
				}else{
					UG_DrawLine(idx_note, 21, idx_note, 23, C_RED);
				}
			}

			if( (((uint64_t)1 << idx_note) & dp->note_stat_upper64[i]) )
			{
				if(isBlackKey(idx_note+64)){
					UG_DrawLine(idx_note+64, 21, idx_note+64, 22, C_RED);
				}else{
					UG_DrawLine(idx_note+64, 21, idx_note+64, 23, C_RED);
				}
			}
		}
	}



}

#define DISP_WAVE_MULTIPLY_BIT (3)
#define DISP_WAVE_BUFFER_SIZE (384)
#define DISP_WAVE_WIDTH (128)
#define DISP_WAVE_TRIGGER_THR (6)
#define DISP_WAVE_TRIGGER_HYS (5)
#define CONV_AUDIO_RESCALE(c) ( (int16_t)(c >> (10-DISP_WAVE_MULTIPLY_BIT)) )
#define CONV_AUDIO_TO_WAVE_POS(c) ( (int16_t)((c >> (10-DISP_WAVE_MULTIPLY_BIT)) + 32 + 21) )

void displaySetDataRough_Wave(DispInfo *dp, int16_t* audio_buffer, volatile uint32_t audio_idx)
{

	int16_t draw_audio_buffer[DISP_WAVE_BUFFER_SIZE];
	uint16_t trigger_position = 0;
	bool pre_triggered = false;

	//store drawing buffer
	for(uint32_t i=0; i<DISP_WAVE_BUFFER_SIZE; i++){
		draw_audio_buffer[i] = (audio_buffer[audio_idx] + audio_buffer[audio_idx+1])/2;

		if(2 >= audio_idx){
			audio_idx = (OUT_BUFSIZE*2 -2);
		}else{
			audio_idx -= 2;
		}
	}

	//search trigger position
	for(uint32_t i=0; i<(DISP_WAVE_BUFFER_SIZE - DISP_WAVE_WIDTH); i++){

		if( ((DISP_WAVE_TRIGGER_THR - DISP_WAVE_TRIGGER_HYS) >=  CONV_AUDIO_RESCALE(draw_audio_buffer[i])) && (CONV_AUDIO_RESCALE(draw_audio_buffer[i] > 0)) ){
			pre_triggered = true;
		}

		if( ((DISP_WAVE_TRIGGER_THR) <=  CONV_AUDIO_RESCALE(draw_audio_buffer[i])) && pre_triggered ){
			trigger_position = i;
			break;
		}
	}

	//draw wave
	volatile uint16_t ypos, ypos_prev;

	ypos_prev = CONV_AUDIO_TO_WAVE_POS(draw_audio_buffer[trigger_position]);

	if(21 > ypos_prev){
		ypos_prev = 21;
	}
	if(84 < ypos_prev){
		ypos_prev = 84;
	}

	for(uint32_t i=0; i<(DISP_WAVE_WIDTH-1); i++){
		ypos = CONV_AUDIO_TO_WAVE_POS(draw_audio_buffer[trigger_position+i+1]);

		if(21 > ypos){
			ypos = 21;
		}
		if(84 < ypos){
			ypos = 84;
		}
			UG_DrawLine(i, ypos_prev, i+1, ypos, C_GOLD);
		ypos_prev = ypos;
	}

	//draw Triggered sign
	if(0 != trigger_position){
		fdSetBackgroundColor(C_BLACK);
		fdSetForegroundColor(C_GOLD);

		fdDrawStr2(112, 21, "T'D");

		fdSetBackgroundColor(curedraw_color_background);
		fdSetForegroundColor(curedraw_color_foreground);
	}else{
		fdSetBackgroundColor(C_BLACK);
		fdSetForegroundColor(C_DARK_GOLDEN_ROD);

		fdDrawStr2(112, 21, "T'D");

		fdSetBackgroundColor(curedraw_color_background);
		fdSetForegroundColor(curedraw_color_foreground);
	}

}

//ch:0(left) 1(right)
//dat:0-63
//inline void displaySetVolumeMeter(uint8_t ch, uint8_t dat)
//{
//	uint16_t start_x = 132+ch*14;
//	uint16_t start_y = 84-dat;
//
//	for(uint32_t idx_y=start_y; idx_y<84; idx_y++){
//		UG_DrawLine(start_x,  idx_y, start_x+11, idx_y, curedraw_color_level_fore[idx_y-21]);
//	}
//
//
//}

//ch:0(left) 1(right)
//dat:0-63
inline void displaySetVolumeMeter(uint8_t ch, uint8_t dat)
{
	uint16_t start_x = 132+ch*14;
	uint16_t start_y = 84-(dat*52/64);

	for(uint32_t idx_y=start_y; idx_y<84; idx_y++){
		UG_DrawLine(start_x,  idx_y, start_x+11, idx_y, curedraw_color_level_fore[idx_y-21]);
	}


}

#define DISP_VOICE_MAXSTATE_CONTINUE_TIME (40)
uint32_t max_voice_flag_deactivate_counter = 0;
uint32_t clipping_flag_deactivate_counter = 0;
inline void displaySetVoiceNumber(uint8_t dat, bool isMaxVoice, bool isVolumeClipped)
{
	char strbuf[8];

	if(isMaxVoice)
	{
		max_voice_flag_deactivate_counter = DISP_VOICE_MAXSTATE_CONTINUE_TIME;
	}

	if(isVolumeClipped)
	{
		clipping_flag_deactivate_counter = DISP_VOICE_MAXSTATE_CONTINUE_TIME;
	}

	if(0 < max_voice_flag_deactivate_counter)
	{
		fdSetBackgroundColor(curedraw_color_foreground);
		fdSetForegroundColor(curedraw_color_basecolor);

		UG_FillFrame(132, 21, 157, 31, curedraw_color_foreground);
		fdDrawStr2(133, 21, "OVER!");

		fdSetBackgroundColor(curedraw_color_background);
		fdSetForegroundColor(curedraw_color_foreground);

		max_voice_flag_deactivate_counter--;

	}else{

		fdSetBackgroundColor(curedraw_color_basecolor);

		sprintf(strbuf, "%02d/%02d", dat, OPNUM);
		fdDrawStr2(133, 21, strbuf);

		fdSetBackgroundColor(curedraw_color_background);

	}

	if(0 < clipping_flag_deactivate_counter)
	{
		fdSetBackgroundColor(curedraw_color_foreground);
		fdSetForegroundColor(curedraw_color_basecolor);

		UG_FillFrame(132, 21, 157, 31, curedraw_color_foreground);
		fdDrawStr2(133, 21, "CLIP!");

		fdSetBackgroundColor(curedraw_color_background);
		fdSetForegroundColor(curedraw_color_foreground);

		clipping_flag_deactivate_counter--;

	}

}

//draw usb status to display
void cdDrawUsbState(uint8_t state)
{
	if(draw_setting.japanese_mode_is_active){
		return;
	}

	switch(state){
		case 0:
			fdSetBackgroundColor(curedraw_color_foreground);
			fdSetForegroundColor(curedraw_color_background);
			UG_FillFrame(152, 0, 152, 9, curedraw_color_foreground);
			fdDrawStr2(153, 0, "U");
			break;
		case 1:
			UG_FillFrame(152, 0, 152, 9, curedraw_color_background);
			fdDrawStr2(153, 0, " ");
			break;
		case 2:
			UG_FillFrame(152, 0, 152, 9, curedraw_color_background);
			fdDrawStr2(153, 0, " ");
			break;
		default:
			break;
	}

	fdSetBackgroundColor(curedraw_color_background);
	fdSetForegroundColor(curedraw_color_foreground);
}

//when increase number, more faster.
#define DISP_BAR_DIFF_SPEED (3)
#define DISP_VOLUME_DIFF_SPEED (2)
void cdMakeIndicator(DispInfo *dp, uint8_t volume[2], int16_t* audio_buffer, volatile uint32_t audio_idx)
{

	int16_t distance_between_drawing_reference = 0;
	int16_t distance_between_volume_reference = 0;

	//set bargraph reference
	for(uint32_t i=0; i<DP_CHNUM; i++){
		dispinfo_reference.display_data[i] = dp->display_data[i];
	}


	//midi bargraph smoother by reference
	for(uint32_t i=0; i<DP_CHNUM; i++){
		distance_between_drawing_reference = (int16_t)dispinfo_reference.display_data[i] - (int16_t)dispinfo_drawing.display_data[i];

		if(0 == distance_between_drawing_reference ){
			//nothing
		}else if(0 < distance_between_drawing_reference){
			if(distance_between_drawing_reference < (DISP_BAR_DIFF_SPEED*4)){
				dispinfo_drawing.display_data[i] = dispinfo_reference.display_data[i];
			}else{
				dispinfo_drawing.display_data[i] += (DISP_BAR_DIFF_SPEED*4);
			}
//			dispinfo_drawing.display_data[i] = dispinfo_reference.display_data[i];

		}else{

			if((distance_between_drawing_reference * -1) < DISP_BAR_DIFF_SPEED){
				dispinfo_drawing.display_data[i] = dispinfo_reference.display_data[i];
			}else{
				dispinfo_drawing.display_data[i] -= DISP_BAR_DIFF_SPEED;
			}
		}
	}

	//set volume reference
	for(uint32_t i=0; i<2; i++){
		volume_ref[i] = volume[i];
	}

	//volume meter smoother
	for(uint32_t i=0; i<2; i++){
		distance_between_volume_reference = (int16_t)volume_ref[i] - (int16_t)volume_drawing[i];

		if(0 == distance_between_volume_reference ){
			//nothing
		}else if(0 < distance_between_volume_reference){
//			if(distance_between_volume_reference < (DISP_VOLUME_DIFF_SPEED)){
//				volume_drawing[i] = volume_ref[i];
//			}else{
//				volume_drawing[i] += (DISP_VOLUME_DIFF_SPEED);
//			}
			volume_drawing[i] = volume_ref[i];
		}else{
			if((distance_between_volume_reference * -1) < DISP_VOLUME_DIFF_SPEED){
				volume_drawing[i] = volume_ref[i];
			}else{
				volume_drawing[i] -= DISP_VOLUME_DIFF_SPEED;
			}
		}
	}

	cdGetBackgroundBottom();

	switch(draw_setting.type)
	{
	case DISP_UI_TYPE_A:
		displaySetDataRough(dispinfo_drawing.display_data);
		break;
	case DISP_UI_TYPE_B:
		displaySetDataRough_typeB(dispinfo_drawing.display_data);
		break;
	case DISP_UI_PIANOROLL:
		displaySetDataRough_PianoRoll(dp);
		break;
	case DISP_UI_WAVE:
		displaySetDataRough_Wave(dp, audio_buffer, audio_idx);
		break;
	default:
		break;
	}

	displaySetVolumeMeter(0, volume_drawing[1]);
	displaySetVolumeMeter(1, volume_drawing[0]);

	displaySetVoiceNumber(dp->voice, dp->voice_is_max, dp->volume_is_clipping);



	//display testdata
//	uint8_t display_test[16] = {0x3F,0x05,0x19,0x10,0x20,0x2F,0x0F,0x18,0x32,0x24,0x07,0x06,0x2A,0x1F,0x17,0x10};
//	displaySetDataRough(display_test);
//	displaySetVolumeMeter(0, 30);
//	displaySetVolumeMeter(1, 63);
//	displaySetVoiceNumber(25, false, false);
}

void cdSetBackgroundTop()
{

	for(uint32_t i=0; i<DISP_MEMORY_DIVIDED_HALFWORD; i++){
			disp_memory_background_top[i] = disp_memory_buffer[i];
	}

}

void cdSetBackgroundTopUpper()
{
	for(uint32_t i=0; i<(DISP_MEMORY_DIVIDED_HALFWORD>>1); i++){
			 disp_memory_background_top[i] = disp_memory_buffer[i];
	}

}
void cdSetBackgroundTopDowner()
{
	for(uint32_t i=(DISP_MEMORY_DIVIDED_HALFWORD>>1); i<DISP_MEMORY_DIVIDED_HALFWORD; i++){
			disp_memory_background_top[i] = disp_memory_buffer[i];
	}

}

void cdSetBackgroundBottom()
{

	for(uint32_t i=0; i<(DISP_MEMORY_SIZE - DISP_MEMORY_DIVIDED_HALFWORD); i++){
			disp_memory_background_bottom[i] = disp_memory_buffer[i + DISP_MEMORY_DIVIDED_HALFWORD];
	}

}

//top = topUpper+topBottom (half)
//all = top + bottom
void cdGetBackgroundTop()
{
	for(uint32_t i=0; i<DISP_MEMORY_DIVIDED_HALFWORD; i++){
			disp_memory_buffer[i] = disp_memory_background_top[i];
	}
}

void cdGetBackgroundTopUpper()
{
	for(uint32_t i=0; i<(DISP_MEMORY_DIVIDED_HALFWORD>>1); i++){
			  disp_memory_buffer[i] = disp_memory_background_top[i];
	}

}
void cdGetBackgroundTopDowner()
{
	for(uint32_t i=(DISP_MEMORY_DIVIDED_HALFWORD>>1); i<DISP_MEMORY_DIVIDED_HALFWORD; i++){
			disp_memory_buffer[i] = disp_memory_background_top[i];
	}

}



void cdGetBackgroundBottom()
{

	for(uint32_t i=0; i<(DISP_MEMORY_SIZE - DISP_MEMORY_DIVIDED_HALFWORD); i++){
			disp_memory_buffer[i + DISP_MEMORY_DIVIDED_HALFWORD] = disp_memory_background_bottom[i];
	}

}


void cdSetLevelColor(uint16_t color_pattern_fore[64], uint16_t color_pattern_back[64])
{
	curedraw_color_level_fore = color_pattern_fore;
	curedraw_color_level_back = color_pattern_back;
}

void cdSetForegroundColor(UG_COLOR col)
{
	curedraw_color_foreground = col;
}

void cdSetBackgroundColor(UG_COLOR col)
{
	curedraw_color_background = col;
}

void cdSetBaseColor(UG_COLOR col)
{
	curedraw_color_basecolor = col;
}

void cdSet12bitBoxPatternArray(UG_COLOR col_front, UG_COLOR col_base, uint8_t rgb444_16dot_pat[])
{
	//14px
	uint8_t rgb444_2dot_box_pattern_fill[3];
	//2px
	uint8_t rgb444_2dot_box_pattern_back[3];

	rgb444_2dot_box_pattern_fill[0] = (uint8_t)((CONV565TO444(col_front) & 0xFF0)>>4);
	rgb444_2dot_box_pattern_fill[1] = (uint8_t)((CONV565TO444(col_front) & 0x00F)<<4) + (uint8_t)((CONV565TO444(col_front) & 0xF00)>>8);
	rgb444_2dot_box_pattern_fill[2] = (uint8_t)(CONV565TO444(col_front) & 0x0FF);

	rgb444_2dot_box_pattern_back[0] = (uint8_t)((CONV565TO444(col_base) & 0xFF0)>>4);
	rgb444_2dot_box_pattern_back[1] = (uint8_t)((CONV565TO444(col_base) & 0x00F)<<4) + (uint8_t)((CONV565TO444(col_base) & 0xF00)>>8);
	rgb444_2dot_box_pattern_back[2] = (uint8_t)(CONV565TO444(col_base) & 0x0FF);


	uint32_t ptr = 0;

	for(uint32_t i=0; i<7; i++){
		rgb444_16dot_pat[ptr] = rgb444_2dot_box_pattern_fill[0];
		rgb444_16dot_pat[ptr+1] = rgb444_2dot_box_pattern_fill[1];
		rgb444_16dot_pat[ptr+2] = rgb444_2dot_box_pattern_fill[2];
		ptr += 3;
	}

	for(uint32_t i=0; i<1; i++){
		rgb444_16dot_pat[ptr] = rgb444_2dot_box_pattern_back[0];
		rgb444_16dot_pat[ptr+1] = rgb444_2dot_box_pattern_back[1];
		rgb444_16dot_pat[ptr+2] = rgb444_2dot_box_pattern_back[2];
		ptr += 3;
	}

}

void cdSet12bitBoxPattern(UG_COLOR col_front, UG_COLOR col_back, UG_COLOR col_base)
{
	cdSet12bitBoxPatternArray(col_front, col_base, rgb444_16dot_box_pattern_on);
	cdSet12bitBoxPatternArray(col_back, col_base, rgb444_16dot_box_pattern_off);
}


void cdDrawText(char* str)
{
	if(draw_setting.japanese_mode_is_active){
		return;
	}
//	cdGetBackgroundTop();
	UG_FillFrame(0, 0, 159, 19, curedraw_color_background);
	fdDrawStr(0, 0, str);
}


void cdDrawDisplayFaceImageStruct(DisplayImageSetting *dsetting)
{
	uint32_t img_addr;
	for(uint32_t idx_y=0; idx_y < dsetting->height; idx_y++){
		for(uint32_t idx_x=0; idx_x < dsetting->width; idx_x++){
			img_addr = idx_x + idx_y * dsetting->width;
			if(((dsetting->startx + idx_x) < (VRAM_X_SIZE/2)) && ((dsetting->starty + idx_y) < (VRAM_Y_SIZE/2))){
				pset(dsetting->startx + idx_x, dsetting->starty + idx_y, dsetting->image_address[img_addr]);
			}
		}
	}
}


void cdDrawDisplayFaceImageStructToBottomBackground(DisplayImageSetting *dsetting)
{
	uint32_t img_addr;

	if(DISP_UI_TYPE_B != draw_setting.type)
	{
		return;
	}

	for(uint32_t idx_y=0; idx_y < dsetting->height; idx_y++){
		for(uint32_t idx_x=0; idx_x < dsetting->width; idx_x++){
			uint16_t xpos = dsetting->startx + idx_x;
			uint16_t ypos = dsetting->starty + idx_y - DISP_DIVIDED_Y_POS;
			img_addr = idx_x + idx_y * dsetting->width;
			psetDirectBackgroundImageBottom(xpos, ypos, dsetting->image_address[img_addr]);

		}
	}
}

#define VOL_ADJ_X (18)
#define VOL_ADJ_Y (39)

void cdDrawRotaryVolume(uint8_t volume_section)
{
	char strbuf[15];

	if(volume_section >= 40){
		volume_section = 40;
	}

	displayWipe();
	UG_FillFrame(0, 0, 159, 19, curedraw_color_background);
	UG_FillFrame(0, 20, 159, 84, curedraw_color_basecolor);

	if(0 < volume_section){
		sprintf(strbuf, "VOLUME: %02d", volume_section);
	}else{
		sprintf(strbuf, "VOLUME: MUTE");
	}

	fdDrawStr(32, 0, strbuf);

	for(uint32_t i=0; i<40; i++){
		UG_FillFrame(VOL_ADJ_X+i*3, VOL_ADJ_Y+31, VOL_ADJ_X+i*3+1, VOL_ADJ_Y+31 - i, curedraw_color_background);
	}

	for(uint32_t i=0; i<volume_section; i++){
		UG_FillFrame(VOL_ADJ_X+i*3, VOL_ADJ_Y+31, VOL_ADJ_X+i*3+1, VOL_ADJ_Y+31 - i, curedraw_color_foreground);
	}
}

bool cdIsJpMode()
{
	return draw_setting.japanese_mode_is_active;
}


void cdChangeToEnMode(){
    UG_FillFrame(0, 0, 159, 19, curedraw_color_background);
    cdDrawText(DISP_DEFAULT_STRING);
    cdSetBackgroundTop();

}

void cdChangeToJpMode()
{
    UG_FillFrame(0, 0, 159, 19, curedraw_color_background);
    cdSetBackgroundTop();

    draw_setting.cursor_upper = 0;
    draw_setting.cursor_lower = 0;

}


void curedrawInit()
{
	dpinfoInit(&dispinfo_drawing);
	dpinfoInit(&dispinfo_reference);

	//curedraw color
	cdSetForegroundColor(COL_FOREGROUND_565);
	cdSetBackgroundColor(COL_BACKGROUND_565);
	cdSetBaseColor(COL_BASE_565);
	cdSet12bitBoxPattern(curedraw_color_foreground, curedraw_color_background, curedraw_color_basecolor);

	//kanji color
	kdSetForegroundColor(COL_FOREGROUND_565);
	kdSetBackgroundColor(COL_BACKGROUND_565);

	//fontdraw color
	fdSetForegroundColor(COL_FOREGROUND_565);
	fdSetBackgroundColor(COL_BACKGROUND_565);


	//init draw settings
	draw_setting.type = DISP_UI_TYPE_A;
	draw_setting.japanese_mode_is_active = false;

	draw_setting.eyebrow_type = FACE_EYEBROW_1;
	draw_setting.eye_type = FACE_EYE_1_BLINK;
	draw_setting.mouth_type = FACE_MOUTH_DEFAULT;


	draw_setting.backimage_setting.image_address = back_default;
	draw_setting.backimage_setting.height = FACE_BASE_HEIGHT;
	draw_setting.backimage_setting.width = FACE_BASE_WIDTH;
	draw_setting.backimage_setting.startx = FACE_BASE_START_ABS_X;
	draw_setting.backimage_setting.starty = FACE_BASE_START_ABS_Y;

	for(uint32_t i=0; i<FACE_EYEBROW_NUM; i++){
		draw_setting.eyebrow_setting[i].height = FACE_EYEBROW_HEIGHT;
		draw_setting.eyebrow_setting[i].width = FACE_EYEBROW_WIDTH;
		draw_setting.eyebrow_setting[i].startx = FACE_BASE_START_ABS_X + FACE_EYEBROW_START_X;
		draw_setting.eyebrow_setting[i].starty = FACE_BASE_START_ABS_Y + FACE_EYEBROW_START_Y;
	}

	draw_setting.eyebrow_setting[FACE_EYEBROW_1].image_address = eyebrow_1;
	draw_setting.eyebrow_setting[FACE_EYEBROW_2].image_address = eyebrow_2;
	draw_setting.eyebrow_setting[FACE_EYEBROW_3].image_address = eyebrow_3;
	draw_setting.eyebrow_setting[FACE_EYEBROW_4].image_address = eyebrow_4;

	for(uint32_t i=0; i<FACE_EYE_NUM; i++){
		draw_setting.eye_setting[i].height = FACE_EYE_HEIGHT;
		draw_setting.eye_setting[i].width = FACE_EYE_WIDTH;
		draw_setting.eye_setting[i].startx = FACE_BASE_START_ABS_X + FACE_EYE_START_X;
		draw_setting.eye_setting[i].starty = FACE_BASE_START_ABS_Y + FACE_EYE_START_Y;
	}

	draw_setting.eye_setting[FACE_EYE_1_BLINK].image_address = eye_1_blink;
	draw_setting.eye_setting[FACE_EYE_2_BLINK].image_address = eye_2_blink;
	draw_setting.eye_setting[FACE_EYE_3_BLINK].image_address = eye_3_blink;
	draw_setting.eye_setting[FACE_EYE_4].image_address = eye_4;
	draw_setting.eye_setting[FACE_EYE_5].image_address = eye_5;
	draw_setting.eye_setting[FACE_EYE_6].image_address = eye_6;
	draw_setting.eye_setting[FACE_EYE_7].image_address = eye_7;

	for(uint32_t i=0; i<FACE_MOUTH_NUM; i++){
		draw_setting.mouth_setting[i].height = FACE_MOUTH_HEIGHT;
		draw_setting.mouth_setting[i].width = FACE_MOUTH_WIDTH;
		draw_setting.mouth_setting[i].startx = FACE_BASE_START_ABS_X + FACE_MOUTH_START_X;
		draw_setting.mouth_setting[i].starty = FACE_BASE_START_ABS_Y + FACE_MOUTH_START_Y;
	}

	draw_setting.mouth_setting[FACE_MOUTH_DEFAULT].image_address = mouth_default;
	draw_setting.mouth_setting[FACE_MOUTH_A].image_address = mouth_a;
	draw_setting.mouth_setting[FACE_MOUTH_I].image_address = mouth_i;
	draw_setting.mouth_setting[FACE_MOUTH_U].image_address = mouth_u;
	draw_setting.mouth_setting[FACE_MOUTH_E].image_address = mouth_e;
	draw_setting.mouth_setting[FACE_MOUTH_O].image_address = mouth_o;
	draw_setting.mouth_setting[FACE_MOUTH_PEKO].image_address = mouth_peko;
	draw_setting.mouth_setting[FACE_MOUTH_BLAME].image_address = mouth_blame;
	draw_setting.mouth_setting[FACE_MOUTH_MID].image_address = mouth_mid;


}

void curedrawBackgroundChangeToTypeA()
{
    UG_FillFrame(0, 20, 159, 84, curedraw_color_basecolor);

    for(uint32_t i=0; i<DP_CHNUM; i++){
    	displaySetBarGraphRough(i, 15, BAR_COLOR_OFF);
    	displaySetDotGraphRough(i, 0, BAR_COLOR_ON);
    }

    for(uint32_t i=0; i<63; i++){
    	UG_DrawLine(132, 21+i, 143, 21+i, curedraw_color_level_back[i] );
    	UG_DrawLine(146, 21+i, 157, 21+i, curedraw_color_level_back[i] );
    }
    for(uint32_t i=0; i<2; i++){
    	UG_DrawLine(132+i*14, 84, 143+i*14, 84, curedraw_color_level_fore[63] );
    }

    UG_DrawLine(129, 21, 129, 84, CONV888TO565(0x4C4C4C));

    cdSetBackgroundBottom();

}

void curedrawBackgroundChangeToTypeB()
{
    UG_FillFrame(0, 20, 159, 84, curedraw_color_basecolor);

    for(uint32_t i=0; i<DP_CHNUM; i++){
    	displaySetBarGraphRough_typeB(i, 15, BAR_COLOR_OFF);
    	displaySetDotGraphRough_typeB(i, 0, BAR_COLOR_ON);
    }

    for(uint32_t i=0; i<63; i++){
    	UG_DrawLine(132, 21+i, 143, 21+i, curedraw_color_level_back[i] );
    	UG_DrawLine(146, 21+i, 157, 21+i, curedraw_color_level_back[i] );
    }
    for(uint32_t i=0; i<2; i++){
    	UG_DrawLine(132+i*14, 84, 143+i*14, 84, curedraw_color_level_fore[63] );
    }

    UG_DrawLine(129, 21, 129, 84, CONV888TO565(0x4C4C4C));


    cdDrawDisplayFaceImageStruct(&(draw_setting.backimage_setting));

    cdSetBackgroundBottom();

}


void curedrawBackgroundChangeToPianoRoll()
{
    UG_FillFrame(0, 20, 128, 84, curedraw_color_basecolor);

    //draw keyboard background
    DisplayImageSetting keyboard_img_setting;
    keyboard_img_setting.image_address = keyboard;
    keyboard_img_setting.height = IMG_KEY_HIGHT;
    keyboard_img_setting.width = IMG_KEY_WIDTH;
    keyboard_img_setting.startx = 0;
    keyboard_img_setting.starty = 21;

    for(uint32_t i=0; i<11; i++){
    	cdDrawDisplayFaceImageStruct(&keyboard_img_setting);
    	keyboard_img_setting.startx += 12;
    }
    UG_FillFrame(129, 20, 159, 84, curedraw_color_basecolor);


    for(uint32_t i=0; i<63; i++){
    	UG_DrawLine(132, 21+i, 143, 21+i, curedraw_color_level_back[i] );
    	UG_DrawLine(146, 21+i, 157, 21+i, curedraw_color_level_back[i] );
    }
    for(uint32_t i=0; i<2; i++){
    	UG_DrawLine(132+i*14, 84, 143+i*14, 84, curedraw_color_level_fore[63] );
    }

    UG_DrawLine(129, 21, 129, 84, CONV888TO565(0x4C4C4C));

    cdSetBackgroundBottom();

}

void curedrawBackgroundChangeToWave()
{
	UG_FillFrame(0, 20, 159, 84, curedraw_color_basecolor);

	UG_DrawLine(0, 21+32, 127, 21+32, CONV888TO565(0x303030));
//	UG_DrawLine(0, 21+32-10, 127, 21+32-10, CONV888TO565(0x303030));
//	UG_DrawLine(0, 21+32+10, 127, 21+32+10, CONV888TO565(0x303030));


    for(uint32_t i=0; i<63; i++){
    	UG_DrawLine(132, 21+i, 143, 21+i, curedraw_color_level_back[i] );
    	UG_DrawLine(146, 21+i, 157, 21+i, curedraw_color_level_back[i] );
    }
    for(uint32_t i=0; i<2; i++){
    	UG_DrawLine(132+i*14, 84, 143+i*14, 84, curedraw_color_level_fore[63] );
    }

    UG_DrawLine(129, 21, 129, 84, CONV888TO565(0x4C4C4C));

    cdSetBackgroundBottom();

}


void curedrawBackgroundInit()
{

    UG_FillScreen(curedraw_color_basecolor);
    UG_FillFrame(0, 0, 159, 19, curedraw_color_background);


    for(uint32_t i=0; i<DP_CHNUM; i++){
    	displaySetBarGraphRough(i, 15, BAR_COLOR_OFF);
    	displaySetDotGraphRough(i, 0, BAR_COLOR_ON);
    }


    //    for(uint32_t i=0; i<63; i++){
    for(uint32_t i=11; i<63; i++){
    	UG_DrawLine(132, 21+i, 143, 21+i, curedraw_color_level_back[i] );
    	UG_DrawLine(146, 21+i, 157, 21+i, curedraw_color_level_back[i] );
    }
    for(uint32_t i=0; i<2; i++){
    	UG_DrawLine(132+i*14, 84, 143+i*14, 84, curedraw_color_level_fore[63] );
    }

    UG_DrawLine(129, 21, 129, 84, CONV888TO565(0x4C4C4C));

    cdSetBackgroundTop();
    cdSetBackgroundBottom();

//    sprintf(display_upper_string, DISP_DEFAULT_STRING);

}

#define EYE_BLINK_WAIT (4)
#define EYE_STATE_LENGTH (72)


void cdDrawEyeBlinking()
{

	static bool isBlinkEnded = false;

	if(!draw_setting.eyeblink_is_active){
		isBlinkEnded = true;
		return;
	}

	static uint16_t wait_counter=0;
	static uint16_t eye_state = 0, eye_state_previous=(EYE_STATE_LENGTH - 1);

	const DisplayFaceImageEyeType eyestate_pattern[EYE_STATE_LENGTH]
		= {FACE_EYE_1_BLINK, FACE_EYE_2_BLINK, FACE_EYE_3_BLINK, FACE_EYE_1_BLINK,
		   FACE_EYE_1_BLINK, FACE_EYE_1_BLINK, FACE_EYE_1_BLINK, FACE_EYE_1_BLINK,
		   FACE_EYE_1_BLINK, FACE_EYE_1_BLINK, FACE_EYE_1_BLINK, FACE_EYE_1_BLINK,
		   FACE_EYE_1_BLINK, FACE_EYE_1_BLINK, FACE_EYE_1_BLINK, FACE_EYE_1_BLINK,
		   FACE_EYE_1_BLINK, FACE_EYE_1_BLINK, FACE_EYE_1_BLINK, FACE_EYE_1_BLINK,
		   FACE_EYE_1_BLINK, FACE_EYE_1_BLINK, FACE_EYE_1_BLINK, FACE_EYE_1_BLINK,
		   FACE_EYE_1_BLINK, FACE_EYE_1_BLINK, FACE_EYE_1_BLINK, FACE_EYE_1_BLINK,
		   FACE_EYE_1_BLINK, FACE_EYE_1_BLINK, FACE_EYE_1_BLINK, FACE_EYE_1_BLINK,
		   FACE_EYE_1_BLINK, FACE_EYE_1_BLINK, FACE_EYE_1_BLINK, FACE_EYE_2_BLINK,
		   FACE_EYE_3_BLINK, FACE_EYE_1_BLINK, FACE_EYE_2_BLINK, FACE_EYE_3_BLINK,
		   FACE_EYE_1_BLINK, FACE_EYE_1_BLINK, FACE_EYE_1_BLINK, FACE_EYE_1_BLINK,
		   FACE_EYE_1_BLINK, FACE_EYE_1_BLINK, FACE_EYE_1_BLINK, FACE_EYE_1_BLINK,
		   FACE_EYE_1_BLINK, FACE_EYE_1_BLINK, FACE_EYE_1_BLINK, FACE_EYE_1_BLINK,
		   FACE_EYE_1_BLINK, FACE_EYE_1_BLINK, FACE_EYE_1_BLINK, FACE_EYE_1_BLINK,
		   FACE_EYE_1_BLINK, FACE_EYE_1_BLINK, FACE_EYE_1_BLINK, FACE_EYE_1_BLINK,
		   FACE_EYE_1_BLINK, FACE_EYE_1_BLINK, FACE_EYE_1_BLINK, FACE_EYE_1_BLINK,
		   FACE_EYE_1_BLINK, FACE_EYE_1_BLINK, FACE_EYE_1_BLINK, FACE_EYE_1_BLINK,
		   FACE_EYE_1_BLINK, FACE_EYE_1_BLINK, FACE_EYE_1_BLINK, FACE_EYE_1_BLINK,
			};

	if(isBlinkEnded){
		eye_state = 0;
		eye_state_previous=(EYE_STATE_LENGTH - 1);
		isBlinkEnded = false;
	}

	if(EYE_BLINK_WAIT < wait_counter){
		if(eyestate_pattern[eye_state_previous] != eyestate_pattern[eye_state]){
			cdDrawDisplayFaceImageStructToBottomBackground(&(draw_setting.eye_setting[ eyestate_pattern[eye_state] ]));
		}
		wait_counter = 0;
		eye_state_previous = eye_state;
		eye_state++;
		eye_state %= EYE_STATE_LENGTH;
	}

	wait_counter++;

}

#define COL_MONO_ALPHA (40)//alpha value. max255.
//todo
void cdSetDisplayThemaMonoColor(uint16_t input_basecolor_565)
{
	uint16_t col_base_565, col_fore_565, col_back_565;
	uint32_t input_basecolor_888 = CONV565TO888(input_basecolor_565);
	uint8_t input_r8 = (input_basecolor_888 & 0xFF0000) >> 16;
	uint8_t input_g8 = (input_basecolor_888 & 0x00FF00) >> 8;
	uint8_t input_b8 = (input_basecolor_888 & 0x0000FF);

	col_base_565 = input_basecolor_565;
	col_fore_565 = CONV888TO565(0x000000);

	input_r8 = input_r8 - ( input_r8 * COL_MONO_ALPHA / 255 );
	input_g8 = input_g8 - ( input_g8 * COL_MONO_ALPHA / 255 );
	input_b8 = input_b8 - ( input_b8 * COL_MONO_ALPHA / 255 );

	col_back_565 = CONV888TO565( ( ((uint32_t)input_r8 << 16) + ((uint32_t)input_g8 << 8) + input_b8) );


	//curedraw color
	cdSetForegroundColor(col_fore_565);
	cdSetBackgroundColor(col_back_565);
	cdSetBaseColor(col_base_565);
	cdSet12bitBoxPattern(curedraw_color_foreground, curedraw_color_background, curedraw_color_basecolor);

	//kanji color
	kdSetForegroundColor(col_fore_565);
	kdSetBackgroundColor(col_back_565);

	//fontdraw color
	fdSetForegroundColor(col_fore_565);
	fdSetBackgroundColor(col_back_565);

	//volume meter
	for(uint32_t i=0; i<64; i++){
		level_free_back[i] = col_back_565;
	}

	cdSetLevelColor((uint16_t*)level_free, (uint16_t*)level_free_back);


	switch(draw_setting.type)
	{
		case DISP_UI_TYPE_A:
			curedrawBackgroundChangeToTypeA();
			UG_FillFrame(0, 0, 159, 19, curedraw_color_background);
			cdSetBackgroundTop();
			break;
		case DISP_UI_TYPE_B:
			curedrawBackgroundChangeToTypeB();
			UG_FillFrame(0, 0, 159, 19, curedraw_color_background);
			cdSetBackgroundTop();
			break;
		case DISP_UI_PIANOROLL:
			curedrawBackgroundChangeToPianoRoll();
			UG_FillFrame(0, 0, 159, 19, curedraw_color_background);
			cdSetBackgroundTop();
			break;
		case DISP_UI_WAVE:
			curedrawBackgroundChangeToWave();
			UG_FillFrame(0, 0, 159, 19, curedraw_color_background);
			cdSetBackgroundTop();
			break;
		default:
			break;

	}
}

#define COL_HSV_S (170)
#define COL_HSV_V (255)

void cdSetDisplayGamingMode()
{
	static uint16_t hsv_h = 0;

	uint8_t max = COL_HSV_V;
	uint8_t min = max - (((COL_HSV_S*max)/255));
	uint8_t col_r=0;
	uint8_t col_g=0;
	uint8_t col_b=0;

	if(hsv_h <= 60){
		col_r = max;
		col_g = ((hsv_h) * (max-min)) / 60 + min;
		col_b = min;

	}else if(hsv_h <= 120){
		col_r = ((120-hsv_h) * (max-min)) / 60 + min;
		col_g = max;
		col_b = min;
	}else if(hsv_h <= 180){
		col_r = min;
		col_g = max;
		col_b = ((hsv_h-120) * (max-min)) / 60 + min;
	}else if(hsv_h <= 240){
		col_r = min;
		col_g = ((240-hsv_h) * (max-min)) / 60 + min;
		col_b = max;
	}else if(hsv_h <= 300){
		col_r = ((hsv_h-240) * (max-min)) / 60 + min;
		col_g = min;
		col_b = max;
	}else if(hsv_h <= 360){
		col_r = max;
		col_g = min;
		col_b = ((360-hsv_h) * (max-min)) / 60 + min;
	}

	cdSetDisplayThemaMonoColor(CONV888TO565( ( ((uint32_t)col_r << 16) + ((uint32_t)col_g << 8) + col_b) ));

	hsv_h += 8;
	if(360 <= hsv_h){
		hsv_h = 0;
	}
}

void cdSetDisplayChangeInfo(DispSettingChangeInfo *dpc)
{
	//Display Color Mode Change
		//UI Type set
	switch(dpc->display_color_change_state){

		case COL_CHANGE_TO_DEFAULT:
			//curedraw color
			cdSetForegroundColor(COL_FOREGROUND_565);
			cdSetBackgroundColor(COL_BACKGROUND_565);
			cdSetBaseColor(COL_BASE_565);
			cdSet12bitBoxPattern(curedraw_color_foreground, curedraw_color_background, curedraw_color_basecolor);

			//kanji color
			kdSetForegroundColor(COL_FOREGROUND_565);
			kdSetBackgroundColor(COL_BACKGROUND_565);

			//fontdraw color
			fdSetForegroundColor(COL_FOREGROUND_565);
			fdSetBackgroundColor(COL_BACKGROUND_565);

			//volume meter
			cdSetLevelColor((uint16_t*)level2, (uint16_t*)level2_back);

			if(DISP_UI_TYPE_A == draw_setting.type){
				curedrawBackgroundChangeToTypeA();
			    UG_FillFrame(0, 0, 159, 19, curedraw_color_background);
			    cdSetBackgroundTop();

			}else if(DISP_UI_TYPE_B == draw_setting.type){
				curedrawBackgroundChangeToTypeB();
			    UG_FillFrame(0, 0, 159, 19, curedraw_color_background);
			    cdSetBackgroundTop();

			}else if(DISP_UI_PIANOROLL == draw_setting.type){
				curedrawBackgroundChangeToPianoRoll();
			    UG_FillFrame(0, 0, 159, 19, curedraw_color_background);
			    cdSetBackgroundTop();

			}else if(DISP_UI_WAVE == draw_setting.type){
				curedrawBackgroundChangeToWave();
				UG_FillFrame(0, 0, 159, 19, curedraw_color_background);
				cdSetBackgroundTop();
			}

			break;

		case COL_CHANGE_TO_GS:
			//curedraw color
			cdSetForegroundColor(COL_FOREGROUND_GS_565);
			cdSetBackgroundColor(COL_BACKGROUND_GS_565);
			cdSetBaseColor(COL_BASE_GS_565);
			cdSet12bitBoxPattern(curedraw_color_foreground, curedraw_color_background, curedraw_color_basecolor);

			//kanji color
			kdSetForegroundColor(COL_FOREGROUND_GS_565);
			kdSetBackgroundColor(COL_BACKGROUND_GS_565);

			//fontdraw color
			fdSetForegroundColor(COL_FOREGROUND_GS_565);
			fdSetBackgroundColor(COL_BACKGROUND_GS_565);

			//volume meter
			cdSetLevelColor((uint16_t*)level_gs, (uint16_t*)level_gs_back);

			if(DISP_UI_TYPE_A == draw_setting.type){
				curedrawBackgroundChangeToTypeA();
			    UG_FillFrame(0, 0, 159, 19, curedraw_color_background);
			    cdSetBackgroundTop();
			}else if(DISP_UI_TYPE_B == draw_setting.type){
				curedrawBackgroundChangeToTypeB();
			    UG_FillFrame(0, 0, 159, 19, curedraw_color_background);
			    cdSetBackgroundTop();
			}else if(DISP_UI_PIANOROLL == draw_setting.type){
				curedrawBackgroundChangeToPianoRoll();
			    UG_FillFrame(0, 0, 159, 19, curedraw_color_background);
			    cdSetBackgroundTop();
			}else if(DISP_UI_WAVE == draw_setting.type){
				curedrawBackgroundChangeToWave();
				UG_FillFrame(0, 0, 159, 19, curedraw_color_background);
				cdSetBackgroundTop();
			}
			break;

		case COL_CHANGE_TO_XG:
			//curedraw color
			cdSetForegroundColor(COL_FOREGROUND_XG_565);
			cdSetBackgroundColor(COL_BACKGROUND_XG_565);
			cdSetBaseColor(COL_BASE_XG_565);
			cdSet12bitBoxPattern(curedraw_color_foreground, curedraw_color_background, curedraw_color_basecolor);

			//kanji color
			kdSetForegroundColor(COL_FOREGROUND_XG_565);
			kdSetBackgroundColor(COL_BACKGROUND_XG_565);

			//fontdraw color
			fdSetForegroundColor(COL_FOREGROUND_XG_565);
			fdSetBackgroundColor(COL_BACKGROUND_XG_565);

			//volume meter
			cdSetLevelColor((uint16_t*)level_xg, (uint16_t*)level_xg_back);

			if(DISP_UI_TYPE_A == draw_setting.type){
				curedrawBackgroundChangeToTypeA();
			    UG_FillFrame(0, 0, 159, 19, curedraw_color_background);
			    cdSetBackgroundTop();
			}else if(DISP_UI_TYPE_B == draw_setting.type){
				curedrawBackgroundChangeToTypeB();
			    UG_FillFrame(0, 0, 159, 19, curedraw_color_background);
			    cdSetBackgroundTop();
			}else if(DISP_UI_PIANOROLL == draw_setting.type){
				curedrawBackgroundChangeToPianoRoll();
			    UG_FillFrame(0, 0, 159, 19, curedraw_color_background);
			    cdSetBackgroundTop();
			}else if(DISP_UI_WAVE == draw_setting.type){
				curedrawBackgroundChangeToWave();
				UG_FillFrame(0, 0, 159, 19, curedraw_color_background);
				cdSetBackgroundTop();
			}
			break;

		case COL_CHANGE_NOTHING:
		default:
			break;
	}

	//UI Type set
	switch(dpc->ui_change_state){

		case UI_CHANGE_TO_TYPE_A:

			if(DISP_UI_TYPE_A != draw_setting.type){
				draw_setting.type = DISP_UI_TYPE_A;
				draw_setting.eyeblink_is_active = false;
				curedrawBackgroundChangeToTypeA();
			}
			break;

		case UI_CHANGE_TO_TYPE_B:

			if(DISP_UI_TYPE_B != draw_setting.type){
				draw_setting.type = DISP_UI_TYPE_B;
				curedrawBackgroundChangeToTypeB();
			}
			break;

		case UI_CHANGE_TO_PIANOROLL:

			if(DISP_UI_PIANOROLL != draw_setting.type){
				draw_setting.type = DISP_UI_PIANOROLL;
				curedrawBackgroundChangeToPianoRoll();
			}
			break;

		case UI_CHANGE_TO_WAVE:

			if(DISP_UI_WAVE != draw_setting.type){
				draw_setting.type = DISP_UI_WAVE;
				curedrawBackgroundChangeToWave();

			}
			break;

		case UI_CHANGE_NOTHING:
		default:
			break;
	}

	//Face Blinking set
	if(DISP_UI_TYPE_B == draw_setting.type){ // (only UI Type-B)
		switch(dpc->eye_blink_state){

			case EYE_BLINK_TURN_ON:
				draw_setting.eyeblink_is_active = true;
				break;

			case EYE_BLINK_TURN_OFF:
				if(draw_setting.eyeblink_is_active){
					draw_setting.eyeblink_is_active = false;
					draw_setting.eye_type = FACE_EYE_1_BLINK;
					cdDrawDisplayFaceImageStructToBottomBackground(&(draw_setting.eye_setting[draw_setting.eye_type]));
				}
				break;

			case EYE_BLINK_NOTHING:
			default:
				break;
		}
	}


	//UI Face set
	if(DISP_UI_TYPE_B == draw_setting.type){ // (only UI Type-B)

		//eye
		if(!draw_setting.eyeblink_is_active){ // (if Face blinking is activated, ignore eye parts changing )
			if((255 != dpc->eye_change_num) && (FACE_EYE_NUM > dpc->eye_change_num)){
				draw_setting.eye_type = dpc->eye_change_num;
				cdDrawDisplayFaceImageStructToBottomBackground(&(draw_setting.eye_setting[draw_setting.eye_type]));
			}
		}

		//eyebrow
		if((255 != dpc->eyebrow_change_num) && (FACE_EYEBROW_NUM > dpc->eyebrow_change_num)){
			draw_setting.eyebrow_type = dpc->eyebrow_change_num;
			cdDrawDisplayFaceImageStructToBottomBackground(&(draw_setting.eyebrow_setting[draw_setting.eyebrow_type]));
		}

		//mouth
		if((255 != dpc->mouth_change_num) && (FACE_MOUTH_NUM > dpc->mouth_change_num)){
			draw_setting.mouth_type = dpc->mouth_change_num;
			cdDrawDisplayFaceImageStructToBottomBackground(&(draw_setting.mouth_setting[draw_setting.mouth_type]));
		}
	}

	//Japanese character mode set

	switch(dpc->language_change_state){

		case LANG_CHANGE_TO_EN:

			if(draw_setting.japanese_mode_is_active){
				draw_setting.japanese_mode_is_active = false;
				cdChangeToEnMode();
			}
			break;

		case LANG_CHANGE_TO_JP:

			if(!draw_setting.japanese_mode_is_active){
				draw_setting.japanese_mode_is_active = true;
				cdChangeToJpMode();
			}
			break;

		case LANG_CHANGE_NOTHING:
		default:
			break;
	}

	//Japanese character control
	switch(dpc->language_control_state_upper){
		case LANG_CTRL_UPDATE:
			if(draw_setting.japanese_mode_is_active){
				cdGetBackgroundTopUpper();
				kdDrawStr(0, 0, dpc->string_upper);
			}
			break;
		case LANG_CTRL_NOTHING:
		default:
			break;
	}

	switch(dpc->language_control_state_lower){
		case LANG_CTRL_UPDATE:
			if(draw_setting.japanese_mode_is_active){
				cdGetBackgroundTopDowner();
				kdDrawStr(0, 10, dpc->string_lower);
			}
			break;
		case LANG_CTRL_NOTHING:
		default:
			break;
	}
}


void cdDrawError(ErrorState err)
{

	UG_FillScreen(C_BLACK);
	fdSetForegroundColor(C_YELLOW);
	fdSetBackgroundColor(C_BLACK);
	kdSetForegroundColor(C_ORANGE);
	kdSetBackgroundColor(C_BLACK);


		switch(err){
			case ERR_VOLTAGE:
				fdDrawStr(0, 0, "USB VOLTAGE IS LOW");

				kdDrawStr(0, 20, "【USBポートの電圧が低下】");
				kdDrawStr(0, 31, "USBハブの使用をやめて下さい。");
				kdDrawStr(0, 42, "他のUSB機器を取り外して下さい。");
				break;
			case ERR_INITSRAM:
				fdDrawStr(0, 0, "PSRAM INIT ERROR!");

				kdDrawStr(0, 20, "【PSRAMの初期化失敗】");
				kdDrawStr(0, 31, "USBケーブルを抜いて下さい。");
				break;
			case ERR_SRAM:
				fdDrawStr(0, 0, "PSRAM TEST ERROR!");

				kdDrawStr(0, 20, "【PSRAMの読書失敗】");
				kdDrawStr(0, 31, "USBケーブルを抜いて下さい。");
				break;
			case ERR_UARTMEM:
				fdDrawStr(0, 0, "UART INIT ERROR!");
				break;
			case ERR_INITMIDI:
				fdDrawStr(0, 0, "MIDI INIT ERROR!");
				break;
			case ERR_INITSYN:
				fdDrawStr(0, 0, "SYNTH ERROR!");
				break;
			case ERR_AUDIO:
				fdDrawStr(0, 0, "AUDIO ERROR!");
				break;
			case ERR_DAC:
				fdDrawStr(0, 0, "DAC ERROR!");

				kdDrawStr(0, 20, "【DACチップの初期化失敗】");
				kdDrawStr(0, 31, "USBケーブルを抜いて下さい。");
				break;

			default:
				fdDrawStr(0, 40, "Unknown ERR");
				break;
	}


	displayUpdate();
	displayWaitDispalyTransfer();
}
