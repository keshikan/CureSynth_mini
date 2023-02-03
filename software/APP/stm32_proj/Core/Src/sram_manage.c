/*
 * sram_manage.c
 *
 *  Created on: 2022/08/24
 *      Author: Keshikan
 */

/*
 * see http://meerstern.seesaa.net/article/479545212.html
 * see AN5050
 * */

/*
 * for APS1604M_SQ
 * */

#include "sram_manage.h"


extern OSPI_HandleTypeDef OSPI_TYPEDEF;
extern DMA_HandleTypeDef DMA_MEMTOMEM_TYPEDEF_DEQUEUE;
extern DMA_HandleTypeDef DMA_MEMTOMEM_TYPEDEF_ENQUEUE;

static volatile bool isCommunicateReady = true;

//ram positioning
#define __ATTR_DTCMRAM   __attribute__ ((section(".DTCMRAM"))) __attribute__ ((aligned (4)))
#define __ATTR_ITCMRAM   __attribute__ ((section(".ITCMRAM"))) __attribute__ ((aligned (8)))
#define __ATTR_RAM_D2	__attribute__ ((section(".RAM_D2"))) __attribute__ ((aligned (4)))
#define __ATTR_RAM_D3	__attribute__ ((section(".RAM_D3"))) __attribute__ ((aligned (4)))
#define __ATTR_QSPI	   __attribute__ ((section(".QSPI_PSRAM")))


void sramEndCommunication()
{
	isCommunicateReady = true;
}

void sramWaitCommunication()
{
	while(1)
	{
		if(isCommunicateReady){
			break;
		}
	}
}


HAL_StatusTypeDef sramReset()
{
	OSPI_RegularCmdTypeDef com;
	HAL_StatusTypeDef ret = HAL_OK;

	com.OperationType = HAL_OSPI_OPTYPE_COMMON_CFG;
	com.FlashId = HAL_OSPI_FLASH_ID_1;
	com.Instruction = SRAM_INSTRUCTION_RESET_ENABLE;
	com.InstructionMode = HAL_OSPI_INSTRUCTION_1_LINE;
	com.InstructionSize = HAL_OSPI_INSTRUCTION_8_BITS;
	com.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
	com.Address = 0;
	com.AddressMode = HAL_OSPI_ADDRESS_NONE;
	com.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
	com.DataMode = HAL_OSPI_DATA_NONE;
	com.NbData = 0;
	com.DataDtrMode = HAL_OSPI_DATA_DTR_DISABLE;
	com.DummyCycles = 0;
	com.DQSMode = HAL_OSPI_DQS_DISABLE;
	com.SIOOMode = HAL_OSPI_SIOO_INST_EVERY_CMD;

	ret += HAL_OSPI_Command(&hospi1, &com, 0xFFF);

	HAL_Delay(5);

	com.Instruction = SRAM_INSTRUCTION_RESET;

	ret += HAL_OSPI_Command(&hospi1, &com, 0xFFF);

	return ret;

}


HAL_StatusTypeDef sramReadModeRegister(uint8_t *dat)
{
	OSPI_RegularCmdTypeDef com;
	HAL_StatusTypeDef ret = HAL_OK;

	com.OperationType = HAL_OSPI_OPTYPE_COMMON_CFG;
	com.FlashId = HAL_OSPI_FLASH_ID_1;
	com.Instruction = SRAM_INSTRUCTION_RDMR;
	com.InstructionMode = HAL_OSPI_INSTRUCTION_1_LINE;
	com.InstructionSize = HAL_OSPI_INSTRUCTION_8_BITS;
	com.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
	com.Address = 0x000000;
	com.AddressMode = HAL_OSPI_ADDRESS_1_LINE;
	com.AddressSize = HAL_OSPI_ADDRESS_24_BITS;
	com.AddressDtrMode = HAL_OSPI_ADDRESS_DTR_DISABLE;
	com.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
	com.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
	com.DataMode = HAL_OSPI_DATA_1_LINE;
	com.NbData = 1;
	com.DataDtrMode = HAL_OSPI_DATA_DTR_DISABLE;
	com.DummyCycles = 8;
	com.DQSMode = HAL_OSPI_DQS_DISABLE;
	com.SIOOMode = HAL_OSPI_SIOO_INST_EVERY_CMD;

	ret += HAL_OSPI_Command(&hospi1, &com, 0xFFF);
	ret += HAL_OSPI_Receive(&hospi1, dat, 0xFFF);
	dat[0] = dat[0] >> 6;

	return ret;

}

