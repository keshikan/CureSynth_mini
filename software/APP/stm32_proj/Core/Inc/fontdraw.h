/*
 * fontdraw.h
 *
 *  Created on: 2022/02/09
 *      Author: Keshikan
 */

#ifndef INC_FONTDRAW_H_
#define INC_FONTDRAW_H_


#include <stdint.h>
#include <stdbool.h>
#include "ugui.h"


#define FD_WIDTH_LMT (159)
#define FD_HEIGHT_LMT (84)

#define FD_X_CHARACTER_LIMIT (20)
#define FD_Y_CHARACTER_LIMIT (2)

extern void pset(UG_S16 x, UG_S16 y, UG_COLOR col);
extern void pset_tiny(UG_S16 x, UG_S16 y, UG_COLOR col);

extern void fdSetForegroundColor(UG_COLOR col);
extern void fdSetBackgroundColor(UG_COLOR col);

extern void fdDrawStr(UG_S16 x, UG_S16 y, char* str);
extern void fdDrawStr2(UG_S16 x, UG_S16 y, char* str);

#endif /* INC_FONTDRAW_H_ */
