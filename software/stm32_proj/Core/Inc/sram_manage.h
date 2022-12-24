/*
 * sram_manage.h
 *
 *  Created on: 2022/08/24
 *      Author: Keshikan
 */

#ifndef INC_SRAM_MANAGE_H_
#define INC_SRAM_MANAGE_H_

#include "stm32h7xx_hal.h"
#include <stdbool.h>

#define SRAM_MEMORY_SIZE (0x020000)
#define SRAM_MEMORY_MAPPED_MODE_ON

//porting
#define OSPI_TYPEDEF hospi1
#define DMA_MEMTOMEM_TYPEDEF_DEQUEUE hdma_memtomem_dma1_stream0
#define DMA_MEMTOMEM_TYPEDEF_ENQUEUE hdma_memtomem_dma1_stream3

//command definition
#define SRAM_INSTRUCTION_READ 0x03
#define SRAM_INSTRUCTION_FAST_READ 0x0B
#define SRAM_INSTRUCTION_FAST_READ_QUAD 0xEB
#define SRAM_INSTRUCTION_WRITE 0x02
#define SRAM_INSTRUCTION_WRITE_QUAD 0x38
#define SRAM_INSTRUCTION_EQIO 0x35
#define SRAM_INSTRUCTION_RSTIO 0xF5
#define SRAM_INSTRUCTION_RDMR 0xB5
#define SRAM_INSTRUCTION_WRMR 0xB1

#define SRAM_INSTRUCTION_RESET_ENABLE 0x66
#define SRAM_INSTRUCTION_RESET 0x99

typedef enum{
	SRAM_BUFFER_FAILURE,SRAM_BUFFER_SUCCESS
}SRAM_BUFFER_STATUS;

typedef struct{
	uint16_t idx_front;
	uint16_t idx_rear;
	uint32_t base_sram_addr;
	uint32_t length_byte;
}SramBufferU8;

typedef struct{
	uint16_t idx_front;
	uint16_t idx_rear;
	uint32_t base_sram_addr;
	uint32_t length_byte;
}SramBuffer16;

typedef struct{
	uint16_t idx_front;
	uint16_t idx_rear;
	uint32_t base_sram_addr;
	uint32_t length_byte;
}SramBuffer32;


extern void sramEndCommunication();

extern HAL_StatusTypeDef sramReadModeRegister(uint8_t *dat);
extern HAL_StatusTypeDef sramWriteModeRegister(uint8_t *dat);
extern HAL_StatusTypeDef sramRead(uint32_t addr, uint8_t *read_dat, uint32_t len);
extern HAL_StatusTypeDef sramWrite(uint32_t addr, uint8_t *write_dat, uint32_t len);
extern HAL_StatusTypeDef sramQuadRead(uint32_t addr, uint8_t *read_dat, uint32_t len);
extern HAL_StatusTypeDef sramQuadWrite(uint32_t addr, uint8_t *write_dat, uint32_t len);
extern HAL_StatusTypeDef sramQuadErase(uint32_t addr, uint8_t fill_data, uint32_t len);
extern HAL_StatusTypeDef sramChangeModeToSingle();
extern HAL_StatusTypeDef sramChangeModeToQuad();

extern HAL_StatusTypeDef sramQuadRead_IT(uint32_t addr, uint8_t *read_dat, uint32_t len);
extern HAL_StatusTypeDef sramQuadWrite_IT(uint32_t addr, uint8_t *write_dat, uint32_t len);

extern HAL_StatusTypeDef sramEnableMemoryMapping();

extern bool sramInit();
extern bool sramCheck(uint32_t* error_addr);

extern SRAM_BUFFER_STATUS sramRingBufferU32Init(SramBuffer32 *rbuf, uint32_t base_addr, uint32_t buflen);
extern SRAM_BUFFER_STATUS sramRingBufferU32Enqueue(SramBuffer32 *rbuf, uint32_t *inputc);
extern SRAM_BUFFER_STATUS sramRingBufferU32EnqueueIgnoreErr(SramBuffer32 *rbuf, uint32_t *inputc);
extern SRAM_BUFFER_STATUS sramRingBufferU32EnqueueIgnoreErr_DMA(SramBuffer32 *rbuf, uint32_t *inputc);
extern SRAM_BUFFER_STATUS sramRingBufferU32Dequeue(SramBuffer32 *rbuf, uint32_t *ret);
extern SRAM_BUFFER_STATUS sramRingBufferU32GetElement(SramBuffer32 *rbuf, uint32_t *ret, uint32_t delaynum, uint32_t delay_buffer_length);
extern SRAM_BUFFER_STATUS sramRingBufferU32GetElement_DMA(SramBuffer32 *rbuf, uint32_t *ret, uint32_t delaynum, uint32_t delay_buffer_length);






#endif /* INC_SRAM_MANAGE_H_ */