HAL_StatusTypeDef sramWriteModeRegister(uint8_t *dat)
{
	OSPI_RegularCmdTypeDef com;
	HAL_StatusTypeDef ret = HAL_OK;

	com.OperationType = HAL_OSPI_OPTYPE_COMMON_CFG;
	com.FlashId = HAL_OSPI_FLASH_ID_1;
	com.Instruction = SRAM_INSTRUCTION_WRMR;
	com.InstructionMode = HAL_OSPI_INSTRUCTION_1_LINE;
	com.InstructionSize = HAL_OSPI_INSTRUCTION_8_BITS;
	com.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
	com.Address = 0x000000;
	com.AddressMode = HAL_OSPI_ADDRESS_1_LINE;
	com.AddressSize = HAL_OSPI_ADDRESS_24_BITS;
	com.AddressDtrMode = HAL_OSPI_ADDRESS_DTR_DISABLE;
	com.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
	com.DataMode = HAL_OSPI_DATA_1_LINE;
	com.NbData = 1;
	com.DataDtrMode = HAL_OSPI_DATA_DTR_DISABLE;
	com.DummyCycles = 0;
	com.DQSMode = HAL_OSPI_DQS_DISABLE;
	com.SIOOMode = HAL_OSPI_SIOO_INST_EVERY_CMD;

	ret += HAL_OSPI_Command(&hospi1, &com, 0xFFF);
	ret += HAL_OSPI_Transmit(&hospi1, dat, 0xFFF);

	return ret;

}


HAL_StatusTypeDef sramRead(uint32_t addr, uint8_t *read_dat, uint32_t len)
{
	OSPI_RegularCmdTypeDef com;
	HAL_StatusTypeDef ret = HAL_OK;

	com.OperationType = HAL_OSPI_OPTYPE_COMMON_CFG;
	com.FlashId = HAL_OSPI_FLASH_ID_1;
	com.Instruction = SRAM_INSTRUCTION_READ;
	com.InstructionMode = HAL_OSPI_INSTRUCTION_1_LINE;
	com.InstructionSize = HAL_OSPI_INSTRUCTION_8_BITS;
	com.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
	com.Address = addr;
	com.AddressMode = HAL_OSPI_ADDRESS_1_LINE;
	com.AddressSize = HAL_OSPI_ADDRESS_24_BITS;
	com.AddressDtrMode = HAL_OSPI_ADDRESS_DTR_DISABLE;
	com.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
	com.DataMode = HAL_OSPI_DATA_1_LINE;
	com.NbData = len;
	com.DataDtrMode = HAL_OSPI_DATA_DTR_DISABLE;
	com.DummyCycles = 0;
	com.DQSMode = HAL_OSPI_DQS_DISABLE;
	com.SIOOMode = HAL_OSPI_SIOO_INST_EVERY_CMD;

	ret += HAL_OSPI_Command(&hospi1, &com, 0xFFF);
	ret += HAL_OSPI_Receive(&hospi1, read_dat, 0xFFF);

	return ret;

}

HAL_StatusTypeDef sramWrite(uint32_t addr, uint8_t *write_dat, uint32_t len)
{
	OSPI_RegularCmdTypeDef com;
	HAL_StatusTypeDef ret = HAL_OK;

	com.OperationType = HAL_OSPI_OPTYPE_COMMON_CFG;
	com.FlashId = HAL_OSPI_FLASH_ID_1;
	com.Instruction = SRAM_INSTRUCTION_WRITE;
	com.InstructionMode = HAL_OSPI_INSTRUCTION_1_LINE;
	com.InstructionSize = HAL_OSPI_INSTRUCTION_8_BITS;
	com.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
	com.Address = addr;
	com.AddressMode = HAL_OSPI_ADDRESS_1_LINE;
	com.AddressSize = HAL_OSPI_ADDRESS_24_BITS;
	com.AddressDtrMode = HAL_OSPI_ADDRESS_DTR_DISABLE;
	com.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
	com.DataMode = HAL_OSPI_DATA_1_LINE;
	com.NbData = len;
	com.DataDtrMode = HAL_OSPI_DATA_DTR_DISABLE;
	com.DummyCycles = 0;
	com.DQSMode = HAL_OSPI_DQS_DISABLE;
	com.SIOOMode = HAL_OSPI_SIOO_INST_EVERY_CMD;

	ret += HAL_OSPI_Command(&hospi1, &com, 0xFFF);
	ret += HAL_OSPI_Transmit(&hospi1, write_dat, 0xFFF);

	return ret;

}

