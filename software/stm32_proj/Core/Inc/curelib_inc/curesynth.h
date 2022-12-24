/*
 *******************************************************************************
 *  [curesynth.h]
 *  This module is for sound generation.
 *
 *  This program is under the terms of the GPLv3.
 *  https://www.gnu.org/licenses/gpl-3.0.html
 *
 *  Copyright(c) 2017 Keshikan (www.keshikan.net)
 *******************************************************************************
 */

#ifndef CURESYNTH_H_
#define CURESYNTH_H_


#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <stdbool.h>

#include "curesynth_setting.h"

#include "curelib_inc/cureeffect.h"
#include "curelib_inc/curebuffer_static.h"
#include "./curelib_inc/curemisc.h"
#include "curelib_inc/cureaudio.h"

#include "./displaytypes.h"

#include "pcm.h"


//Interpolation algorithm
//#define OVERSAMPLE_ON
//#define LERP_ON
#define OVERSAMPLING_NUM (8) //todo

//Triangle Peak shifter
#define TRIANGLE_PEAK_SHIFTER_ON

//Effect setting.
//#ifndef EFX_DELAY_ON
//#define EFX_DELAY_ON
//#endif
//
//#ifndef EFX_CHORUS_ON
//#define EFX_CHORUS_ON
//#endif
//
//#ifndef EFX_REVERB_ON
//#define EFX_REVERB_ON
//#endif

//Pitchbend
#define PITCHTABLE_NUM (768)

//PitchBend Control
#define PITCH_CTR_DIV (384) //need 2^n

//ADSR
#define FADESAMPLE_BIT (6)	//sampling number for generating fade effects.
#define ADSR_SCALEFACTOR (SAMPLE_RATE / 400) //to multiply adsr time.

//Pan
#define PANTABLE_NUM (128+1)

//Modulation
#define MODTABLE_NUM (256)
#define MOD_PLL_DIV (4500) // The base frequency [BF] is set to (SAMPLE_RATE / MOD_PLL_DIV) / MODTABLE, and Modulation frequency set to [BF] * modulation
#define MOD_PLL_OFFSET (20) // Offset number: modulation = modulation + 33  (e.g. When modulation = 64, Modulation frequency set to [BF] * (modulation + MOD_PLL_OFFSET)
#define MOD_PLL_SCALE (2)
#define MOD_AMPLITUDE (0.7f)
#define MOD_GAIN_SCALEFACTOR (0.8f)

//Delay
#define DELAYBUFFER_LENGTH (65536)//need 2^n, default:32768

//Filter
//[CAUTION]: This frequency must be lower than Nyquist frequency (SAMPLE_RATE / 2).
#define FLT_FREQ_MIN (200.0f)	  //When cutoff = 0
#define FLT_FREQ_MID (15000.0f)  //When cutoff = 64
#define FLT_FREQ_MAX (18000.0f)	  //When cutoff = 127
#define FLT_QFACT_MIN (0.001f)   //When resonance = 0
#define FLT_QFACT_MID (0.85f)    //When resonance = 64
#define FLT_QFACT_MAX (15.00f)   //When resonance = 127

//DC-block Filter
//standard:0.9985f
#define DCBLOCK_FACTOR (0.9985f)

//UNISON
#define UNISON_MAX_NUM (9)

//Output
#define OUTPUT_DIV_BIT (0) // output / 2^(this)
#define OUTPUT_VOLUME_DIV_BIT (0)
#define MASTER_OUTPUT_DIV_BIT (0) // output / 2^(this)

//Distortion
#define OUTPUT_DIST_DIV_BIT (10) // output / 2^(this)
#define DIST_LEVEL_DIV_BIT (1)
#define DIST_TABLE_NUM (256)

//Peak Detector
#define PEAKDETECT_FACTOR (0.999f)


