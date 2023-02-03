/*
 * curesynth_setting.h
 *
 *  Created on: 2022/09/04
 *      Author: Keshikan
 */

#ifndef INC_CURELIB_INC_CURESYNTH_SETTING_H_
#define INC_CURELIB_INC_CURESYNTH_SETTING_H_


#define OPNUM (40)	// a number of operators.(= Maximum polyphony)
#define CHNUM (16)	//a number of output channels.

#define OUTPUT_CH_NUM (2)

//Sound precision settings.
#define SAMPLE_NUM (2048) 	// Sampling number of wave tables.
#define SAMPLE_NUM_HALF (1024) 	// Half of sampling number of wave tables.
#define SAMPLE_NUM_BIT (11) // bit of SAMPLE_NUM
#define SAMPLE_RATE (48000)	// sampling rate of DAC interrupt.
#define AUDIO_BIT (16)		//16bit
#define AUDIO_MAX_NUM   (32767)		//16bit
#define AUDIO_MIN_NUM  (-32768)		//16bit


//DAC
#define OUT_BUFSIZE (1024)  //need 2^n


#endif /* INC_CURELIB_INC_CURESYNTH_SETTING_H_ */