HAL_StatusTypeDef sramQuadRead(uint32_t addr, uint8_t *read_dat, uint32_t len)
{

#ifdef SRAM_MEMORY_MAPPED_MODE_ON

	__IO uint8_t *mem_addr = (__IO uint8_t *)(OCTOSPI1_BASE + addr);

	for(uint32_t i=0; i<len; i++){
		read_dat[i] = *mem_addr;
		mem_addr++;
	}
	return HAL_OK;

#else

	OSPI_RegularCmdTypeDef com;
	HAL_StatusTypeDef ret = HAL_OK;

	com.OperationType = HAL_OSPI_OPTYPE_COMMON_CFG;
	com.FlashId = HAL_OSPI_FLASH_ID_1;
	com.Instruction = SRAM_INSTRUCTION_FAST_READ_QUAD;
	com.InstructionMode = HAL_OSPI_INSTRUCTION_4_LINES;
	com.InstructionSize = HAL_OSPI_INSTRUCTION_8_BITS;
	com.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
	com.Address = addr;
	com.AddressMode = HAL_OSPI_ADDRESS_4_LINES;
	com.AddressSize = HAL_OSPI_ADDRESS_24_BITS;
	com.AddressDtrMode = HAL_OSPI_ADDRESS_DTR_DISABLE;
	com.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
	com.DataMode = HAL_OSPI_DATA_4_LINES;
	com.NbData = len;
	com.DataDtrMode = HAL_OSPI_DATA_DTR_DISABLE;
	com.DummyCycles = 6;
	com.DQSMode = HAL_OSPI_DQS_DISABLE;
	com.SIOOMode = HAL_OSPI_SIOO_INST_EVERY_CMD;

	ret += HAL_OSPI_Command(&hospi1, &com, 0xFFF);
	ret += HAL_OSPI_Receive(&hospi1, read_dat, 0xFFF);

	return ret;
#endif

}

HAL_StatusTypeDef sramQuadWrite(uint32_t addr, uint8_t *write_dat, uint32_t len)
{

#ifdef SRAM_MEMORY_MAPPED_MODE_ON

	__IO uint8_t *mem_addr = (__IO uint8_t *)(OCTOSPI1_BASE + addr);

	for(uint32_t i=0; i<len; i++){
		*mem_addr = write_dat[i];
		mem_addr++;
	}
	return HAL_OK;

#else
	OSPI_RegularCmdTypeDef com;
	HAL_StatusTypeDef ret = HAL_OK;

	com.OperationType = HAL_OSPI_OPTYPE_COMMON_CFG;
	com.FlashId = HAL_OSPI_FLASH_ID_1;
	com.Instruction = SRAM_INSTRUCTION_WRITE_QUAD;
	com.InstructionMode = HAL_OSPI_INSTRUCTION_4_LINES;
	com.InstructionSize = HAL_OSPI_INSTRUCTION_8_BITS;
	com.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
	com.Address = addr;
	com.AddressMode = HAL_OSPI_ADDRESS_4_LINES;
	com.AddressSize = HAL_OSPI_ADDRESS_24_BITS;
	com.AddressDtrMode = HAL_OSPI_ADDRESS_DTR_DISABLE;
	com.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
	com.DataMode = HAL_OSPI_DATA_4_LINES;
	com.NbData = len;
	com.DataDtrMode = HAL_OSPI_DATA_DTR_DISABLE;
	com.DummyCycles = 0;
	com.DQSMode = HAL_OSPI_DQS_DISABLE;
	com.SIOOMode = HAL_OSPI_SIOO_INST_EVERY_CMD;

	ret += HAL_OSPI_Command(&hospi1, &com, 0xFFF);
	ret += HAL_OSPI_Transmit(&hospi1, write_dat, 0xFFF);

	return ret;
#endif

}

