/*
 * kanjidraw.h
 *
 *  Created on: 2018/09/16
 *      Author: Keshikan
 */

#ifndef KANJIDRAW_H_
#define KANJIDRAW_H_

#include <stdint.h>
#include <stdbool.h>
#include "ugui.h"

#define STR_WIDTH_LMT_1B (250)
#define STR_WIDTH_LMT_2B (244)

extern void kdSetForegroundColor(UG_COLOR col);
extern void kdSetBackgroundColor(UG_COLOR col);

extern void pset(UG_S16 x, UG_S16 y, UG_COLOR col);
extern void kdDrawStr(UG_S16 x, UG_S16 y, char* str);


extern void pset_tiny(UG_S16 x, UG_S16 y, UG_COLOR col);

typedef struct{
	uint16_t code;
	uint16_t width;
	uint16_t height;
	uint16_t size;
}FontInfo;



#endif /* KANJIDRAW_H_ */
