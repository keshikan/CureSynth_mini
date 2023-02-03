/*
 * dfilter.c
 *
 *  Created on: Jul 27, 2019
 *      Author: Keshikan
 */


#include "dfilter.h"

uint16_t dfRCFilt(uint16_t in, uint16_t prev_out)
{
	float out;

	out = (DF_FACTOR_A * prev_out) + (1.0f-DF_FACTOR_A) * in;

	return (uint16_t)out;

}