typedef enum{
	ADSR_START, // Operator output process has been started.
	ATTACK,     // Processing Attack.
	DECAY,      // Processing Decay.
	SUSTAIN,    // Processing Sustain.
	RELEASE,    // Processing Release.
	ADSR_END,    // Operator output process has been ended.
	ADSR_FINAL	// Operator output finalize procedure.
}EnvelopeStatus;

typedef enum{
	NOTE_OFF, NOTE_ON
}NoteStatus;

typedef enum{ //todo for internal sequencer.
	C0 = 24, C0_up, D0, D0_up, E0, F0, F0_up, G0, G0_up, A0, A0_up, B0,
	C1, C1_up, D1, D1_up, E1, F1, F1_up, G1, G1_up, A1, A1_up, B1,
	C2, C2_up, D2, D2_up, E2, F2, F2_up, G2, G2_up, A2, A2_up, B2,
	C3, C3_up, D3, D3_up, E3, F3, F3_up, G3, G3_up, A3, A3_up, B3,
	C4, C4_up, D4, D4_up, E4, F4, F4_up, G4, G4_up, A4, A4_up, B4,
	C5, C5_up, D5, D5_up, E5, F5, F5_up, G5, G5_up, A5, A5_up, B5,
	C6, C6_up, D6, D6_up, E6, F6, F6_up, G6, G6_up, A6, A6_up, B6,
	C7, C7_up, D7, D7_up, E7, F7, F7_up, G7, G7_up, A7, A7_up, B7, SEQUENCER_END = 999
}Scale;

typedef enum{
	LPF, HPF
}FilterType;

typedef enum{
	SWEEP_UP, SWEEP_DOWN, SWEEP_NONE
}PBSweepType;

typedef enum{
	MODE_ORIGINAL, // Original mode. WaveType can be controlled by program change message.
	MODE_GM1,      // General MIDI Level1 compatible mode. Some parameters (including WaveType) are set by [patchlist.h].
	MODE_HYBRID,   // Hybrid (Original + GM1) mode.
	MODE_GM2,      //todo
	MODE_GS,       //todo
	MODE_XG        //todo
}MidiMode;

typedef enum{
	NOTE_MELODY, NOTE_DRUM
}NoteType;

typedef enum{
	TRACK_MELODY, TRACK_DRUM
}TrackType;

typedef enum{
	DIST_OFF, DIST_HARD, DIST_SOFT, DIST_ASSYM
}DistortionMode;

typedef struct{
	uint16_t divider;
	uint32_t idx;
	EnvelopeStatus stat;
	uint16_t attack;
	uint16_t decay;
	uint16_t sustainLevel;
	uint16_t sustainRate;
	uint16_t release;
	uint16_t out;
}EnvelopeGen;

typedef struct{
	WaveType type;

	uint8_t triangle_peak_time;
	uint8_t triangle_peak_time_buf;

	float freq;
	float freqbuf;

	int16_t pitch;
	int16_t pitchbendsensitivity;
	float pitchScaleFactor;

	float pitch_sweep_ScaleFactor;
	uint16_t pitch_sweep_idx;
	uint16_t pitch_sweep_divider;
	uint8_t pitch_sweep_spd;
	PBSweepType pitch_sweep_type;

	float modScaleFactor;

	uint32_t pointer;
	uint32_t angular;

	uint32_t angular_pcm;

	int16_t out;
}WaveGen;

typedef struct{
	WaveType type;
	float multiply;
	uint32_t pointer;
	uint32_t angular;
	uint16_t gain;
	int16_t out;
}RingMod;

typedef struct{
	float prev_in, prev_out;
}DCBlocker;

typedef struct{
	WaveGen wav;
	RingMod ringmod;
	EnvelopeGen env;
	NoteStatus stat;
	NoteType ntype;
	uint8_t wavenum;
	uint16_t velocity;
	uint8_t ch;	//set 255 means "NO OUTPUT"
	uint8_t ch_dcblock;//when ch is 255, ch_dcblock continue previous value.
	int16_t rightout;
	int16_t leftout;
	int32_t out;
	uint8_t out_gain;
	float unisonScaleFactor;
	int8_t unisonScale;
	uint16_t noise_prev_out;
	uint8_t note_number;
}Operator;


