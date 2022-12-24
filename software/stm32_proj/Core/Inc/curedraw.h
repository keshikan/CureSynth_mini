/*
 * curedraw.h
 *
 *  Created on: Feb 5, 2022
 *      Author: Keshikan
 */

#ifndef INC_CUREDRAW_H_
#define INC_CUREDRAW_H_

#include "ugui.h"
#include "st7735s_SPI.h"
#include "displaytypes.h"

#include "facepattern.h"

#include "kanjidraw.h"

#include "errortypes.h"

#define DISP_DEFAULT_STRING "    [CureSynth]     - MIDI Synthesizer -"
#define DISP_UPPER_STRING_NUM (41)

#define FACE_BASE_WIDTH (64)
#define FACE_BASE_HEIGHT (64)
#define FACE_BASE_START_ABS_X (0)
#define FACE_BASE_START_ABS_Y  (21)

#define FACE_EYEBROW_NUM (4)
#define FACE_EYEBROW_WIDTH (34)
#define FACE_EYEBROW_HEIGHT (6)
#define FACE_EYEBROW_START_X (15)
#define FACE_EYEBROW_START_Y (29)
typedef enum{
	FACE_EYEBROW_1=0, FACE_EYEBROW_2, FACE_EYEBROW_3, FACE_EYEBROW_4,
}DisplayFaceImageEyebrowType;


#define FACE_EYE_NUM (7)
#define FACE_EYEBLINK_NUM (3)
#define FACE_EYE_WIDTH (34)
#define FACE_EYE_HEIGHT (13)
#define FACE_EYE_START_X (15)
#define FACE_EYE_START_Y (35)
typedef enum{
	FACE_EYE_1_BLINK=0, FACE_EYE_2_BLINK, FACE_EYE_3_BLINK,
	FACE_EYE_4, FACE_EYE_5, FACE_EYE_6, FACE_EYE_7,
}DisplayFaceImageEyeType;


#define FACE_MOUTH_NUM (9)
#define FACE_MOUTH_WIDTH (12)
#define FACE_MOUTH_HEIGHT (9)
#define FACE_MOUTH_START_X (26)
#define FACE_MOUTH_START_Y (52)
typedef enum{
	 FACE_MOUTH_DEFAULT=0, FACE_MOUTH_A, FACE_MOUTH_I,
	 FACE_MOUTH_U, FACE_MOUTH_E, FACE_MOUTH_O,
	 FACE_MOUTH_PEKO, FACE_MOUTH_BLAME, FACE_MOUTH_MID,
}DisplayFaceImageMouthType;


typedef struct{
	const uint16_t *image_address;
	uint16_t startx, starty;
	uint16_t width, height;
}DisplayImageSetting;

typedef enum{
	DISP_UI_TYPE_A, DISP_UI_TYPE_B, DISP_UI_PIANOROLL, DISP_UI_WAVE,
}DisplayUIType;

typedef struct{
	DisplayUIType type;

	DisplayFaceImageEyebrowType eyebrow_type;
	DisplayFaceImageEyeType	eye_type;
	DisplayFaceImageMouthType mouth_type;

	DisplayImageSetting backimage_setting;
	DisplayImageSetting eyebrow_setting[FACE_EYEBROW_NUM];
	DisplayImageSetting eye_setting[FACE_EYE_NUM];
	DisplayImageSetting mouth_setting[FACE_MOUTH_NUM];

	//eyeblink
	bool eyeblink_is_active;
	uint32_t eyeblink_pattern_number_list[FACE_EYEBLINK_NUM];


	//jp draw
	bool japanese_mode_is_active;
	uint8_t cursor_upper;
	uint8_t cursor_lower;

}DispDrawingSetting;





extern void pset(UG_S16 x, UG_S16 y, UG_COLOR col);
extern void pset_tiny(UG_S16 x, UG_S16 y, UG_COLOR col);

extern void displaySetColorNormal();

extern void cdDrawUsbState(uint8_t state);
extern void cdDrawDbgInfo();

extern void cdMakeIndicator(DispInfo *dp, uint8_t volume[2], int16_t* audio_buffer, volatile uint32_t audio_idx);
extern void cdPeakDownValueCounter();

extern void cdSetBackgroundTop();
extern void cdGetBackgroundTop();

extern void cdSetBackgroundBottom();
extern void cdGetBackgroundBottom();

extern void cdSetLevelColor(uint16_t color_pattern_fore[64], uint16_t color_pattern_back[64]);
extern void cdSetForegroundColor(UG_COLOR col);
extern void cdSetBackgroundColor(UG_COLOR col);
extern void cdSetBaseColor(UG_COLOR col);


extern void cdDrawText(char* str);
extern void cdDrawEyeBlinking();

extern void cdDrawRotaryVolume(uint8_t volume_section);

extern bool cdIsJpMode();

extern void cdSetDisplayGamingMode();

extern void cdSetDisplayChangeInfo(DispSettingChangeInfo *dpc);

extern void curedrawInit();
extern void curedrawBackgroundInit();

extern void cdDrawError(ErrorState err);


#endif /* INC_CUREDRAW_H_ */
