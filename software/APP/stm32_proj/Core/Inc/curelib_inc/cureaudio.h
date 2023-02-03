/*
 * cureaudio.h
 *
 *  Created on: 2018/02/18
 *      Author: Keshikan(www.keshikan.net)
 */

#ifndef CURELIB_INC_CUREAUDIO_H_
#define CURELIB_INC_CUREAUDIO_H_


#define AUDIO_MAX_NUM   (32767)		//16bit
#define AUDIO_MIN_NUM  (-32768)		//16bit
#define AUDIO_MID_NUM  (32768)		//16bit

#define AUDIO_SAMPLE_RATE (32000)	// sampling rate of DAC interrupt.


#include <stdint.h>


typedef union{
	struct{
		int16_t left;
		int16_t right;
	}mono;
	uint32_t stereo_out;
}Audio;


extern int16_t auConvU16ToI16(uint16_t dat);
extern uint16_t auConvI16ToU16(int16_t dat);

extern int16_t auLerpMono(int16_t a1, int16_t a2, float ratio);
extern uint32_t auLerpStereo(uint32_t a1, uint32_t a2, float ratio);

#endif /* CURELIB_INC_CUREAUDIO_H_ */