typedef enum{
	DSP_NONE, DSP_GM, DSP_GS, DSP_SYSMODE1, DSP_SYSMODE2, DSP_XG, DSP_CURE_HYBRID, DSP_CURE_ORIG, DSP_BUFFER_FULL,DSP_OVERLOAD
}DisplayMessageType;

typedef struct{

	uint8_t volume[CHNUM];
	uint8_t expression[CHNUM];

	uint8_t volume_ref[CHNUM];
	uint8_t expression_ref[CHNUM];

	uint8_t master_volume;

	uint16_t delay_time;
	uint8_t delay_feedback_gain;
	uint8_t delay_feedback_ison;

	uint8_t delay_level[CHNUM];
	uint8_t chorus_level[CHNUM];
	uint8_t reverb_level[CHNUM];

	uint8_t pan[CHNUM];
	uint8_t pan_ref[CHNUM];

	uint32_t modulation_pointer[CHNUM];
	uint32_t modulation_angular[CHNUM];
	uint32_t modulation_PLL_idx[CHNUM];
	float modulation_scalefactor[CHNUM];
	uint8_t modulation[CHNUM];
	uint8_t modulation_rate[CHNUM];

	uint8_t cutoff[CHNUM];
	uint8_t resonance[CHNUM];
	FilterType ftype[CHNUM];

	MidiMode mode;
	TrackType tr_type[CHNUM];

	DistortionMode dst_mode[CHNUM];
	uint8_t dst_level[CHNUM];
	uint8_t dst_gain[CHNUM];

	uint8_t unison_num[CHNUM];// a number of operator activating at same time. //max=UNISON_MAX_NUM
	int8_t unison_detune[CHNUM]; //detune cents between operator.

	DisplayMessageType disp_msg_type;

}SynthSettings;


typedef struct{
	float fc, Q;
	int16_t in1, in2, out1, out2;
	float b0a0, b1a0, b2a0, a1a0, a2a0;
}SynthFilterSettings;

typedef struct{
	float prev_out[2];
	uint8_t detect_left_time_r, detect_left_time_l;
}SynthPeakIndicator;

typedef struct{
	uint8_t spare_voice_num_counter;
	uint8_t voice_num;
	bool isFullyVoiceDetected;
}SynthVoiceNumberStatus;

//Public variables
extern Operator au_operator[OPNUM];
extern Audio au_out[CHNUM];
extern SynthSettings synth_settings;
extern SynthFilterSettings flt_settings[CHNUM];
extern SynthPeakIndicator synth_peak_detect;
extern SynthVoiceNumberStatus synth_voice_number_status;
extern bool synth_is_clipped;


//Public functions
extern void cureSynthOperatorInit(Operator *op);
extern FUNC_STATUS cureSynthInit();
extern void cureSynthSettingInit();

void cureSynthGetOutput(int16_t output[OUTPUT_CH_NUM]);
void cureSynthGenerateTestSignalSquare(int16_t output[OUTPUT_CH_NUM]);

extern void cureSynthSetNoteON(Operator *op, uint8_t sc, uint8_t vel, uint8_t midich);
extern void cureSynthSetNoteON_DRUM(Operator *op, uint16_t scale_and_realscale, uint8_t vel, uint8_t midich);
extern void cureSynthSetNoteOFF(Operator *op);
extern void cureSynthSetNoteOFFImmediately(Operator *op);
extern void cureSynthOperatorApplyPitchBendAndTuning(Operator *op, float tuning_scalefactor);
extern void cureSynthSetFilter(uint8_t ch);
extern void cureSynthAllSoundOff(uint8_t ch);

extern void cureSynthOperatorCopy(Operator *src, Operator *dest);



#endif /* CURESYNTH_H_ */
