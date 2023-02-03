/*
 * cureeffect.c
 *
 *  Created on: 2018/02/17
 *      Author: Keshikan(www.keshikan.net)
 */


#define ARM_MATH_CM7
#include "stm32h7xx_hal.h"
#include <./dsp_lib/arm_math.h>
#include <stdlib.h>

#include "curelib_inc/cureeffect.h"
#include "curelib_inc/curebuffer_static.h"
#include "curelib_inc/curemisc.h"

#include "sram_manage.h"

//ram positioning
#define __ATTR_DTCMRAM   __attribute__ ((section(".DTCMRAM"))) __attribute__ ((aligned (4)))
#define __ATTR_ITCMRAM   __attribute__ ((section(".ITCMRAM"))) __attribute__ ((aligned (8)))
#define __ATTR_RAM_D2	__attribute__ ((section(".RAM_D2"))) __attribute__ ((aligned (4)))
#define __ATTR_RAM_D3	__attribute__ ((section(".RAM_D3"))) __attribute__ ((aligned (4)))
#define __ATTR_QSPI	   __attribute__ ((section(".QSPI_PSRAM")))


EfxSettings setting;

#ifdef EFX_USING_EXTERNAL_QSPI_SRAM
SramBuffer32 delay_buffer;
#else
RingBufferU32 delay_buffer;
uint32_t delay_buffer_array[EFX_DELAY_BUF_LEN] ;
#endif

Audio dly_in_dma_buffer __ATTR_RAM_D2;
Audio dly_out_dma_buffer __ATTR_RAM_D2;

RingBufferU32 chorus_buffer;
RingBufferU32 reverb_buffer_initial;
RingBufferU32 reverb_buffer_comb[EFX_REVERB_COMB_NUM];
RingBufferU32 reverb_buffer_allpass[EFX_REVERB_ALLPASS_NUM];

uint32_t chorus_buffer_array[EFX_CHORUS_BUF_LEN];
uint32_t reverb_buffer_initial_array[EFX_REVERB_INITIAL_BUF_LEN];
uint32_t reverb_buffer_comb_array[EFX_REVERB_COMB_NUM][EFX_REVERB_COMB_BUF_LEN];
uint32_t reverb_buffer_allpass_array[EFX_REVERB_ALLPASS_NUM][EFX_REVERB_ALLPASS_BUF_LEN];

