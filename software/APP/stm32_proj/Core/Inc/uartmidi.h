/*
 * uartmidi.h
 *
 *  Created on: 2022/11/24
 *      Author: Keshikan
 */

#ifndef INC_UARTMIDI_H_
#define INC_UARTMIDI_H_

#include "curemisc.h"
#include "curebuffer_static.h"
#include <stdint.h>
#include <stdbool.h>
#include "cmsis_gcc.h"

//midi from uart
#define UART_MIDI_BUFFER_LENGTH (256)
#define UART_MIDI_BUFFER_FULL_THR (200)


extern FUNC_STATUS cureUartMidiInit();
extern FUNC_STATUS cureUartMidiBufferEnqueue(uint8_t* inputc);
extern FUNC_STATUS cureUartMidiBufferDequeue(uint8_t* buf);
extern bool cureUartMidiBufferIsEmpty();
extern bool isUartRxBufferFull();
extern void uartMidiBufferFlush();


#endif /* INC_UARTMIDI_H_ */