HAL_StatusTypeDef sramQuadRead_IT(uint32_t addr, uint8_t *read_dat, uint32_t len)
{
	OSPI_RegularCmdTypeDef com;
	HAL_StatusTypeDef ret = HAL_OK;

	com.OperationType = HAL_OSPI_OPTYPE_COMMON_CFG;
	com.FlashId = HAL_OSPI_FLASH_ID_1;
	com.Instruction = SRAM_INSTRUCTION_FAST_READ_QUAD;
	com.InstructionMode = HAL_OSPI_INSTRUCTION_4_LINES;
	com.InstructionSize = HAL_OSPI_INSTRUCTION_8_BITS;
	com.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
	com.Address = addr;
	com.AddressMode = HAL_OSPI_ADDRESS_4_LINES;
	com.AddressSize = HAL_OSPI_ADDRESS_24_BITS;
	com.AddressDtrMode = HAL_OSPI_ADDRESS_DTR_DISABLE;
	com.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
	com.DataMode = HAL_OSPI_DATA_4_LINES;
	com.NbData = len;
	com.DataDtrMode = HAL_OSPI_DATA_DTR_DISABLE;
	com.DummyCycles = 6;
	com.DQSMode = HAL_OSPI_DQS_DISABLE;
	com.SIOOMode = HAL_OSPI_SIOO_INST_EVERY_CMD;


	sramWaitCommunication();
	isCommunicateReady = false;

	ret += HAL_OSPI_Command(&hospi1, &com, 0xFFF);
	ret += HAL_OSPI_Receive_IT(&hospi1, read_dat);

	return ret;

}

HAL_StatusTypeDef sramQuadWrite_IT(uint32_t addr, uint8_t *write_dat, uint32_t len)
{
	OSPI_RegularCmdTypeDef com;
	HAL_StatusTypeDef ret = HAL_OK;

	com.OperationType = HAL_OSPI_OPTYPE_COMMON_CFG;
	com.FlashId = HAL_OSPI_FLASH_ID_1;
	com.Instruction = SRAM_INSTRUCTION_WRITE;
	com.InstructionMode = HAL_OSPI_INSTRUCTION_4_LINES;
	com.InstructionSize = HAL_OSPI_INSTRUCTION_8_BITS;
	com.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
	com.Address = addr;
	com.AddressMode = HAL_OSPI_ADDRESS_4_LINES;
	com.AddressSize = HAL_OSPI_ADDRESS_24_BITS;
	com.AddressDtrMode = HAL_OSPI_ADDRESS_DTR_DISABLE;
	com.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
	com.DataMode = HAL_OSPI_DATA_4_LINES;
	com.NbData = len / 2;//dma half-word
	com.DataDtrMode = HAL_OSPI_DATA_DTR_DISABLE;
	com.DummyCycles = 0;
	com.DQSMode = HAL_OSPI_DQS_DISABLE;
	com.SIOOMode = HAL_OSPI_SIOO_INST_EVERY_CMD;

//	sramWaitCommunication();
//	isCommunicateReady = false;

	ret += HAL_OSPI_Command(&hospi1, &com, 0xFFF);
	ret += HAL_OSPI_Transmit_DMA(&hospi1, write_dat);

	return ret;

}

HAL_StatusTypeDef sramQuadErase(uint32_t addr, uint8_t fill_data, uint32_t len)
{
	HAL_StatusTypeDef ret = HAL_OK;
	for(uint32_t i=0; i<len; i++){
		ret = sramQuadWrite(addr + i, (uint8_t *)(&fill_data), 1);
		if(HAL_OK != ret ){
			return ret;
		}
	}

	return ret;

}