const uint16_t chorus_lfo_table[EFX_CHORUS_LFO_TABLE_LEN] = {32768, 33170, 33572, 33974, 34376, 34777, 35178, 35579, 35980, 36380, 36779, 37178, 37576, 37973, 38370, 38766, 39161, 39554, 39947, 40339, 40730, 41119, 41507, 41894, 42280, 42664, 43046, 43427, 43807, 44185, 44561, 44935, 45307, 45678, 46047, 46413, 46778, 47140, 47500, 47858, 48214, 48568, 48919, 49267, 49614, 49957, 50298, 50637, 50972, 51305, 51636, 51963, 52287, 52609, 52927, 53243, 53555, 53864, 54171, 54473, 54773, 55069, 55362, 55652, 55938, 56220, 56499, 56775, 57047, 57315, 57579, 57840, 58097, 58350, 58600, 58845, 59087, 59324, 59558, 59787, 60013, 60234, 60451, 60664, 60873, 61078, 61278, 61474, 61666, 61853, 62036, 62215, 62389, 62559, 62724, 62885, 63041, 63192, 63339, 63482, 63620, 63753, 63881, 64005, 64124, 64238, 64348, 64453, 64553, 64648, 64739, 64825, 64905, 64981, 65053, 65119, 65180, 65237, 65289, 65335, 65377, 65414, 65446, 65473, 65496, 65513, 65525, 65533, 65535, 65533, 65525, 65513, 65496, 65473, 65446, 65414, 65377, 65335, 65289, 65237, 65180, 65119, 65053, 64981, 64905, 64825, 64739, 64648, 64553, 64453, 64348, 64238, 64124, 64005, 63881, 63753, 63620, 63482, 63339, 63192, 63041, 62885, 62724, 62559, 62389, 62215, 62036, 61853, 61666, 61474, 61278, 61078, 60873, 60664, 60451, 60234, 60013, 59787, 59558, 59324, 59087, 58845, 58600, 58350, 58097, 57840, 57579, 57315, 57047, 56775, 56499, 56220, 55938, 55652, 55362, 55069, 54773, 54473, 54171, 53864, 53555, 53243, 52927, 52609, 52287, 51963, 51636, 51305, 50972, 50637, 50298, 49957, 49614, 49267, 48919, 48568, 48214, 47858, 47500, 47140, 46778, 46413, 46047, 45678, 45307, 44935, 44561, 44185, 43807, 43427, 43046, 42664, 42280, 41894, 41507, 41119, 40730, 40339, 39947, 39554, 39161, 38766, 38370, 37973, 37576, 37178, 36779, 36380, 35980, 35579, 35178, 34777, 34376, 33974, 33572, 33170, 32768, 32366, 31964, 31562, 31160, 30759, 30358, 29957, 29556, 29156, 28757, 28358, 27960, 27563, 27166, 26770, 26375, 25982, 25589, 25197, 24806, 24417, 24029, 23642, 23256, 22872, 22490, 22109, 21729, 21351, 20975, 20601, 20229, 19858, 19489, 19123, 18758, 18396, 18036, 17678, 17322, 16968, 16617, 16269, 15922, 15579, 15238, 14899, 14564, 14231, 13900, 13573, 13249, 12927, 12609, 12293, 11981, 11672, 11365, 11063, 10763, 10467, 10174, 9884, 9598, 9316, 9037, 8761, 8489, 8221, 7957, 7696, 7439, 7186, 6936, 6691, 6449, 6212, 5978, 5749, 5523, 5302, 5085, 4872, 4663, 4458, 4258, 4062, 3870, 3683, 3500, 3321, 3147, 2977, 2812, 2651, 2495, 2344, 2197, 2054, 1916, 1783, 1655, 1531, 1412, 1298, 1188, 1083, 983, 888, 797, 711, 631, 555, 483, 417, 356, 299, 247, 201, 159, 122, 90, 63, 40, 23, 11, 3, 1, 3, 11, 23, 40, 63, 90, 122, 159, 201, 247, 299, 356, 417, 483, 555, 631, 711, 797, 888, 983, 1083, 1188, 1298, 1412, 1531, 1655, 1783, 1916, 2054, 2197, 2344, 2495, 2651, 2812, 2977, 3147, 3321, 3500, 3683, 3870, 4062, 4258, 4458, 4663, 4872, 5085, 5302, 5523, 5749, 5978, 6212, 6449, 6691, 6936, 7186, 7439, 7696, 7957, 8221, 8489, 8761, 9037, 9316, 9598, 9884, 10174, 10467, 10763, 11063, 11365, 11672, 11981, 12293, 12609, 12927, 13249, 13573, 13900, 14231, 14564, 14899, 15238, 15579, 15922, 16269, 16617, 16968, 17322, 17678, 18036, 18396, 18758, 19123, 19489, 19858, 20229, 20601, 20975, 21351, 21729, 22109, 22490, 22872, 23256, 23642, 24029, 24417, 24806, 25197, 25589, 25982, 26375, 26770, 27166, 27563, 27960, 28358, 28757, 29156, 29556, 29957, 30358, 30759, 31160, 31562, 31964, 32366};
const float chr_scalefactor = (float)EFX_CHORUS_LFO_TABLE_LEN / (float)AUDIO_SAMPLE_RATE;

const uint32_t chr_phase_scalefactor = EFX_CHORUS_LFO_TABLE_LEN * (1 << 16) / EFX_CHORUS_PHASE_NUM;

#ifdef EFX_USING_EXTERNAL_QSPI_SRAM
#else
#endif

inline SRAM_BUFFER_STATUS efxDelayBufferInit()
{
#ifdef EFX_USING_EXTERNAL_QSPI_SRAM
	return sramRingBufferU32Init(&delay_buffer, 0x00, EFX_DELAY_BUF_LEN);
#else
	return cureRingBufferU32Init(&delay_buffer, EFX_DELAY_BUF_LEN, delay_buffer_array);
#endif

}

