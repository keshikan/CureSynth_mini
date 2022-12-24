/**
  ******************************************************************************
  * @file           : usbd_midi_if.h
  * @brief          : Header for usbd_midi_if file.
  ******************************************************************************

    (CC at)2016 by D.F.Mac. @TripArts Music

  ******************************************************************************

    Modified by keshikan (www.keshikan.net) 2018
    The license is (CC BY 4.0), and takes over from original usbd_midi_if.h/c.

    See also original source code page.
    https://github.com/mimuz/mimuz-tuch/blob/master/STM32/

  ******************************************************************************
 */

#ifndef __USBD_MIDI_IF_H
#define __USBD_MIDI_IF_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32h7xx_hal.h"	//please edit

#include "usbd_midi.h"
#include "usbd_desc.h"
#include "curemisc.h"
#include "curebuffer_static.h"

#define UM_MIDI_BUFFER_LENGTH (2048)	// MIDI buffer size every I/O channel.
#define UM_MIDI_SENDDATA_MAX (64)		// Maximum MIDI Event size

////public typedef////
typedef enum{
	UM_START_ANALYSIS,    // Initial Status, including exception.
	UM_WAIT_DATA1,        // Waiting data byte(1st byte)
	UM_WAIT_DATA2,        // Waiting data byte(2nd byte)
	UM_WAIT_SYSTEM_DATA,  // Waiting data byte(system exclusive)
	UM_END_ANALYSIS       // Analysis is ended.
}UM_AnalysisStatus;

typedef enum{
	UM_MSG_NOTHING,    // Exception(can't resolved, missing data, etc.)
	UM_MSG_SYSEX,      // System Exclusive message
	UM_MSG_ONE_BYTE,
	UM_MSG_TWO_BYTE,
	UM_MSG_THREE_BYTE,
}UM_EventType;

typedef struct{
	uint8_t length;
	uint8_t midi_byte[UM_MIDI_SENDDATA_MAX]; //data_byte[0]=MSB, [1]=LSB, [2]=OTHER...(e.g. sysEx, Control Change...)
}UM_MIDIEvent;

typedef struct{
	UM_AnalysisStatus stat;
	UM_EventType type;
	bool is_system_common;
	uint8_t data_idx;
}UM_MidiAnalysisStatus;

extern USBD_MIDI_ItfTypeDef  USBD_Interface_fops_FS;

//for cure series
extern FUNC_STATUS midiInit();//call before use functions in this files.
extern FUNC_STATUS midiGetFromUsbRx(uint8_t ch, uint8_t* dat);
extern FUNC_STATUS midiGetFromJackRx(uint8_t cable_num);
extern FUNC_STATUS midiSetFromJackRx(uint8_t cable_num, uint8_t* dat);
extern bool isUsbRxBufEmpty(uint8_t ch);
extern uint16_t getUsbRxBufUsedSize(uint8_t cable_num);
extern bool isJackRxBufEmpty(uint8_t ch);
extern bool isRxBufEmpty();
extern bool isAllUsbRxBufEmpty();

extern bool isUsbRxBufferFull();
extern void usbMidiBufferFlush(uint8_t cable_num);

//USB function
extern void sendMidiMessage(uint8_t *msg, uint16_t size);
extern uint8_t USBD_MIDI_SendData (USBD_HandleTypeDef *pdev, uint8_t *pBuf, uint16_t length);


// Call in main loop
extern void midiInProcess(void);
extern void USBD_MIDI_SendPacket(void);



#ifdef __cplusplus
}
#endif
  
#endif /* __USBD_MIDI_IF_H */