HAL_StatusTypeDef sramChangeModeToSingle()
{
	OSPI_RegularCmdTypeDef com;
	HAL_StatusTypeDef ret;

	com.OperationType = HAL_OSPI_OPTYPE_COMMON_CFG;
	com.FlashId = HAL_OSPI_FLASH_ID_1;
	com.Instruction = SRAM_INSTRUCTION_RSTIO;
	com.InstructionMode = HAL_OSPI_INSTRUCTION_4_LINES;
	com.InstructionSize = HAL_OSPI_INSTRUCTION_8_BITS;
	com.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
	com.Address = 0;
	com.AddressMode = HAL_OSPI_ADDRESS_NONE;
	com.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
	com.DataMode = HAL_OSPI_DATA_NONE;
	com.NbData = 0;
	com.DataDtrMode = HAL_OSPI_DATA_DTR_DISABLE;
	com.DummyCycles = 0;
	com.DQSMode = HAL_OSPI_DQS_DISABLE;
	com.SIOOMode = HAL_OSPI_SIOO_INST_EVERY_CMD;

	ret = HAL_OSPI_Command(&hospi1, &com, 0xFFF);

	return ret;

}

HAL_StatusTypeDef sramChangeModeToQuad()
{
	OSPI_RegularCmdTypeDef com;
	HAL_StatusTypeDef ret;

	com.OperationType = HAL_OSPI_OPTYPE_COMMON_CFG;
	com.FlashId = HAL_OSPI_FLASH_ID_1;
	com.Instruction = SRAM_INSTRUCTION_EQIO;
	com.InstructionMode = HAL_OSPI_INSTRUCTION_1_LINE;
	com.InstructionSize = HAL_OSPI_INSTRUCTION_8_BITS;
	com.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
	com.Address = 0;
	com.AddressMode = HAL_OSPI_ADDRESS_NONE;
	com.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
	com.DataMode = HAL_OSPI_DATA_NONE;
	com.NbData = 0;
	com.DataDtrMode = HAL_OSPI_DATA_DTR_DISABLE;
	com.DummyCycles = 0;
	com.DQSMode = HAL_OSPI_DQS_DISABLE;
	com.SIOOMode = HAL_OSPI_SIOO_INST_EVERY_CMD;

	ret = HAL_OSPI_Command(&hospi1, &com, 0xFFF);

	return ret;

}

//cf.AN5050 "OCTOSPI application examples"
HAL_StatusTypeDef sramEnableMemoryMapping()
{

	OSPI_RegularCmdTypeDef com;
	OSPI_MemoryMappedTypeDef sMemMappedCfg;
	HAL_StatusTypeDef ret = HAL_OK;

	com.OperationType = HAL_OSPI_OPTYPE_READ_CFG;
	com.FlashId = HAL_OSPI_FLASH_ID_1;
	com.Instruction = SRAM_INSTRUCTION_FAST_READ_QUAD;
	com.InstructionMode = HAL_OSPI_INSTRUCTION_4_LINES;
	com.InstructionSize = HAL_OSPI_INSTRUCTION_8_BITS;
	com.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
	com.Address = 0;
	com.AddressMode = HAL_OSPI_ADDRESS_4_LINES;
	com.AddressSize = HAL_OSPI_ADDRESS_24_BITS;
	com.AddressDtrMode = HAL_OSPI_ADDRESS_DTR_DISABLE;
	com.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
	com.DataMode = HAL_OSPI_DATA_4_LINES;
	com.NbData = 1;
	com.DataDtrMode = HAL_OSPI_DATA_DTR_DISABLE;
	com.DummyCycles = 6;
	com.DQSMode = HAL_OSPI_DQS_DISABLE;
	com.SIOOMode = HAL_OSPI_SIOO_INST_EVERY_CMD;

	ret = HAL_OSPI_Command(&hospi1, &com, 0xFFF);
	if(HAL_OK != ret){
		return ret;
	}

	com.OperationType = HAL_OSPI_OPTYPE_WRITE_CFG;
	com.Instruction = SRAM_INSTRUCTION_WRITE_QUAD;
	com.DummyCycles = 0;
	com.DQSMode = HAL_OSPI_DQS_ENABLE;

	ret = HAL_OSPI_Command(&hospi1, &com, 0xFFF);
	if(HAL_OK != ret){
		return ret;
	}

	sMemMappedCfg.TimeOutActivation = HAL_OSPI_TIMEOUT_COUNTER_DISABLE;

	ret = HAL_OSPI_MemoryMapped(&hospi1, &sMemMappedCfg);
	if(HAL_OK != ret){
		return ret;
	}

	return ret;

}