inline BUFFER_STATUS efxDelayBufferEnqueue(uint32_t *input)
{
#ifdef EFX_USING_EXTERNAL_QSPI_SRAM
//	return sramRingBufferU32EnqueueIgnoreErr(&delay_buffer, input);
	return sramRingBufferU32EnqueueIgnoreErr_DMA(&delay_buffer, input);
#else
	return cureRingBufferU32EnqueueIgnoreErr(&delay_buffer, input);
#endif

}

inline BUFFER_STATUS efxDelayBufferGetElement(uint32_t *ret, uint32_t num)
{
#ifdef EFX_USING_EXTERNAL_QSPI_SRAM
//	return sramRingBufferU32GetElement(&delay_buffer, ret, num, EFX_DELAY_BUF_LEN);
	return sramRingBufferU32GetElement_DMA(&delay_buffer, ret, num, EFX_DELAY_BUF_LEN);
#else
	return cureRingBufferU32GetElement(&delay_buffer, ret, num, EFX_DELAY_BUF_LEN);
#endif

}

inline BUFFER_STATUS efxChorusBufferInit()
{
	return cureRingBufferU32Init(&chorus_buffer, EFX_CHORUS_BUF_LEN, chorus_buffer_array);
}

inline BUFFER_STATUS efxChorusBufferEnqueue(uint32_t *input)
{
	return cureRingBufferU32EnqueueIgnoreErr(&chorus_buffer, input);
}

inline BUFFER_STATUS efxChorusBufferGetElement(uint32_t *ret, uint32_t num)
{
	return cureRingBufferU32GetElement(&chorus_buffer, ret, num, EFX_CHORUS_BUF_LEN);
}

inline BUFFER_STATUS efxReverbBufferInit(RingBufferU32 *buf, uint32_t len, uint32_t* ptr)
{
	return cureRingBufferU32Init(buf, len, ptr);
}

inline BUFFER_STATUS efxReverbBufferEnqueue(RingBufferU32 *buf, uint32_t *input)
{
	return cureRingBufferU32EnqueueIgnoreErr(buf, input);
}

inline BUFFER_STATUS efxReverbBufferGetElement(RingBufferU32 *buf, uint32_t *ret, uint32_t num, uint32_t len)
{
	return cureRingBufferU32GetElement(buf, ret, num, len);
}


void efxSetReverbPreset(ReverbPreset preset)
{
	switch(preset){
		case REV_INIT:
			setting.reverb.initial[0].dly_time = 101;
			setting.reverb.initial[1].dly_time = 211;
			setting.reverb.initial[2].dly_time = 311;
			setting.reverb.initial[3].dly_time = 401;
			setting.reverb.initial[4].dly_time = 503;
			setting.reverb.comb[0].dly_time = 1901;
			setting.reverb.comb[1].dly_time = 1669;
			setting.reverb.comb[2].dly_time = 1597;
			setting.reverb.comb[3].dly_time = 1447;
			setting.reverb.comb[0].fb_gain = -0.871402f;
			setting.reverb.comb[1].fb_gain = -0.882762f;
			setting.reverb.comb[2].fb_gain = -0.891443f;
			setting.reverb.comb[3].fb_gain = -0.901117f;
			setting.reverb.allpass[0].dly_time = 241;
			setting.reverb.allpass[1].dly_time = 83;
			setting.reverb.allpass[0].fb_gain = 0.7;
			setting.reverb.allpass[1].fb_gain = 0.7;

			setting.reverb.input_gain = 100;
			setting.reverb.initial_mix_gain = 90;
			setting.reverb.mix_gain = 100;
			break;

		case REV_HALL:
			setting.reverb.initial[0].dly_time = 311;
			setting.reverb.initial[1].dly_time = 499;
			setting.reverb.initial[2].dly_time = 797;
			setting.reverb.initial[3].dly_time = 907;
			setting.reverb.initial[4].dly_time = 1021;
			setting.reverb.comb[0].dly_time = 2039;
			setting.reverb.comb[1].dly_time = 1789;
			setting.reverb.comb[2].dly_time = 1693;
			setting.reverb.comb[3].dly_time = 1511;
			setting.reverb.comb[0].fb_gain = -0.929003;
			setting.reverb.comb[1].fb_gain = -0.936953f;
			setting.reverb.comb[2].fb_gain = -0.940957f;
			setting.reverb.comb[3].fb_gain = -0.945031f;
			setting.reverb.allpass[0].dly_time = 241;
			setting.reverb.allpass[1].dly_time = 83;
			setting.reverb.allpass[0].fb_gain = 0.75;
			setting.reverb.allpass[1].fb_gain = 0.75;

			setting.reverb.input_gain = 100;
			setting.reverb.initial_mix_gain = 100;
			setting.reverb.mix_gain = 130;
			break;

		case REV_ROOM:
		default:
			setting.reverb.initial[0].dly_time = 311;
			setting.reverb.initial[1].dly_time = 401;
			setting.reverb.initial[2].dly_time = 503;
			setting.reverb.initial[3].dly_time = 647;
			setting.reverb.initial[4].dly_time = 797;
			setting.reverb.comb[0].dly_time = 1597;
			setting.reverb.comb[1].dly_time = 1511;
			setting.reverb.comb[2].dly_time = 1423;
			setting.reverb.comb[3].dly_time = 1327;
			setting.reverb.comb[0].fb_gain = -0.750037f;
			setting.reverb.comb[1].fb_gain = -0.761251f;
			setting.reverb.comb[2].fb_gain = -0.774523f;
			setting.reverb.comb[3].fb_gain = -0.786833f;
			setting.reverb.allpass[0].dly_time = 241;
			setting.reverb.allpass[1].dly_time = 83;
			setting.reverb.allpass[0].fb_gain = 0.7;
			setting.reverb.allpass[1].fb_gain = 0.7;

			setting.reverb.input_gain = 100;
			setting.reverb.initial_mix_gain = 110;
			setting.reverb.mix_gain = 100;
			break;
	}
}

