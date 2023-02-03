/*
 * cureflash.c
 *
 *  Created on: 2022/04/01
 *      Author: Keshikan
 */

#include "cureflash.h"

#define ADDR_SECTOR_START (0x08000000)
#define ADDR_SECTOR_SIZE  (0x00020000)
#define ADDR_SECTOR_END (0x080FFFFF)

uint8_t convertAddressToSector(uint32_t addr)
{
	uint8_t ret;

	if((addr > ADDR_SECTOR_END) || (addr < ADDR_SECTOR_START)){
		return 0xFF;//error magic number
	}

	ret = (addr - ADDR_SECTOR_START) / ADDR_SECTOR_SIZE;

	return ret;

}
