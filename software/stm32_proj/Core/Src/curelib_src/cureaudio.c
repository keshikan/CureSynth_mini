/*
 * cureaudio.c
 *
 *  Created on: 2018/02/18
 *      Author: Keshikan(www.keshikan.net)
 */


#include "curelib_inc/cureaudio.h"


int16_t auConvU16ToI16(uint16_t dat)
{
	int16_t ret;

	ret = (int32_t)dat + AUDIO_MIN_NUM;

	return ret;

}

uint16_t auConvI16ToU16(int16_t dat)
{
	uint16_t ret;

	ret = (int32_t)dat - AUDIO_MIN_NUM;

	return ret;

}


int16_t auLerpMono(int16_t a1, int16_t a2, float ratio)
{
	int16_t ret;

	ret = (int16_t)(a1 * (1.0f - ratio) + a2 * ratio);
	return ret;

}


uint32_t auLerpStereo(uint32_t a1, uint32_t a2, float ratio)
{
	Audio ret, audio_a1, audio_a2;

	audio_a1.stereo_out = a1;
	audio_a2.stereo_out = a2;

	ret.mono.left = auLerpMono(audio_a1.mono.left, audio_a2.mono.left, ratio);
	ret.mono.right = auLerpMono(audio_a1.mono.right, audio_a2.mono.right, ratio);

	return ret.stereo_out;

}

