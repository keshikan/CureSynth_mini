/*
 * cureflash.h
 *
 *  Created on: 2022/04/01
 *      Author: Keshikan
 */

#ifndef INC_CUREFLASH_H_
#define INC_CUREFLASH_H_

#include <stm32h7xx_hal.h>
#include <stdint.h>
#include <stdbool.h>


extern uint8_t convertAddressToSector(uint32_t addr);

#endif /* INC_CUREFLASH_H_ */
