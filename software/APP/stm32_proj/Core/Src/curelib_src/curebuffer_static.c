/*
 *******************************************************************************
 *  [curebuffer.c]
 *  This module is for FIFO buffer.
 *
 *  This program is under the terms of the GPLv3.
 *  https://www.gnu.org/licenses/gpl-3.0.html
 *
 *  Copyright(c) 2017 Keshikan (www.keshikan.net)
 *******************************************************************************
 */

#include "curebuffer_static.h"
#include <stdint.h>
#include <stdlib.h>


/////////////////////////////
//methods for uint8_t FIFO.
/////////////////////////////

BUFFER_STATUS cureRingBufferU8Init(RingBufferU8 *rbuf, uint32_t buf_len, uint8_t* buf_ptr)
{

	cureRingBufferU8Free(rbuf);

	if(NULL == buf_ptr){
		return BUFFER_FAILURE;
	}

	rbuf->buffer = buf_ptr;
	rbuf->length = buf_len;

	for(uint32_t i=0; i<(rbuf->length); i++){
		rbuf->buffer[i] = 0;
	}

	return BUFFER_SUCCESS;
}

BUFFER_STATUS cureRingBufferU8Free(RingBufferU8 *rbuf)
{
	rbuf->buffer = NULL;

	rbuf->idx_front = rbuf->idx_rear = 0;
	rbuf->length = 0;

	return BUFFER_SUCCESS;
}

BUFFER_STATUS cureRingBufferU8Enqueue(RingBufferU8 *rbuf, uint8_t *inputc)
{
	if( ((rbuf->idx_front +1)&(rbuf->length -1)) == rbuf->idx_rear ){//buffer overrun error occurs.
		return BUFFER_FAILURE;
	}else{

		rbuf->buffer[rbuf->idx_front]=  *inputc;
		rbuf->idx_front++;
		rbuf->idx_front &= (rbuf->length -1);
		return BUFFER_SUCCESS;
	}
}

BUFFER_STATUS cureRingBufferU8Dequeue(RingBufferU8 *rbuf, uint8_t *ret)
{
	if(rbuf->idx_front == rbuf->idx_rear){//if buffer underrun error occurs.
		return BUFFER_FAILURE;
	}else{

		*ret = (rbuf->buffer[rbuf->idx_rear]);
		rbuf->idx_rear++;
		rbuf->idx_rear &= (rbuf->length -1);
		return BUFFER_SUCCESS;
	}
}

//debug
uint16_t _cureRingBufferU8GetUsedSize(RingBufferU8 *rbuf)
{
	if(rbuf->idx_front >= rbuf->idx_rear){
		return rbuf->idx_front - rbuf->idx_rear;
	}else{
		return rbuf->idx_front + rbuf->length - rbuf->idx_rear;
	}

}

void cureRingBufferU8Flush(RingBufferU8 *rbuf)
{
	if(NULL == rbuf->buffer){
		return;
	}

	rbuf->idx_rear = rbuf->idx_front;
}

/////////////////////////////
//methods for int16_t FIFO.
/////////////////////////////

BUFFER_STATUS cureRingBuffer16Init(RingBuffer16 *rbuf, uint16_t buf_len, int16_t* buf_ptr)
{
	cureRingBuffer16Free(rbuf);

	if(NULL == buf_ptr){
		return BUFFER_FAILURE;
	}

	rbuf->buffer = buf_ptr;
	rbuf->length = buf_len;

	for(uint32_t i=0; i<(rbuf->length); i++){
		rbuf->buffer[i] = 0;
	}

	return BUFFER_SUCCESS;
}

BUFFER_STATUS cureRingBuffer16Free(RingBuffer16 *rbuf)
{
	rbuf->buffer = NULL;
	rbuf->idx_front = rbuf->idx_rear = 0;
	rbuf->length = 0;

	return BUFFER_SUCCESS;
}

BUFFER_STATUS cureRingBuffer16Enqueue(RingBuffer16 *rbuf, int16_t *inputc)
{
	if( ((rbuf->idx_front +1)&(rbuf->length -1)) == rbuf->idx_rear ){//buffer overrun error occurs.
		return BUFFER_FAILURE;
	}else{
		rbuf->buffer[rbuf->idx_front]=  *inputc;
		rbuf->idx_front++;
		rbuf->idx_front &= (rbuf->length -1);
		return BUFFER_SUCCESS;
	}
}