void efxSetDelayTime(uint8_t time)
{
	setting.delay.time = time << 7;
}

#define DELAY_FDBK_GAIN_MAX (240)
void efxSetDelayFdbkGain(uint8_t gain)
{
	//max 127
	setting.delay.feedback_gain = ((gain * DELAY_FDBK_GAIN_MAX) / 127);
}

void efxSetChorusDepth(uint8_t depth)
{
	setting.chorus.depth = depth;
}

void efxSetChorusFreq(uint8_t freq)
{
	setting.chorus.freq = 0.04f * (freq+1);
}

void efxSetParameterInit()
{
	//delay
	setting.delay.time = 16383;//default 16383
	setting.delay.mix_gain = 80;
	setting.delay.feedback_ison = true;
	setting.delay.feedback_gain = 120;
	dly_in_dma_buffer.stereo_out = 0;
	dly_out_dma_buffer.stereo_out = 0;

	//chorus
	efxSetChorusDepth(100);
	efxSetChorusFreq(9);
	setting.chorus.dly_time = 512;
	setting.chorus.feedback_gain = 0;
	setting.chorus.mix_gain = 120;

	//reverb
	efxSetReverbPreset(REV_INIT);

}


FUNC_STATUS efxInit()
{

	FUNC_STATUS ret_flag = FUNC_SUCCESS;

	efxSetParameterInit();

#ifdef EFX_DELAY_ON
	//Delay Init


	if( SRAM_BUFFER_FAILURE == efxDelayBufferInit() ){
		ret_flag = FUNC_ERROR;
	}

#endif

#ifdef EFX_CHORUS_ON
	//Chorus Init

	if( BUFFER_FAILURE == efxChorusBufferInit() ){
		ret_flag = FUNC_ERROR;
	}
#endif

#ifdef EFX_REVERB_ON
	//Reverb Init

	if( BUFFER_FAILURE == efxReverbBufferInit(&reverb_buffer_initial, EFX_REVERB_INITIAL_BUF_LEN, reverb_buffer_initial_array) ){
		ret_flag = FUNC_ERROR;
	}

	for(uint32_t i=0; i<EFX_REVERB_COMB_NUM; i++)
	{
		if( BUFFER_FAILURE == efxReverbBufferInit(&reverb_buffer_comb[i], EFX_REVERB_COMB_BUF_LEN, reverb_buffer_comb_array[i]) ){
			ret_flag = FUNC_ERROR;
		}
	}

	for(uint32_t i=0; i<EFX_REVERB_ALLPASS_NUM; i++)
	{
		if( BUFFER_FAILURE == efxReverbBufferInit(&reverb_buffer_allpass[i], EFX_REVERB_ALLPASS_BUF_LEN, reverb_buffer_allpass_array[i]) ){
			ret_flag = FUNC_ERROR;
		}
	}
#endif

	return ret_flag;
}


