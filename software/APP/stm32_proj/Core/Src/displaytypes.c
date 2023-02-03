/*
 * displaytypes.c
 *
 *  Created on: 2022/02/10
 *      Author: Keshikan
 */


#include "displaytypes.h"


void dpinfoInit(DispInfo *dp)
{
	dp->voice = 0;
	for(uint32_t i=0; i<DP_CHNUM; i++)
	{
		dp->display_data[i] = 0;
	}
}

void dpChangeInfoInit(DispSettingChangeInfo *dpc)
{
	dpc->eye_blink_state = EYE_BLINK_NOTHING;

	dpc->ui_change_state = UI_CHANGE_NOTHING;

	dpc->eyebrow_change_num = DISPSETTING_CHANGE_NOTHING;
	dpc->eye_change_num = DISPSETTING_CHANGE_NOTHING;
	dpc->mouth_change_num = DISPSETTING_CHANGE_NOTHING;

	dpc->language_change_state = LANG_CHANGE_NOTHING;

	dpc->language_control_state_upper = LANG_CTRL_NOTHING;
	dpc->language_control_state_lower = LANG_CTRL_NOTHING;
//	dpc->string_upper[0] = '\0';
//	dpc->string_downer[0] = '\0'; // this method is disable for after volume change redraw function.

	dpc->display_color_change_state = COL_CHANGE_NOTHING;
}
