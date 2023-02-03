/*
 * img.h
 *
 *  Created on: Apr 5, 2022
 *      Author: Keshikan
 */

#ifndef INC_IMG_H_
#define INC_IMG_H_

#include <stdint.h>

//color565
#define IMG_NYAN_WIDTH (48)
#define IMG_NYAN_HEIGHT (23)
#define IMG_TAIL_WIDTH (30)
#define IMG_TAIL_HEIGHT (22)

//grayscale 0-f
#define IMG_LAYOUT_WIDTH (160)
#define IMG_LAYOUT_HEIGHT (60)
#define IMG_BUBBLE_WIDTH (23)
#define IMG_BUBBLE_HEIGHT (21)


extern const uint16_t nyan_1[IMG_NYAN_HEIGHT][IMG_NYAN_WIDTH];
extern const uint16_t nyan_2[IMG_NYAN_HEIGHT][IMG_NYAN_WIDTH];
extern const uint16_t tail_1[IMG_TAIL_HEIGHT][IMG_TAIL_WIDTH];
extern const uint16_t tail_2[IMG_TAIL_HEIGHT][IMG_TAIL_WIDTH];
extern const uint8_t layout_1[IMG_LAYOUT_HEIGHT][IMG_LAYOUT_WIDTH];
extern const uint8_t layout_2[IMG_LAYOUT_HEIGHT][IMG_LAYOUT_WIDTH];
extern const uint8_t bubble_1[IMG_BUBBLE_HEIGHT][IMG_BUBBLE_WIDTH];
extern const uint8_t bubble_2[IMG_BUBBLE_HEIGHT][IMG_BUBBLE_WIDTH];

#endif /* INC_IMG_H_ */
