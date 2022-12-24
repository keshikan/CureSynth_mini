/*
 *******************************************************************************
 *  [curemidi.h]
 *  This module is for analyzing MIDI messages and controlling "curesynth.h/c"
 *
 *  This program is under the terms of the GPLv3.
 *  https://www.gnu.org/licenses/gpl-3.0.html
 *
 *  Copyright(c) 2017 Keshikan (www.keshikan.net)
 *******************************************************************************
 */

#ifndef _CUREMIDI_H_
#define _CUREMIDI_H_



#define CUREMIDI_BUFFER_LENGTH (256)
#define MIDI_DATABYTE_MAX (128)

#define CENT_TABLE_NUM (1200)

//input channel for cureMidiEventIsGenerated()
#define MIDI_INPUT_SRC_NUM (2)

typedef enum{
	SRC_USB, SRC_UART
}MIDISource;

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include "./curelib_inc/curesynth.h"
#include "./curelib_inc/curebuffer_static.h"
#include "./curelib_inc/curemisc.h"

#include "displaytypes.h"

//debug
#include "curelib_inc/curedbg.h"

#define GM1_EXCEPT(n) if(MODE_GM1 != synth_settings.mode){(n);}

//todo: for inline
//#define cureMidiBufferEnqueue(f1) ( cureRingBufferU8Enqueue(&rxbuf, f1) )
//#define cureMidiBufferDequeue() ( cureRingBufferU8Dequeue(&rxbuf, &midi_buf) )

////public typedef////
typedef enum{
	START_ANALYSIS,    // Initial Status, including exception.
	WAIT_DATA1,        // Waiting data byte(1st byte)
	WAIT_DATA2,        // Waiting data byte(2nd byte)
	WAIT_SYSTEM_DATA,  // Waiting data byte(system exclusive)
	END_ANALYSIS,       // Analysis is ended.
	IGNORE_DATA,		//Ignore data
}AnalysisStatus;

typedef enum{
	EXCC_START, EXCC_PARAM, EXCC_DAT
}ExtendCCStatus;

typedef enum{
	MSG_NOTHING,    // Exception(can't resolved, missing data, etc.)
	MSG_NOTE_ON,    // Note-on message
	MSG_NOTE_OFF,   // Note-off message
	MSG_PITCH,      // PitchBend message
	MSG_SYSEX,      // System Exclusive message
	MSG_CC,         // Control Change message
	MSG_PROG,       // Program Change message
	MSG_RPN,        // RPN message
	MSG_NRPN        // NRPN message
}EventType;

typedef struct{
	EventType type;
	uint8_t channel;
	uint8_t data_byte[MIDI_DATABYTE_MAX]; //data_byte[0]=MSB, [1]=LSB, [2]=OTHER...(e.g. sysEx, Control Change...)
}MIDIEvent;

typedef struct{
	AnalysisStatus stat;
	EventType type;
	uint8_t channel;
	uint8_t data_idx;
}MidiAnalysisStatus;

typedef enum{
 OPERATOR_OFF = 255
}OperatorStatus;

typedef struct{
	OperatorStatus stat[128];//status of note number (is ON or OFF)
}MidiNoteOnStatus;

typedef struct{
	NoteStatus ison[OPNUM];
}MidiOperatorOnStatus;

typedef enum{
	RPN_PITCH_SENS, RPN_FINE_TUNE, RPN_COARSE_TUNE, RPN_UNDEFINED, RPN_NULL
}RPNType;

typedef enum{
	NRPN_WAVE_TYPE, NRPN_WAVE_NUM, NRPN_OUTPUT_GAIN, NRPN_RING_MULTIPLY, NRPN_RING_GAIN,
	NRPN_SWEEP_TYPE, NRPN_SWEEP_SPD, NRPN_TRI_PEAKTIME, NRPN_SQU_DUTY,
	NRPN_SUSTAIN_RATE, NRPN_SUSTAIN_LEVEL, NRPN_DELAY_TIME, NRPN_FEEDBK_GAIN,
	NRPN_DIST_TYPE, NRPN_DIST_LEVEL, NRPN_DIST_GAIN, NRPN_DIST_CHANNEL,
	NRPN_UNISON_NUM, NRPN_UNISON_DET,
	NRPN_DRUM_TRACK, NRPN_CHORUS_DEPTH, NRPN_CHORUS_FREQ, NRPN_REVERB_TYPE,
	NRPN_UNDEFINED, NRPN_NULL
}NRPNType;

typedef enum{
	CHOSE_RPN, CHOSE_NRPN
}RPN_NRPN_choose;

typedef enum{
	STN_INIT, STN_REGISTERED, STN_CONTINUE
}SostenutoState;

typedef struct{
	MidiNoteOnStatus operator_to_note;//status of ch1 to 16
	MidiOperatorOnStatus operator_to_channel;//status of ch1 to 16
	Operator operatorsettings;
	bool isHoldOn;
	bool isSostenutoOn;
	bool hold_note_list[128];
	SostenutoState sostenuto_note_list[128];
	uint8_t rpn_msb, rpn_lsb;//255: magic number (means no assignment. It will be cleared after RPN NULL received.)
	RPNType rpn_type;
	uint8_t nrpn_msb, nrpn_lsb;//255: magic number (means no assignment. It will be cleared after RPN NULL received.)
	NRPNType nrpn_type;
	RPN_NRPN_choose rpn_nrpn_choose;
	uint8_t fine_tune_msb, fine_tune_lsb, coarse_tune_msb;
	float tuning_scalefactor;
}MidiChannel;

typedef struct{
	MidiChannel channel[CHNUM];
	uint8_t front_opnum;	//A operator number to use when it received Note-on message.
}MidiMasterChannel;

typedef struct{

	NoteType ntype;
	WaveType wtype;

	uint8_t wavenum;

	uint8_t ringmod_multiply;
	uint8_t ringmod_gain;

	PBSweepType pitch_sweep_type;
	uint8_t pitch_sweep_spd;

	uint8_t attack;
	uint8_t decay;
	uint8_t sustainLevel;
	uint8_t sustainRate;
	uint8_t release;

	uint8_t notenum;// Drum Only
	uint8_t out_gain;

}MidiPatch;


////public variables////
extern MidiMasterChannel master;
extern RingBufferU8 rxbuf;


////public func////
extern void cureMidiMain();
extern FUNC_STATUS cureMidiInit();
//extern BUFFER_STATUS cureMidiBufferEnqueue(uint8_t* inputc);
//extern BUFFER_STATUS cureMidiBufferDequeue();
//extern bool cureMidiBufferIsEmpty();


extern void cureMidiGetDisplayData(DispInfo* dp);
extern void cureMidiDisplayDataInit(DispInfo* dp);
extern bool cureMidiIsGetDisplayMessage(char* txt);

extern void cureMidiGetDisplayChangeInfo(DispSettingChangeInfo* dpc_to_curedraw);


extern bool cureMidiEventIsGenerated(uint8_t midi_buf, uint8_t src_no);
extern void cureMidiAnalyzeEvent(uint8_t src_no);

extern void cureMidiResetSynth();

extern void cureMidiDirectInput(uint8_t midibyte);
extern void cureMidiResetFromBufferFull();

//overload
extern void cureMidiResetFromOverload();
extern void cureMidiEnableOverloadFlag();
extern void cureMidiDisableOverloadFlag();
extern bool cureMidiGetOverloadFlag();

//debug
extern bool cureMidiIsDebugDisplay();


#endif /* CUREMIDI_H_ */