void efxApplyDelay(Audio *io)
{

	Audio in = {0}, dly_in = {0}, dly_out = {0};

	in.stereo_out = io->stereo_out;


//	efxDelayBufferGetElement(&dly_out.stereo_out, setting.delay.time);
	dly_out.stereo_out = dly_out_dma_buffer.stereo_out;

	if(setting.delay.feedback_ison){
		dly_in.mono.left  = ((dly_out.mono.left * setting.delay.feedback_gain)  / 256 ) + in.mono.left;
		dly_in.mono.right = ((dly_out.mono.right * setting.delay.feedback_gain) / 256 ) + in.mono.right;
	}

	dly_in_dma_buffer.stereo_out = dly_in.stereo_out;
	efxDelayBufferEnqueue(&dly_in_dma_buffer.stereo_out);


	efxDelayBufferGetElement(&dly_out_dma_buffer.stereo_out, setting.delay.time);

	io->stereo_out = dly_out.stereo_out;

}

uint16_t efxGetSin(uint32_t pointer)
{

	uint32_t a1, a2;
	float ratio;
	uint16_t ret;

	a1 = (pointer >> 16) << 16;
	a1 &= (EFX_CHORUS_LFO_TABLE_LEN * (1 << 16) - 1);
	a2 = ((pointer >> 16) + 1) << 16;
	a2 &= (EFX_CHORUS_LFO_TABLE_LEN * (1 << 16) - 1);

	ratio = (float)(pointer - a1) / (float)(1 << 16);

	ret = (uint16_t)(chorus_lfo_table[(a1 >> 16)] * (1.0f - ratio) + chorus_lfo_table[(a2 >> 16)] * ratio);

	return ret;

}

//3-phase chorus. phase settings:EFX_CHORUS_PHASE_NUM;
void efxApplyChorus(Audio *io)
{
	Audio in = {0}, dly_in = {0}, dly_out = {0}, dly_out_buf, fdbk_out = {0};
	static uint32_t vib_pointer = 0;
	uint32_t vib_angular;
	uint32_t chorus_buf;


	//Lerp
	uint32_t chorus_a1, chorus_a2;
	float chorus_ratio;
	Audio a1, a2;
	uint16_t chorus_depth_buf_ref = setting.chorus.depth << 8;
	static uint16_t chorus_depth_buf = 0;

	if(chorus_depth_buf != chorus_depth_buf_ref){
		chorus_depth_buf += ( ((int32_t)chorus_depth_buf_ref - (int32_t)chorus_depth_buf) / abs((int32_t)chorus_depth_buf_ref - (int32_t)chorus_depth_buf));
	}


	in.stereo_out = io->stereo_out;

	//calculate angular velocity
	vib_angular = (uint32_t)(setting.chorus.freq * chr_scalefactor * (1 << 16));


	for(uint32_t i=0; i<EFX_CHORUS_PHASE_NUM; i++){

		chorus_buf = ((efxGetSin(vib_pointer) * (chorus_depth_buf)) >> 8);
		chorus_a1 = (chorus_buf  >> 16) << 16;
		chorus_a2 = ((chorus_buf  >> 16) + 1) << 16;
		chorus_ratio =  (float)(chorus_buf - chorus_a1)/(1<<16);
		efxChorusBufferGetElement(&a1.stereo_out, setting.chorus.dly_time + (chorus_a1 >> 16) );
		efxChorusBufferGetElement(&a2.stereo_out, setting.chorus.dly_time + (chorus_a2 >> 16) );
		dly_out_buf.stereo_out = auLerpStereo(a1.stereo_out, a2.stereo_out, chorus_ratio);
		dly_out.stereo_out = __QADD16( dly_out.stereo_out , dly_out_buf.stereo_out);
		vib_pointer = (vib_pointer + chr_phase_scalefactor) & (EFX_CHORUS_LFO_TABLE_LEN * (1 << 16) - 1);
	}

//	//use feedback
//	fdbk_out.mono.left  = ((dly_out.mono.left * setting.chorus.feedback_gain) >> 8);
//	fdbk_out.mono.right = ((dly_out.mono.right * setting.chorus.feedback_gain) >> 8);
//	dly_in.stereo_out = __QADD16( in.stereo_out , fdbk_out.stereo_out);
//	efxChorusBufferEnqueue(&dly_in.stereo_out);

	efxChorusBufferEnqueue(&in.stereo_out);

	vib_pointer += vib_angular;
	vib_pointer &= (EFX_CHORUS_LFO_TABLE_LEN * (1 << 16) - 1);

	//mixing
	io->stereo_out = dly_out.stereo_out;

 }


