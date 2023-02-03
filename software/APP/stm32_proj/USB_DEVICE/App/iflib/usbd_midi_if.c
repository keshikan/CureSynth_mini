/**
  ******************************************************************************
  * @file           : usbd_midi_if.c
  * @brief          :
  ******************************************************************************

    (CC at)2016 by D.F.Mac. @TripArts Music

  ******************************************************************************

    Modified by keshikan (www.keshikan.net) 2018
    The license is (CC BY 4.0), and takes over from original usbd_midi_if.h/c.

    See also original source code page.
    https://github.com/mimuz/mimuz-tuch/blob/master/STM32/

  ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "usbd_midi_if.h"
#include <stdbool.h>


// basic midi rx/tx functions
static uint16_t MIDI_DataRx(uint8_t *msg, uint16_t length);
static uint16_t MIDI_DataTx(uint8_t *msg, uint16_t length);

// for Cure Series
#define MIDI_BUFFER_SIZ (8192)//FIFO buffer byte size for midi message buffer. need 2^n
#define MIDI_BUFFER_FULL_THR (8000)

bool is_usb_midi_buffer_full = false;

RingBufferU8 rbuf_usb_rx[MIDI_OUT_JACK_NUM]; //for input from USB
RingBufferU8 rbuf_jack_rx[MIDI_IN_JACK_NUM];  //for input from MIDI-IN jack
uint8_t rbuf_usb_rx_buffer[MIDI_OUT_JACK_NUM][MIDI_BUFFER_SIZ];
uint8_t rbuf_jack_rx_buffer[MIDI_IN_JACK_NUM][MIDI_BUFFER_SIZ];

//for receiving midi data from jack
UM_MidiAnalysisStatus analyzed_status_if[MIDI_IN_JACK_NUM];
UM_MIDIEvent midi_event_if[MIDI_IN_JACK_NUM];	//received midi data

uint8_t rx_midi_msg[MIDI_IN_JACK_NUM];


FUNC_STATUS midiInit()
{
	uint32_t i,j;

	for(i=0; i<MIDI_OUT_JACK_NUM; i++){
		if(BUFFER_SUCCESS != cureRingBufferU8Init(&rbuf_usb_rx[i], MIDI_BUFFER_SIZ, rbuf_usb_rx_buffer[i]))
		{
			return FUNC_ERROR;
		}
	}

	for(i=0; i<MIDI_IN_JACK_NUM; i++){
		if(BUFFER_SUCCESS != cureRingBufferU8Init(&rbuf_jack_rx[i], MIDI_BUFFER_SIZ, rbuf_jack_rx_buffer[i]))
		{
			return FUNC_ERROR;
		}
	}

	//Init RX
	for(i=0; i<MIDI_IN_JACK_NUM; i++){

		rx_midi_msg[i] = 0x00;
		analyzed_status_if[i].data_idx = 0;
		midi_event_if[i].length = 0;

		for(j=0; j<UM_MIDI_SENDDATA_MAX; j++){
			midi_event_if[i].midi_byte[j] = 0x00;
		}
	}

	return FUNC_SUCCESS;
}

FUNC_STATUS midiGetFromUsbRx(uint8_t cable_num, uint8_t* dat)
{
	__disable_irq();//This is irq guard to avoid conflicting ring buffer pointer idx.
	if(BUFFER_SUCCESS != cureRingBufferU8Dequeue(&rbuf_usb_rx[cable_num], dat))
	{
		__enable_irq();
		return FUNC_ERROR;
	}

	__enable_irq();
	return FUNC_SUCCESS;
}

FUNC_STATUS midiGetFromJackRx(uint8_t cable_num)
{
	__disable_irq();//This is irq guard to avoid conflicting ring buffer pointer idx.
	if(BUFFER_SUCCESS != cureRingBufferU8Dequeue(&rbuf_jack_rx[cable_num], &rx_midi_msg[cable_num]))
	{
		__enable_irq();
		return FUNC_ERROR;
	}

	__enable_irq();
	return FUNC_SUCCESS;
}

FUNC_STATUS midiSetFromJackRx(uint8_t cable_num, uint8_t* dat)
{
	if(BUFFER_SUCCESS != cureRingBufferU8Enqueue(&rbuf_jack_rx[cable_num], dat))
	{
		return FUNC_ERROR;
	}

	return FUNC_SUCCESS;
}


bool isUsbRxBufEmpty(uint8_t cable_num)
{
	if( 0 != _cureRingBufferU8GetUsedSize(&rbuf_usb_rx[cable_num]) ){
		return false;
	}

	return true;
}

inline uint16_t getUsbRxBufUsedSize(uint8_t cable_num)
{
	return _cureRingBufferU8GetUsedSize(&rbuf_usb_rx[cable_num]);
}

bool isJackRxBufEmpty(uint8_t cable_num)
{
	if( 0 != _cureRingBufferU8GetUsedSize(&rbuf_jack_rx[cable_num]) ){
		return false;
	}

	return true;
}

bool isAllUsbRxBufEmpty()
{
	for(uint32_t i=0; i<MIDI_OUT_JACK_NUM; i++){
		if(0 != _cureRingBufferU8GetUsedSize(&rbuf_usb_rx[i]))
		{
			return false;
		}
	}

	return true;
}

bool isRxBufEmpty()
{
	uint32_t i;

	for(i=0; i<MIDI_OUT_JACK_NUM; i++){
		if(0 != _cureRingBufferU8GetUsedSize(&rbuf_usb_rx[i]))
		{
			return false;
		}
	}

	for(i=0; i<MIDI_IN_JACK_NUM; i++){
		if(0 != _cureRingBufferU8GetUsedSize(&rbuf_jack_rx[i]))
		{
			return false;
		}
	}
	return true;
}

bool isUsbRxBufferFull()
{
	return is_usb_midi_buffer_full;
}

void usbMidiBufferFlush(uint8_t cable_num)
{
	cureRingBufferU8Flush(&rbuf_usb_rx[cable_num]);
	is_usb_midi_buffer_full = false;
}

USBD_MIDI_ItfTypeDef USBD_Interface_fops_FS =
{
  MIDI_DataRx,
  MIDI_DataTx
};


static uint16_t MIDI_DataRx(uint8_t *msg, uint16_t length){
  uint16_t cnt;
  uint16_t msgs = length / 4;
  uint16_t chk = length % 4;
  uint8_t u8b;
  uint8_t midi_size;

  if(0 != chk)
  {
	  return 0;
  }

  for(uint32_t cnt_msgs = 0; cnt_msgs < msgs; cnt_msgs++){

	  uint8_t cable_num = (msg[0 + 4*cnt_msgs] & 0xF0) >> 4;
	  uint8_t code_idx_num = msg[0 + 4*cnt_msgs] & 0x0F;

	  switch (code_idx_num) {

	  	  //not defined
		  case 0x0:
		  case 0x1:
			  midi_size = 0;
			  break;

		  //1byte message
		  case 0x5:
		  case 0xF:
			  midi_size = 1;
			  break;

		  //2byte message
		  case 0x2:
		  case 0x6:
		  case 0xC:
		  case 0xD:
			  midi_size = 2;
			  break;

		  //3byte message
		  case 0x3:
		  case 0x4:
		  case 0x7:
		  case 0x8:
		  case 0x9:
		  case 0xA:
		  case 0xB:
		  case 0xE:
			  midi_size = 3;
			  break;

		  default:
			  midi_size = 0;
			  break;
	  }

	  for(cnt = 0;cnt < midi_size;cnt ++){
		  u8b = *(msg + 4*cnt_msgs + cnt + 1);
		  cureRingBufferU8Enqueue(&rbuf_usb_rx[cable_num], &u8b);
	  }
  }

  //buffer full judge
  if(MIDI_BUFFER_FULL_THR < getUsbRxBufUsedSize(0)){
	  is_usb_midi_buffer_full = true;
  }

  return 0;
}

void sendMidiMessage(uint8_t *msg, uint16_t size){
  if(size == 4){
    MIDI_DataTx(msg, size);
  }
}

static uint16_t MIDI_DataTx(uint8_t *msg, uint16_t length){
  uint32_t i = 0;
  while (i < length) {
    APP_Rx_Buffer[APP_Rx_ptr_in] = *(msg + i);
    APP_Rx_ptr_in++;
    i++;
    if (APP_Rx_ptr_in == APP_RX_DATA_SIZE) {
      APP_Rx_ptr_in = 0;
    }
  }
  return USBD_OK;
}

bool midiIsEventGenerated(uint8_t cable_num)
{
	uint8_t upper_half_byte= (rx_midi_msg[cable_num]) & 0xF0;

	if( upper_half_byte & 0x80 ){//0x80-0xFF:status byte

		switch(upper_half_byte){

			case 0xF0://0xF0-0xFF:system message
				switch(rx_midi_msg[cable_num]){

					case 0xF0://SysEx Start
						analyzed_status_if[cable_num].data_idx = 0;
						midi_event_if[cable_num].midi_byte[ analyzed_status_if[cable_num].data_idx++ ] = rx_midi_msg[cable_num];
						analyzed_status_if[cable_num].type = UM_MSG_SYSEX;
						analyzed_status_if[cable_num].stat = UM_WAIT_SYSTEM_DATA;
						break;

					case 0xF7://SysEx End
						midi_event_if[cable_num].midi_byte[ analyzed_status_if[cable_num].data_idx++ ] = rx_midi_msg[cable_num];
						midi_event_if[cable_num].length = analyzed_status_if[cable_num].data_idx;
						analyzed_status_if[cable_num].stat = UM_END_ANALYSIS;
						break;

					case 0xF2://Song Position
						midi_event_if[cable_num].midi_byte[0] = rx_midi_msg[cable_num];
						analyzed_status_if[cable_num].type = UM_MSG_THREE_BYTE;
						analyzed_status_if[cable_num].stat = UM_WAIT_DATA1;
						break;

					case 0xF1://Time Code
					case 0xF3://Song Select
						midi_event_if[cable_num].midi_byte[0] = rx_midi_msg[cable_num];
						analyzed_status_if[cable_num].type = UM_MSG_TWO_BYTE;
						analyzed_status_if[cable_num].stat = UM_WAIT_DATA1;
						break;

					case 0xF4://Undefined
					case 0xF5://Undefined
					case 0xF6://Tune request
					case 0xF8://Timing clock
					case 0xF9://Undefined
					case 0xFA://Start
					case 0xFB://Continue
					case 0xFC://Stop
					case 0xFD://Undefined
					case 0xFE://Active Sensing
					case 0xFF://Reset
						midi_event_if[cable_num].midi_byte[0] = rx_midi_msg[cable_num];
						midi_event_if[cable_num].length = 1;
						analyzed_status_if[cable_num].type = UM_MSG_ONE_BYTE;
						analyzed_status_if[cable_num].stat = UM_END_ANALYSIS;
						break;
				}
				analyzed_status_if[cable_num].is_system_common = true;
				break;

			case 0x80://Note Off
			case 0x90://Note On
			case 0xA0://Polyphonic key-pressure
			case 0xB0://ControlChange
			case 0xE0://PitchBend
				midi_event_if[cable_num].midi_byte[0] = rx_midi_msg[cable_num];
				analyzed_status_if[cable_num].type = UM_MSG_THREE_BYTE;
				analyzed_status_if[cable_num].stat = UM_WAIT_DATA1;
				analyzed_status_if[cable_num].is_system_common = false;
				break;

			case 0xC0://Program Change
			case 0xD0://Channel pressure
				midi_event_if[cable_num].midi_byte[0] = rx_midi_msg[cable_num];
				analyzed_status_if[cable_num].type = UM_MSG_TWO_BYTE;
				analyzed_status_if[cable_num].stat = UM_WAIT_DATA1;
				analyzed_status_if[cable_num].is_system_common = false;
				break;

			default:
				analyzed_status_if[cable_num].type = UM_MSG_NOTHING;
				analyzed_status_if[cable_num].stat = UM_START_ANALYSIS;
				analyzed_status_if[cable_num].is_system_common = false;
				break;
		}

	}else{//0x00-0x7F:data byte

		switch(analyzed_status_if[cable_num].stat){

			case UM_WAIT_DATA1:
				midi_event_if[cable_num].midi_byte[1] = rx_midi_msg[cable_num];

				if(UM_MSG_THREE_BYTE == analyzed_status_if[cable_num].type ){
					analyzed_status_if[cable_num].stat = UM_WAIT_DATA2;
				}else if( UM_MSG_TWO_BYTE == analyzed_status_if[cable_num].type ){
					midi_event_if[cable_num].length = 2;
					analyzed_status_if[cable_num].stat = UM_END_ANALYSIS;
				}else{
					analyzed_status_if[cable_num].stat = UM_START_ANALYSIS;
				}
				break;

			case UM_WAIT_DATA2:
				if(UM_MSG_THREE_BYTE == analyzed_status_if[cable_num].type ){
					midi_event_if[cable_num].midi_byte[2] = rx_midi_msg[cable_num];
					midi_event_if[cable_num].length = 3;
					analyzed_status_if[cable_num].stat = UM_END_ANALYSIS;
				}else{
					analyzed_status_if[cable_num].stat = UM_START_ANALYSIS;
				}
				break;

			case UM_WAIT_SYSTEM_DATA:
				midi_event_if[cable_num].midi_byte[ analyzed_status_if[cable_num].data_idx++ ] = rx_midi_msg[cable_num];

				if(analyzed_status_if[cable_num].data_idx > (UM_MIDI_SENDDATA_MAX - 1) ){
					analyzed_status_if[cable_num].stat = UM_END_ANALYSIS;
				}
				break;

			case UM_END_ANALYSIS://running status:When status byte is omitted.
				midi_event_if[cable_num].midi_byte[1] = rx_midi_msg[cable_num];
				if(UM_MSG_THREE_BYTE == analyzed_status_if[cable_num].type){
					analyzed_status_if[cable_num].stat = UM_WAIT_DATA2;
				}else if(UM_MSG_TWO_BYTE == analyzed_status_if[cable_num].type){
					midi_event_if[cable_num].length = 2;
					analyzed_status_if[cable_num].stat = UM_END_ANALYSIS;
				}
				break;

			case UM_START_ANALYSIS:
				break;

			default:
				break;
		}
	}

	if(UM_END_ANALYSIS == analyzed_status_if[cable_num].stat){
		return true;
	}else{
		return false;
	}

}


void midiGenerateUsbPacket(uint8_t cable_num)
{
	uint8_t msg_buf[4] = {0x00,0x00,0x00,0x00};
	uint32_t cnt_remain=0, cnt_length;

	switch(analyzed_status_if[cable_num].type){

		case UM_MSG_ONE_BYTE:
			//byte 0: cable number + code index number
			msg_buf[0] = (cable_num << 4) + ((midi_event_if[cable_num].midi_byte[0] & 0xF0) >> 4);

			sendMidiMessage(msg_buf,4);
			break;

		case UM_MSG_TWO_BYTE:
		case UM_MSG_THREE_BYTE:
			//byte 0: cable number + code index number
			if(analyzed_status_if[cable_num].is_system_common){
				msg_buf[0] = (cable_num << 4) + midi_event_if[cable_num].length;
			}else{
				msg_buf[0] = (cable_num << 4) + ((midi_event_if[cable_num].midi_byte[0] & 0xF0) >> 4);
			}

			//byte 1-3
			for(uint32_t i=0; i<midi_event_if[cable_num].length; i++){
				msg_buf[i+1] = midi_event_if[cable_num].midi_byte[i];
			}

			sendMidiMessage(msg_buf,4);
			break;

		case UM_MSG_SYSEX:
			for(cnt_length = 0; cnt_length < midi_event_if[cable_num].length; cnt_length++){
				if( 0xF7 !=midi_event_if[cable_num].midi_byte[cnt_length] ){
					if(2 == cnt_remain){
						msg_buf[3] = midi_event_if[cable_num].midi_byte[cnt_length];
						msg_buf[0] = (cable_num << 4) + 0x4;	//SysEx starts or continues
						sendMidiMessage(msg_buf,4);
					}else{
						msg_buf[1 + cnt_remain] = midi_event_if[cable_num].midi_byte[cnt_length];
					}

				}else{

					switch (cnt_remain) {
						case 0:
							msg_buf[0] = (cable_num << 4) + 0x5;	//SysEx ends with following single byte
							msg_buf[1] = 0xF7;
							msg_buf[2] = msg_buf[3] = 0x00;
							sendMidiMessage(msg_buf,4);
							break;

						case 1:
							msg_buf[0] = (cable_num << 4) + 0x6;	//SysEx ends with following two bytes.
							msg_buf[2] = 0xF7;
							msg_buf[3] = 0x00;
							sendMidiMessage(msg_buf,4);
							break;

						case 2:
							msg_buf[0] = (cable_num << 4) + 0x7;	//SysEx ends with following three bytes.
							msg_buf[3] = 0xF7;
							sendMidiMessage(msg_buf,4);
							break;
						default:
							break;
					}
				}

				cnt_remain++;
				if(cnt_remain >=3){
					cnt_remain = 0;
				}
			}
			break;

		default:
			break;
	}
	USBD_MIDI_SendPacket();
}

void midiInProcess(){

	for(uint32_t cable_num=0; cable_num<MIDI_IN_JACK_NUM; cable_num++){
		while( FUNC_SUCCESS == midiGetFromJackRx(cable_num) ){
			if( midiIsEventGenerated(cable_num) ){// Generate MIDI event from UART buffer.
				//Analyze MIDI Message.
				midiGenerateUsbPacket(cable_num);
			}
		}
	}
}

