/*
 * pcm.h
 *
 *  Created on: 2022/03/06
 *      Author: Keshikan
 */

#ifndef INC_PCM_H_
#define INC_PCM_H_

#define PCM_NUM (13)

#include <stdint.h>

extern const uint8_t tom_2_raw[];
extern const uint8_t snare_3_raw[];
extern const uint8_t snare_2_raw[];
extern const uint8_t side_stick_2_raw[];
extern const uint8_t kick_1_raw[];
extern const uint8_t kick_2_raw[];
extern const uint8_t handclap_raw[];
extern const uint8_t cymbal_splash_raw[];
extern const uint8_t cymbal_ride_raw[];
extern const uint8_t cymbal_crash_short_raw[];
extern const uint8_t hihat_open_raw[];
extern const uint8_t hihat_close_raw[];
extern const uint8_t hihat_pedal_raw[];

extern const uint32_t tom_2_raw_len;
extern const uint32_t snare_3_raw_len;
extern const uint32_t snare_2_raw_len;
extern const uint32_t side_stick_2_raw_len;
extern const uint32_t kick_1_raw_len;
extern const uint32_t kick_2_raw_len;
extern const uint32_t handclap_raw_len;
extern const uint32_t cymbal_splash_raw_len;
extern const uint32_t cymbal_ride_raw_len;
extern const uint32_t cymbal_crash_short_raw_len;
extern const uint32_t hihat_open_raw_len;
extern const uint32_t hihat_close_raw_len;
extern const uint32_t hihat_pedal_raw_len;

extern const uint8_t* pcm_ptr[PCM_NUM];
extern const uint32_t pcm_len[PCM_NUM];


#endif /* INC_PCM_H_ */