//sram init and check
//return true: sram is ok
bool sramInit()
{
	volatile uint8_t mode_register = 0b01110001;
	volatile uint8_t mode_register_read;

	if( HAL_OK != sramChangeModeToSingle() ){
		return false;
	}

	if( HAL_OK != sramReset() ){
		return false;
	}

	sramReadModeRegister((uint8_t *)&mode_register_read);

	if( HAL_OK != sramWriteModeRegister((uint8_t *)&mode_register) ){
		return false;
	}

	sramReadModeRegister((uint8_t *)&mode_register_read);

	if( HAL_OK != sramChangeModeToQuad() ){
		return false;
	}


#ifdef SRAM_MEMORY_MAPPED_MODE_ON
	//memory mapped mode test
	__IO uint8_t *mem_addr = (__IO uint8_t *)OCTOSPI1_BASE;

	if( HAL_OK != sramEnableMemoryMapping() ){
		return false;
	}

	for(uint32_t i=0; i<100; i++){
		*mem_addr = i;
		mem_addr++;
	}

	volatile uint8_t read_dat;

	mem_addr = (__IO uint8_t *)OCTOSPI1_BASE;
	for(uint32_t i=0; i<100; i++){

		read_dat = *mem_addr;
		if((uint8_t)i != read_dat){
			return false;
		}
		mem_addr++;
	}

#else

	uint8_t wData[0x100];
	uint8_t rData[0x100];
	for(uint32_t i =0;i<0x100;i ++)
	{
		wData[i] = i;
		rData[i] = 0;
	}

	if( HAL_OK != sramQuadWrite(0x00, wData, 0x100) ){
		return false;
	}

	if( HAL_OK != sramQuadRead(0x00, rData, 0x100) ){
		return false;
	}


	for(uint32_t i =0;i<0x100;i++)
	{
		if(rData[i] != wData[i])
		{
			return false;
		}
	}

#endif

	return true;
}

#define SRAM_CHECK_SIZE (0x0200000)

bool sramCheck(uint32_t* error_addr)
{

	__IO uint8_t* mem_addr = (__IO uint8_t*)OCTOSPI1_BASE;

	for(uint32_t i=0; i<SRAM_CHECK_SIZE; i++){
		*mem_addr = (i & 0xFF);
		mem_addr++;
	}

	volatile uint8_t read_dat;

	mem_addr = (__IO uint8_t *)OCTOSPI1_BASE;
	for(uint32_t i=0; i<SRAM_CHECK_SIZE; i++){

		read_dat = *mem_addr;
		if((i & 0xFF) != read_dat){
			*error_addr = i;
			return false;
		}
		mem_addr++;
	}

	return true;

}



/////////////////////////////
//methods for uint32_t FIFO.
/////////////////////////////


SRAM_BUFFER_STATUS sramRingBufferU32Init(SramBuffer32 *rbuf, uint32_t base_addr, uint32_t buflen)
{

	//check memory length
	if((base_addr + buflen) > SRAM_MEMORY_SIZE){
		return SRAM_BUFFER_FAILURE;
	}

	//init
	if(HAL_OK != sramQuadErase(base_addr, 0x00, buflen * 4)){
		return SRAM_BUFFER_FAILURE;
	}

	rbuf->base_sram_addr = base_addr;
	rbuf->length_byte = buflen;
	rbuf->idx_front = rbuf->idx_rear =0;

	return SRAM_BUFFER_SUCCESS;
}

