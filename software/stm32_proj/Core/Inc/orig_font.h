/*
 * orig_font.h
 *
 *  Created on: 2022/02/09
 *      Author: Keshikan
 */

#ifndef INC_ORIG_FONT_H_
#define INC_ORIG_FONT_H_

#include <stdint.h>

#define ORIG_FONT_WIDTH (8)
#define ORIG_FONT_HEIGHT (10)

#define ORIG_FONT_OFFSET (0x20)

extern const uint8_t original_font[][10];
extern const uint8_t original_font_narrow[][10];

extern const uint8_t original_font2[][10];

#endif /* INC_ORIG_FONT_H_ */
