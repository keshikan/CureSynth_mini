/*
 * sjis_font.h
 *
 *  Created on: 2018/09/16
 *      Author: Keshikan
 */

#ifndef SJIS_FONT_H_
#define SJIS_FONT_H_

#include <stdint.h>


//config usage
#define USE_1B_FONT
#define USE_2B_FONT

//override font name
#define sjis_1b_font MPLHN10_FNT
#define sjis_2b_font MPLZN10X_fixed_FNT

//__04GZN16X_FNT, SHNHN12_FNT, SHGZN12X_FNT


//FONTX2 format

#ifdef USE_1B_FONT
extern const uint8_t SHNHN12_FNT[];
extern const uint32_t SHNHN12_FNT_len;
extern const uint8_t SHNHN16_FNT[];
extern const uint32_t SHNHN16_FNT_len;
extern const uint8_t MPLHN10_FNT[];
extern const uint32_t MPLHN10_FNT_len;
#endif

#ifdef USE_2B_FONT
extern const uint8_t SHGZN12X_FNT[];
extern const uint32_t SHGZN12X_FNT_len;
extern const uint8_t __04GZN16X_FNT[];
extern const uint32_t __04GZN16X_FNT_len;
extern const uint8_t K12_2000_FNT[];
extern const uint32_t K12_2000_FNT_len;
extern const uint8_t MPLZN10X_fixed_FNT[];
extern const uint32_t MPLZN10X_fixed_FNT_len;
#endif

#endif /* SJIS_FONT_H_ */