void efxApplyReverb(Audio *io)
{
	Audio in, out, initial_buf, initial_out = {0},
			comb_in, comb_out, comb_mix = {0},
			allpass_in, allpass_buf, allpass_buf2, allpass_out;

	in.mono.left = (io->mono.left * setting.reverb.input_gain) >> 8;
	in.mono.right = (io->mono.right * setting.reverb.input_gain) >> 8;

	//initial filter
	efxReverbBufferEnqueue(&reverb_buffer_initial, &in.stereo_out);

	for(uint32_t i=0; i<EFX_REVERB_INITIAL_NUM; i++){
		efxReverbBufferGetElement(&reverb_buffer_initial, &initial_buf.stereo_out, setting.reverb.initial[i].dly_time, EFX_REVERB_INITIAL_BUF_LEN);
		initial_out.stereo_out = __QADD16(initial_out.stereo_out , initial_buf.stereo_out);
	}

	initial_out.mono.left = (initial_out.mono.left * setting.reverb.initial_mix_gain) >> 8;
	initial_out.mono.right = (initial_out.mono.right * setting.reverb.initial_mix_gain) >> 8;


	//comb filter
	for(uint32_t i=0; i<EFX_REVERB_COMB_NUM; i++){

		efxReverbBufferGetElement(&reverb_buffer_comb[i], &comb_out.stereo_out, setting.reverb.comb[i].dly_time, EFX_REVERB_COMB_BUF_LEN);

		comb_in.mono.left  = (int16_t)(comb_out.mono.left * setting.reverb.comb[i].fb_gain);
		comb_in.mono.right = (int16_t)(comb_out.mono.right * setting.reverb.comb[i].fb_gain);
		comb_in.stereo_out = __QADD16(comb_in.stereo_out, in.stereo_out);

		efxReverbBufferEnqueue(&reverb_buffer_comb[i], &comb_in.stereo_out);

		comb_mix.stereo_out = __QADD16(comb_mix.stereo_out , comb_out.stereo_out);

	}

	//all-pass filter
	allpass_in.stereo_out = comb_mix.stereo_out;

	for(uint32_t i=0; i<EFX_REVERB_ALLPASS_NUM; i++){

		efxReverbBufferGetElement(&reverb_buffer_allpass[i], &allpass_buf.stereo_out, setting.reverb.allpass[i].dly_time, EFX_REVERB_ALLPASS_BUF_LEN);

		allpass_buf2.mono.left  = (int16_t)(allpass_buf.mono.left * setting.reverb.allpass[i].fb_gain);
		allpass_buf2.mono.right = (int16_t)(allpass_buf.mono.right * setting.reverb.allpass[i].fb_gain);
		allpass_in.stereo_out = __QADD16(allpass_in.stereo_out , allpass_buf2.stereo_out);

		efxReverbBufferEnqueue(&reverb_buffer_allpass[i], &allpass_in.stereo_out);

		allpass_out.mono.left  = (int16_t)(allpass_in.mono.left * setting.reverb.allpass[i].fb_gain * (-1));
		allpass_out.mono.right = (int16_t)(allpass_in.mono.right * setting.reverb.allpass[i].fb_gain * (-1));
		allpass_out.stereo_out = __QADD16(allpass_out.stereo_out , allpass_buf.stereo_out);

		allpass_in.stereo_out = allpass_out.stereo_out;

	}
	allpass_out.mono.left = (allpass_out.mono.left * setting.reverb.mix_gain) >> 8;
	allpass_out.mono.right = (allpass_out.mono.right * setting.reverb.mix_gain) >> 8;

	//mixing
	io->stereo_out = __QADD16( initial_out.stereo_out , allpass_out.stereo_out);


}