BUFFER_STATUS cureRingBuffer16EnqueueIgnoreErr(RingBuffer16 *rbuf, int16_t *inputc)
{

	rbuf->buffer[rbuf->idx_front]= *inputc;
	rbuf->idx_front++;
	rbuf->idx_front &= (rbuf->length -1);
	return BUFFER_SUCCESS;

}


BUFFER_STATUS cureRingBuffer16Dequeue(RingBuffer16 *rbuf, int16_t *ret)
{
	if(rbuf->idx_front == rbuf->idx_rear){//if buffer underrun error occurs.
		return BUFFER_FAILURE;
	}else{
		*ret = (rbuf->buffer[rbuf->idx_rear]);
		rbuf->idx_rear++;
		rbuf->idx_rear &= (rbuf->length -1);
		return BUFFER_SUCCESS;
	}
}


BUFFER_STATUS cureRingBuffer16GetElement(RingBuffer16 *rbuf, int16_t *ret, uint16_t delaynum, uint16_t delay_buffer_length)
{

	if(rbuf->idx_front >= delaynum){
		rbuf->idx_rear = rbuf->idx_front - delaynum;
	}else{
		rbuf->idx_rear = delay_buffer_length - (delaynum - rbuf->idx_front);
	}
		*ret = (rbuf->buffer[rbuf->idx_rear]);
		return BUFFER_SUCCESS;

}

/////////////////////////////
//methods for uint32_t FIFO.
/////////////////////////////

BUFFER_STATUS cureRingBufferU32Init(RingBufferU32 *rbuf, uint16_t buf_len, uint32_t* buf_ptr)
{

	cureRingBufferU32Free(rbuf);

	if(NULL == buf_ptr){
		return BUFFER_FAILURE;
	}

	rbuf->buffer = buf_ptr;
	rbuf->length = buf_len;

	for(uint32_t i=0; i<(rbuf->length); i++){
		rbuf->buffer[i] = 0;
	}

	return BUFFER_SUCCESS;
}

BUFFER_STATUS cureRingBufferU32Free(RingBufferU32 *rbuf)
{
	rbuf->buffer = NULL;
	rbuf->idx_front = rbuf->idx_rear = 0;
	rbuf->length = 0;

	return BUFFER_SUCCESS;
}

BUFFER_STATUS cureRingBufferU32Enqueue(RingBufferU32 *rbuf, uint32_t *inputc)
{
	if( ((rbuf->idx_front +1)&(rbuf->length -1)) == rbuf->idx_rear ){//buffer overrun error occurs.
		return BUFFER_FAILURE;
	}else{
		rbuf->buffer[rbuf->idx_front]=  *inputc;
		rbuf->idx_front++;
		rbuf->idx_front &= (rbuf->length -1);
		return BUFFER_SUCCESS;
	}
}

BUFFER_STATUS cureRingBufferU32EnqueueIgnoreErr(RingBufferU32 *rbuf, uint32_t *inputc)
{

	rbuf->buffer[rbuf->idx_front]= *inputc;
	rbuf->idx_front++;
	rbuf->idx_front &= (rbuf->length -1);
	return BUFFER_SUCCESS;

}


BUFFER_STATUS cureRingBufferU32Dequeue(RingBufferU32 *rbuf, uint32_t *ret)
{
	if(rbuf->idx_front == rbuf->idx_rear){//if buffer underrun error occurs.
		return BUFFER_FAILURE;
	}else{
		*ret = (rbuf->buffer[rbuf->idx_rear]);
		rbuf->idx_rear++;
		rbuf->idx_rear &= (rbuf->length -1);
		return BUFFER_SUCCESS;
	}
}


BUFFER_STATUS cureRingBufferU32GetElement(RingBufferU32 *rbuf, uint32_t *ret, uint32_t delaynum, uint32_t delay_buffer_length)
{
	uint16_t buf;


	if(rbuf->idx_front >= delaynum){
		buf = rbuf->idx_front - delaynum;
	}else{
		buf = delay_buffer_length - (delaynum - rbuf->idx_front);
	}
		*ret = (rbuf->buffer[buf]);
		return BUFFER_SUCCESS;

}
