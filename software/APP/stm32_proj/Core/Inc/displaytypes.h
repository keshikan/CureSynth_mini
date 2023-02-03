/*
 * displaytypes.h
 *
 *  Created on: 2018/03/21
 *      Author: Keshikan
 */

#ifndef DISPLAYTYPES_H_
#define DISPLAYTYPES_H_

#define DP_CHNUM (16)
#define DISPSETTING_CHANGE_NOTHING (255)

#include <stdint.h>
#include <stdbool.h>
#include "curesynth_setting.h"

typedef enum{
	SINE, SQUARE, SAW, TRIANGLE, WHITE_NOISE, PINK_NOISE, PCM, PCM_REV
}WaveType;

typedef struct{
	uint8_t display_data[DP_CHNUM];
	uint8_t voice;
	bool voice_is_max;
	bool volume_is_clipping;
	uint64_t note_stat_upper64[CHNUM];
	uint64_t note_stat_lower64[CHNUM];
}DispInfo;

extern void dpinfoInit(DispInfo *dp);


typedef enum{
	EYE_BLINK_NOTHING, EYE_BLINK_TURN_ON, EYE_BLINK_TURN_OFF
}EyeBlinkState;

typedef enum{
	UI_CHANGE_NOTHING, UI_CHANGE_TO_TYPE_A, UI_CHANGE_TO_TYPE_B,
	UI_CHANGE_TO_PIANOROLL, UI_CHANGE_TO_WAVE
}UIChangeState;

typedef enum{
	LANG_CHANGE_NOTHING, LANG_CHANGE_TO_EN, LANG_CHANGE_TO_JP
}LanguageChangeState;

typedef enum{
	LANG_CTRL_NOTHING, LANG_CTRL_UPDATE
}LanguageControlState;

typedef enum{
	COL_CHANGE_NOTHING, COL_CHANGE_TO_DEFAULT, COL_CHANGE_TO_GS, COL_CHANGE_TO_XG
}DisplayColorChangeState;

#define DISPSETTING_STRING_MAX_LENGTH (35)
#define DISPSETTING_STRING_MAX_WIDTH (32)
typedef struct{
	EyeBlinkState eye_blink_state;

	UIChangeState ui_change_state;

	//255 is do nothing flag
	uint8_t eyebrow_change_num;
	uint8_t eye_change_num;//eye_blink_flag has higher priority than eye_change_num
	uint8_t mouth_change_num;

	LanguageChangeState language_change_state;
	LanguageControlState language_control_state_upper;
	LanguageControlState language_control_state_lower;
	char string_upper[DISPSETTING_STRING_MAX_LENGTH];
	char string_lower[DISPSETTING_STRING_MAX_LENGTH];

	DisplayColorChangeState display_color_change_state;

}DispSettingChangeInfo;

extern void dpChangeInfoInit(DispSettingChangeInfo *dpc);

#endif /* DISPLAYTYPES_H_ */
