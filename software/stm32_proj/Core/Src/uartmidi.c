/*
 * uartmidi.c
 *
 *  Created on: 2022/11/24
 *      Author: Keshikan
 */

#include "uartmidi.h"

RingBufferU8 uart_midi_rx_buf;
uint8_t uart_midi_rx_buf_buffer[UART_MIDI_BUFFER_LENGTH];

bool is_uart_midi_buffer_full = false;


FUNC_STATUS cureUartMidiInit()
{
	//UART Buffer Init
	if( BUFFER_FAILURE == cureRingBufferU8Init(&uart_midi_rx_buf, UART_MIDI_BUFFER_LENGTH, uart_midi_rx_buf_buffer) ){
		return FUNC_ERROR;
	}


	return FUNC_SUCCESS;
}


FUNC_STATUS cureUartMidiBufferEnqueue(uint8_t* inputc)
{
	FUNC_STATUS ret = cureRingBufferU8Enqueue(&uart_midi_rx_buf, inputc);

	  //buffer full judge
	  if(UART_MIDI_BUFFER_FULL_THR < _cureRingBufferU8GetUsedSize(&uart_midi_rx_buf)){
		  is_uart_midi_buffer_full = true;
	  }

	return ret;
}

FUNC_STATUS cureUartMidiBufferDequeue(uint8_t* buf)
{

	__disable_irq();//This is irq guard to avoid conflicting ring buffer pointer idx.
	if(BUFFER_SUCCESS != cureRingBufferU8Dequeue(&uart_midi_rx_buf, buf))
	{
		__enable_irq();
		return FUNC_ERROR;
	}

	__enable_irq();
	return FUNC_SUCCESS;
}

bool cureUartMidiBufferIsEmpty()
{
	if( 0 == _cureRingBufferU8GetUsedSize(&uart_midi_rx_buf) ){
		return true;
	}

	return false;
}


bool isUartRxBufferFull()
{
	return is_uart_midi_buffer_full;
}

void uartMidiBufferFlush()
{
	cureRingBufferU8Flush(&uart_midi_rx_buf);
	is_uart_midi_buffer_full = false;
}
