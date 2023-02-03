/*
 * cureeffect.h
 *
 *  Created on: 2018/02/17
 *      Author: Keshikan(www.keshikan.net)
 */

#ifndef CURELIB_INC_CUREEFFECT_H_
#define CURELIB_INC_CUREEFFECT_H_


#include <stdint.h>
#include <stdbool.h>

#include "curelib_inc/cureaudio.h"
#include "curemisc.h"


#define EFX_DELAY_BUF_LEN (16384)// 2^n(this), need >= 16384sample: 0.341sec. if change this, consider cureMidiSetDelayTime().

#define EFX_DIST_OUT_DIV (3) // output / 2^(this)
#define EFX_DIST_OUT_THR (100)

#define EFX_CHORUS_BUF_LEN (1024)
#define EFX_CHORUS_LFO_TABLE_LEN (512)
#define EFX_CHORUS_PHASE_NUM (3)

#define EFX_REVERB_INITIAL_BUF_LEN (1024)
#define EFX_REVERB_COMB_BUF_LEN (2048)//2048
#define EFX_REVERB_ALLPASS_BUF_LEN (512)
#define EFX_REVERB_INITIAL_NUM (5)
#define EFX_REVERB_COMB_NUM (4)
#define EFX_REVERB_ALLPASS_NUM (2)


//Effect setting.
#ifndef EFX_DELAY_ON
#define EFX_DELAY_ON
#endif

#ifndef EFX_CHORUS_ON
#define EFX_CHORUS_ON
#endif

#ifndef EFX_REVERB_ON
#define EFX_REVERB_ON
#endif

//Using sram
//#define EFX_USING_EXTERNAL_QSPI_SRAM

typedef enum
{
	REV_INIT, REV_HALL, REV_ROOM
}ReverbPreset;

//Effector setting structure
typedef struct
{

	struct del{

		bool feedback_ison;
		uint16_t time;
		uint8_t feedback_gain;
		uint8_t mix_gain;
	}delay;


	struct cho{

		uint8_t depth;
		float freq;
		uint16_t dly_time;
		uint8_t feedback_gain;
		uint8_t mix_gain;

	}chorus;


	struct rev{

		struct init{

			uint16_t dly_time;

		}initial[EFX_REVERB_INITIAL_NUM];

		struct com{

			uint16_t dly_time;
			float fb_gain;

		}comb[EFX_REVERB_COMB_NUM];

		struct all{

			uint16_t dly_time;
			float fb_gain;

		}allpass[EFX_REVERB_ALLPASS_NUM];

		uint8_t input_gain;
		uint8_t initial_mix_gain;
		uint8_t mix_gain;

	}reverb;


}EfxSettings;


//public variables
extern EfxSettings setting;

//public func
extern void efxSetReverbPreset(ReverbPreset preset);
extern void efxSetDelayTime(uint8_t time);
extern void efxSetDelayFdbkGain(uint8_t gain);
extern void efxSetChorusDepth(uint8_t depth);
extern void efxSetChorusFreq(uint8_t freq);
extern void efxSetParameterInit();

extern FUNC_STATUS efxInit();
extern void efxApplyDelay(Audio *io);
extern void efxApplyChorus(Audio *io);
extern void efxApplyReverb(Audio *io);

#endif /* CURELIB_INC_CUREEFFECT_H_ */
