
#include "pcm.h"

#include "pcm_bin.c"

const uint8_t* pcm_ptr[PCM_NUM] = {
		tom_2_raw, snare_3_raw, snare_2_raw, side_stick_2_raw,
		kick_1_raw, kick_2_raw, handclap_raw, cymbal_splash_raw,
		cymbal_ride_raw, cymbal_crash_short_raw, hihat_open_raw,
		hihat_close_raw, hihat_pedal_raw,
};

const uint32_t pcm_len[PCM_NUM] = {
		tom_2_raw_len, snare_3_raw_len, snare_2_raw_len, side_stick_2_raw_len,
		kick_1_raw_len, kick_2_raw_len, handclap_raw_len, cymbal_splash_raw_len,
		cymbal_ride_raw_len, cymbal_crash_short_raw_len, hihat_open_raw_len,
		hihat_close_raw_len, hihat_pedal_raw_len,
};
