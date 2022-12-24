/*
 * dfilter.h
 *
 *  Created on: Jul 27, 2019
 *      Author: Keshikan
 */

#ifndef DFILTER_H_
#define DFILTER_H_


#define DF_FACTOR_A (0.93f)

#include <stdint.h>


extern uint16_t dfRCFilt(uint16_t in, uint16_t prev_out);


#endif /* DFILTER_H_ */