SRAM_BUFFER_STATUS sramRingBufferU32Enqueue(SramBuffer32 *rbuf, uint32_t *inputc)
{
	uint32_t wdat = *inputc;
	SRAM_BUFFER_STATUS status = SRAM_BUFFER_SUCCESS;

	if( ((rbuf->idx_front +1)&(rbuf->length_byte -1)) == rbuf->idx_rear ){//buffer overrun error occurs.
		status = SRAM_BUFFER_FAILURE;
	}else{
		if( HAL_OK != sramQuadWrite(rbuf->base_sram_addr + (rbuf->idx_front * 4), (uint8_t *)(&wdat), 4)){
			status = SRAM_BUFFER_FAILURE;
		}
		rbuf->idx_front++;
		rbuf->idx_front &= (rbuf->length_byte -1);
	}

	return status;
}


SRAM_BUFFER_STATUS sramRingBufferU32EnqueueIgnoreErr(SramBuffer32 *rbuf, uint32_t *inputc)
{
	uint32_t wdat = *inputc;
	SRAM_BUFFER_STATUS status = SRAM_BUFFER_SUCCESS;

	if( HAL_OK != sramQuadWrite(rbuf->base_sram_addr + (rbuf->idx_front * 4), (uint8_t *)(&wdat), 4)){
		status = SRAM_BUFFER_FAILURE;
	}
	rbuf->idx_front++;
	rbuf->idx_front &= (rbuf->length_byte -1);
	return status;

}

SRAM_BUFFER_STATUS sramRingBufferU32EnqueueIgnoreErr_DMA(SramBuffer32 *rbuf, uint32_t *inputc)
{
	SRAM_BUFFER_STATUS status = SRAM_BUFFER_SUCCESS;
	uint32_t dest_addr = OCTOSPI1_BASE + rbuf->base_sram_addr + (rbuf->idx_front * 4);

	HAL_DMA_Abort(&DMA_MEMTOMEM_TYPEDEF_ENQUEUE);
	if(HAL_OK != HAL_DMA_Start(&DMA_MEMTOMEM_TYPEDEF_ENQUEUE, (uint32_t)inputc, dest_addr, 4)){
		status = SRAM_BUFFER_FAILURE;
	}


	rbuf->idx_front++;
	rbuf->idx_front &= (rbuf->length_byte -1);
	return status;

}



SRAM_BUFFER_STATUS sramRingBufferU32Dequeue(SramBuffer32 *rbuf, uint32_t *ret)
{
	SRAM_BUFFER_STATUS status = SRAM_BUFFER_SUCCESS;

	if(rbuf->idx_front == rbuf->idx_rear){//if buffer underrun error occurs.
		status = SRAM_BUFFER_FAILURE;
	}else{
		sramQuadRead(rbuf->base_sram_addr + (rbuf->idx_rear * 4), (uint8_t *)ret, 4);
		rbuf->idx_rear++;
		rbuf->idx_rear &= (rbuf->length_byte -1);
	}

	return status;
}


SRAM_BUFFER_STATUS sramRingBufferU32GetElement(SramBuffer32 *rbuf, uint32_t *ret, uint32_t delaynum, uint32_t delay_buffer_length)
{
	uint32_t read_position;


	if(rbuf->idx_front >= delaynum){
		read_position = rbuf->idx_front - delaynum;
	}else{
		read_position = delay_buffer_length - (delaynum - rbuf->idx_front);
	}

	read_position = rbuf->base_sram_addr + (read_position * 4);

	sramQuadRead(read_position, (uint8_t *)ret, 4);

	return SRAM_BUFFER_SUCCESS;

}

uint32_t readpos_debug __ATTR_RAM_D2;

SRAM_BUFFER_STATUS sramRingBufferU32GetElement_DMA(SramBuffer32 *rbuf, uint32_t *ret, uint32_t delaynum, uint32_t delay_buffer_length)
{
	uint32_t read_position;


	if(rbuf->idx_front >= delaynum){
		read_position = rbuf->idx_front - delaynum;
	}else{
		read_position = delay_buffer_length - (delaynum - rbuf->idx_front);
	}

	read_position = OCTOSPI1_BASE + rbuf->base_sram_addr + (read_position * 4);

	HAL_DMA_Abort(&DMA_MEMTOMEM_TYPEDEF_DEQUEUE);
	HAL_DMA_Start(&DMA_MEMTOMEM_TYPEDEF_DEQUEUE, read_position, (uint32_t)ret, 4);

	return SRAM_BUFFER_SUCCESS;

}


