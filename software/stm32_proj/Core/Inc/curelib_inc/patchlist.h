/*
 *******************************************************************************
 *  [patchlist.h]
 *  This header file contains patch-lists for GM1 mode.
 *
 *  This program is under the terms of the GPLv3.
 *  https://www.gnu.org/licenses/gpl-3.0.html
 *
 *  Copyright(c) 2017 Keshikan (www.keshikan.net)
 *******************************************************************************
 */

#ifndef CURELIB_INC_PATCHLIST_H_
#define CURELIB_INC_PATCHLIST_H_

#include "curelib_inc/curemidi.h"

//GM1 Patch for melody instruments.
const MidiPatch patch_melody[128] = {
		// note type  wave wavenum  rm  rg  Sweeptyp  spd A  D   SL  SR  R  Nt  Gain
		{NOTE_MELODY, SINE, 0, 31, 70, SWEEP_NONE, 0, 0, 5, 90, 12, 0, 0, 48},
		{NOTE_MELODY, TRIANGLE, 0, 31, 100, SWEEP_NONE, 0, 0, 5, 90, 12, 0, 0, 100},
		{NOTE_MELODY, TRIANGLE, 0, 31, 110, SWEEP_NONE, 0, 0, 5, 90, 12, 0, 0, 110},
		{NOTE_MELODY, SINE, 0, 31, 100, SWEEP_NONE, 0, 0, 15, 90, 12, 0, 0, 50},
		{NOTE_MELODY, SINE, 0, 95, 80, SWEEP_NONE, 0, 0, 5, 90, 12, 0, 0, 42},
		{NOTE_MELODY, SINE, 0, 127, 100, SWEEP_NONE, 0, 0, 5, 90, 12, 0, 0, 46},
		{NOTE_MELODY, SAW, 0, 63, 35, SWEEP_NONE, 0, 0, 10, 50, 9, 0, 0, 42},
		{NOTE_MELODY, SAW, 0, 63, 127, SWEEP_NONE, 0, 0, 10, 50, 23, 0, 0, 60},
		{NOTE_MELODY, TRIANGLE, 0, 127, 90, SWEEP_NONE, 0, 0, 10, 50, 23, 0, 0, 55},
		{NOTE_MELODY, SINE, 0, 115, 80, SWEEP_NONE, 0, 0, 10, 50, 15, 0, 0, 44},
		{NOTE_DRUM, SINE, 0, 95, 40, SWEEP_NONE, 0, 0, 15, 40, 15, 0, 0, 43},
		{NOTE_MELODY, SINE, 0, 95, 80, SWEEP_NONE, 0, 0, 10, 80, 12, 50, 0, 42},
		{NOTE_MELODY, TRIANGLE, 0, 127, 80, SWEEP_NONE, 0, 0, 10, 50, 45, 15, 0, 55},
		{NOTE_DRUM, TRIANGLE, 0, 63, 80, SWEEP_NONE, 0, 0, 5, 50, 80, 0, 0, 70},
		{NOTE_DRUM, TRIANGLE, 0, 79, 45, SWEEP_NONE, 0, 0, 5, 50, 10, 0, 0, 45},
		{NOTE_MELODY, TRIANGLE, 0, 79, 70, SWEEP_NONE, 0, 0, 5, 50, 30, 10, 0, 45},
		{NOTE_MELODY, SINE, 0, 95, 64, SWEEP_NONE, 0, 1, 2, 100, 0, 0, 0, 44},
		{NOTE_MELODY, SINE, 0, 95, 40, SWEEP_NONE, 0, 1, 20, 40, 0, 0, 0, 43},
		{NOTE_MELODY, SINE, 0, 47, 80, SWEEP_NONE, 0, 1, 20, 40, 0, 0, 0, 46},
		{NOTE_MELODY, SQUARE, 0, 95, 127, SWEEP_NONE, 0, 3, 20, 80, 0, 6, 0, 35},
		{NOTE_MELODY, SAW, 0, 95, 127, SWEEP_NONE, 0, 5, 20, 80, 0, 6, 0, 57},
		{NOTE_MELODY, SAW, 0, 63, 127, SWEEP_NONE, 0, 5, 20, 80, 0, 6, 0, 69},
		{NOTE_MELODY, SAW, 0, 63, 40, SWEEP_NONE, 0, 8, 20, 80, 0, 3, 0, 44},
		{NOTE_MELODY, SAW, 0, 63, 40, SWEEP_NONE, 0, 8, 20, 80, 0, 3, 0, 44},
		{NOTE_MELODY, TRIANGLE, 0, 63, 100, SWEEP_NONE, 0, 0, 3, 80, 10, 3, 0, 70},
		{NOTE_MELODY, TRIANGLE, 0, 63, 65, SWEEP_NONE, 0, 0, 3, 80, 10, 3, 0, 67},
		{NOTE_MELODY, TRIANGLE, 0, 95, 100, SWEEP_NONE, 0, 0, 3, 80, 10, 3, 0, 62},
		{NOTE_MELODY, TRIANGLE, 0, 95, 65, SWEEP_NONE, 0, 0, 3, 80, 10, 3, 0, 60},
		{NOTE_MELODY, TRIANGLE, 0, 95, 65, SWEEP_NONE, 0, 0, 3, 80, 52, 3, 0, 59},
		{NOTE_MELODY, SQUARE, 0, 95, 100, SWEEP_NONE, 0, 0, 3, 80, 0, 3, 0, 41},
		{NOTE_MELODY, SQUARE, 0, 79, 80, SWEEP_NONE, 0, 0, 3, 80, 0, 3, 0, 33},
		{NOTE_MELODY, SQUARE, 0, 31, 120, SWEEP_NONE, 0, 0, 3, 80, 5, 3, 0, 52},
		{NOTE_MELODY, TRIANGLE, 0, 63, 120, SWEEP_NONE, 0, 0, 2, 80, 4, 3, 0, 72},
		{NOTE_MELODY, TRIANGLE, 0, 63, 40, SWEEP_NONE, 0, 0, 2, 80, 4, 3, 0, 62},
		{NOTE_MELODY, SINE, 0, 63, 40, SWEEP_NONE, 0, 0, 2, 80, 4, 3, 0, 52},
		{NOTE_MELODY, SAW, 0, 31, 120, SWEEP_NONE, 0, 0, 2, 80, 4, 3, 0, 93},
		{NOTE_MELODY, SAW, 0, 63, 90, SWEEP_NONE, 0, 0, 5, 60, 4, 3, 0, 62},
		{NOTE_MELODY, SAW, 0, 63, 110, SWEEP_NONE, 0, 0, 5, 60, 4, 3, 0, 65},
		{NOTE_MELODY, SQUARE, 0, 63, 20, SWEEP_NONE, 0, 0, 5, 60, 4, 3, 0, 34},
		{NOTE_MELODY, SQUARE, 0, 63, 70, SWEEP_NONE, 0, 0, 5, 60, 4, 3, 0, 43},
		{NOTE_MELODY, SAW, 0, 63, 70, SWEEP_NONE, 0, 8, 5, 60, 0, 6, 0, 62},
		{NOTE_MELODY, SAW, 0, 63, 110, SWEEP_NONE, 0, 8, 5, 60, 0, 6, 0, 113},
		{NOTE_MELODY, SAW, 0, 63, 120, SWEEP_NONE, 0, 8, 5, 60, 0, 6, 0, 78},
		{NOTE_MELODY, SAW, 0, 95, 127, SWEEP_NONE, 0, 8, 5, 60, 0, 6, 0, 59},
		{NOTE_MELODY, SQUARE, 0, 31, 127, SWEEP_NONE, 0, 8, 5, 60, 0, 6, 0, 65},
		{NOTE_DRUM, SQUARE, 0, 31, 127, SWEEP_NONE, 0, 0, 5, 60, 75, 0, 0, 53},
		{NOTE_MELODY, TRIANGLE, 0, 31, 80, SWEEP_NONE, 0, 0, 4, 60, 30, 15, 0, 93},
		{NOTE_MELODY, TRIANGLE, 0, 31, 40, SWEEP_NONE, 0, 0, 4, 30, 10, 30, 0, 65},
		{NOTE_MELODY, SQUARE, 0, 31, 105, SWEEP_NONE, 0, 8, 5, 60, 0, 6, 0, 58},
		{NOTE_MELODY, SQUARE, 0, 31, 105, SWEEP_NONE, 0, 25, 20, 80, 0, 6, 0, 58},
		{NOTE_MELODY, SAW, 0, 31, 110, SWEEP_NONE, 0, 15, 20, 80, 0, 6, 0, 115},
		{NOTE_MELODY, SAW, 0, 31, 127, SWEEP_NONE, 0, 15, 20, 80, 0, 6, 0, 127},
		{NOTE_MELODY, SAW, 0, 47, 127, SWEEP_NONE, 0, 15, 20, 80, 0, 6, 0, 60},
		{NOTE_MELODY, SAW, 0, 47, 127, SWEEP_NONE, 0, 1, 8, 80, 0, 6, 0, 60},
		{NOTE_MELODY, SAW, 0, 63, 127, SWEEP_NONE, 0, 1, 8, 80, 0, 6, 0, 63},
		{NOTE_DRUM, SQUARE, 0, 79, 100, SWEEP_NONE, 0, 0, 8, 80, 45, 6, 0, 33},
		{NOTE_MELODY, SAW, 0, 95, 127, SWEEP_NONE, 0, 3, 8, 80, 0, 3, 0, 58},
		{NOTE_MELODY, SAW, 0, 127, 127, SWEEP_NONE, 0, 3, 8, 80, 0, 3, 0, 53},
		{NOTE_MELODY, SAW, 0, 127, 127, SWEEP_NONE, 0, 3, 8, 80, 0, 3, 0, 53},
		{NOTE_MELODY, SAW, 0, 63, 120, SWEEP_NONE, 0, 3, 8, 80, 0, 3, 0, 70},
		{NOTE_MELODY, SAW, 0, 31, 127, SWEEP_NONE, 0, 3, 50, 50, 0, 3, 0, 114},
		{NOTE_MELODY, SAW, 0, 47, 80, SWEEP_NONE, 0, 3, 50, 50, 0, 3, 0, 61},
		{NOTE_MELODY, SAW, 0, 63, 90, SWEEP_NONE, 0, 3, 50, 50, 0, 3, 0, 67},
		{NOTE_MELODY, SAW, 0, 63, 127, SWEEP_NONE, 0, 3, 50, 50, 0, 3, 0, 70},
		{NOTE_MELODY, SAW, 0, 63, 127, SWEEP_NONE, 0, 3, 5, 70, 0, 3, 0, 70},
		{NOTE_MELODY, SAW, 0, 63, 100, SWEEP_NONE, 0, 10, 5, 70, 0, 3, 0, 69},
		{NOTE_MELODY, SQUARE, 0, 63, 120, SWEEP_NONE, 0, 10, 5, 70, 0, 3, 0, 44},
		{NOTE_MELODY, SQUARE, 0, 63, 100, SWEEP_NONE, 0, 10, 5, 70, 0, 3, 0, 44},
		{NOTE_MELODY, SQUARE, 0, 31, 100, SWEEP_NONE, 0, 10, 5, 70, 0, 3, 0, 56},
		{NOTE_MELODY, SQUARE, 0, 31, 100, SWEEP_NONE, 0, 10, 5, 70, 0, 3, 0, 56},
		{NOTE_MELODY, TRIANGLE, 0, 31, 110, SWEEP_NONE, 0, 10, 5, 70, 0, 3, 0, 112},
		{NOTE_MELODY, TRIANGLE, 0, 31, 110, SWEEP_NONE, 0, 10, 5, 70, 0, 3, 0, 112},
		{NOTE_MELODY, SINE, 0, 31, 110, SWEEP_NONE, 0, 10, 5, 70, 0, 3, 0, 79},
		{NOTE_MELODY, SINE, 0, 63, 50, SWEEP_NONE, 0, 10, 5, 70, 0, 3, 0, 52},
		{NOTE_MELODY, SINE, 0, 63, 110, SWEEP_NONE, 0, 10, 5, 70, 0, 3, 0, 57},
		{NOTE_MELODY, SINE, 0, 63, 60, SWEEP_NONE, 0, 3, 50, 40, 0, 3, 0, 53},
		{NOTE_MELODY, SINE, 0, 63, 80, SWEEP_NONE, 0, 3, 50, 40, 0, 3, 0, 54},
		{NOTE_MELODY, SINE, 0, 63, 100, SWEEP_NONE, 0, 3, 80, 30, 0, 3, 0, 56},
		{NOTE_MELODY, TRIANGLE, 0, 63, 120, SWEEP_NONE, 0, 3, 20, 50, 0, 3, 0, 73},
		{NOTE_MELODY, SINE, 0, 63, 110, SWEEP_NONE, 0, 3, 20, 50, 0, 3, 0, 56},
		{NOTE_MELODY, SQUARE, 0, 0, 0, SWEEP_NONE, 0, 0, 0, 127, 0, 0, 0, 30},
		{NOTE_MELODY, SAW, 0, 0, 0, SWEEP_NONE, 0, 0, 0, 127, 0, 0, 0, 30},
		{NOTE_MELODY, SAW, 0, 63, 110, SWEEP_NONE, 0, 0, 0, 127, 0, 0, 0, 50},
		{NOTE_MELODY, SAW, 0, 63, 120, SWEEP_NONE, 0, 1, 25, 50, 0, 0, 0, 63},
		{NOTE_MELODY, SQUARE, 0, 63, 120, SWEEP_NONE, 0, 1, 25, 50, 0, 0, 0, 44},
		{NOTE_MELODY, SQUARE, 0, 79, 120, SWEEP_NONE, 0, 1, 25, 50, 0, 0, 0, 32},
		{NOTE_MELODY, TRIANGLE, 0, 111, 100, SWEEP_NONE, 0, 1, 25, 50, 0, 0, 0, 45},
		{NOTE_MELODY, SAW, 0, 127, 120, SWEEP_NONE, 0, 1, 25, 50, 0, 0, 0, 51},
		{NOTE_MELODY, SINE, 0, 95, 120, SWEEP_NONE, 0, 1, 25, 35, 0, 20, 0, 43},
		{NOTE_MELODY, SINE, 0, 95, 120, SWEEP_NONE, 0, 127, 127, 80, 0, 35, 0, 43},
		{NOTE_MELODY, SQUARE, 0, 95, 50, SWEEP_NONE, 0, 0, 127, 80, 0, 8, 0, 39},
		{NOTE_MELODY, SQUARE, 0, 95, 80, SWEEP_NONE, 0, 0, 127, 80, 0, 13, 0, 37},
		{NOTE_MELODY, TRIANGLE, 0, 95, 80, SWEEP_NONE, 0, 50, 127, 80, 0, 13, 0, 60},
		{NOTE_MELODY, TRIANGLE, 0, 127, 80, SWEEP_NONE, 0, 50, 127, 80, 0, 13, 0, 55},
		{NOTE_MELODY, TRIANGLE, 0, 127, 100, SWEEP_NONE, 0, 6, 127, 80, 0, 13, 0, 55},
		{NOTE_MELODY, SAW, 0, 95, 127, SWEEP_NONE, 0, 50, 127, 80, 0, 13, 0, 58},
		{NOTE_MELODY, TRIANGLE, 0, 1, 30, SWEEP_NONE, 0, 1, 50, 80, 0, 15, 0, 43},
		{NOTE_MELODY, TRIANGLE, 0, 79, 70, SWEEP_NONE, 0, 30, 50, 80, 0, 15, 0, 44},
		{NOTE_MELODY, SINE, 0, 47, 50, SWEEP_NONE, 0, 3, 50, 70, 12, 15, 0, 46},
		{NOTE_MELODY, SINE, 0, 63, 50, SWEEP_NONE, 0, 2, 120, 40, 0, 15, 0, 52},
		{NOTE_MELODY, SINE, 0, 63, 110, SWEEP_NONE, 0, 2, 120, 40, 10, 35, 0, 57},
		{NOTE_MELODY, SINE, 0, 63, 110, SWEEP_NONE, 0, 127, 120, 100, 10, 35, 0, 55},
		{NOTE_MELODY, SINE, 0, 95, 110, SWEEP_NONE, 0, 2, 30, 55, 0, 25, 0, 43},
		{NOTE_MELODY, SINE, 0, 127, 110, SWEEP_NONE, 0, 2, 30, 55, 0, 25, 0, 46},
		{NOTE_MELODY, SAW, 0, 127, 127, SWEEP_NONE, 0, 0, 3, 45, 4, 15, 0, 53},
		{NOTE_MELODY, TRIANGLE, 0, 127, 127, SWEEP_NONE, 0, 0, 3, 45, 4, 13, 0, 56},
		{NOTE_MELODY, SAW, 0, 127, 120, SWEEP_NONE, 0, 0, 3, 45, 15, 50, 0, 54},
		{NOTE_MELODY, SQUARE, 0, 127, 127, SWEEP_NONE, 0, 0, 3, 45, 15, 50, 0, 43},
		{NOTE_MELODY, SQUARE, 0, 63, 127, SWEEP_NONE, 0, 0, 3, 45, 28, 18, 0, 42},
		{NOTE_MELODY, SAW, 0, 95, 127, SWEEP_NONE, 0, 8, 3, 45, 0, 3, 0, 58},
		{NOTE_MELODY, SAW, 0, 127, 127, SWEEP_NONE, 0, 8, 3, 45, 0, 3, 0, 53},
		{NOTE_MELODY, SAW, 0, 95, 100, SWEEP_NONE, 0, 8, 3, 45, 0, 3, 0, 57},
		{NOTE_MELODY, SINE, 0, 79, 100, SWEEP_NONE, 0, 0, 8, 45, 25, 35, 0, 45},
		{NOTE_DRUM, SINE, 0, 127, 100, SWEEP_NONE, 40, 0, 8, 45, 110, 6, 0, 46},
		{NOTE_DRUM, SINE, 0, 127, 110, SWEEP_NONE, 0, 0, 8, 45, 52, 5, 0, 46},
		{NOTE_DRUM, SINE, 0, 100, 50, SWEEP_NONE, 50, 0, 8, 45, 120, 5, 0, 46},
		{NOTE_DRUM, TRIANGLE, 0, 20, 127, SWEEP_DOWN, 35, 0, 8, 80, 127, 5, 0, 47},
		{NOTE_DRUM, SINE, 0, 31, 127, SWEEP_DOWN, 35, 0, 8, 80, 127, 5, 0, 50},
		{NOTE_DRUM, SQUARE, 0, 31, 127, SWEEP_DOWN, 35, 0, 8, 80, 127, 5, 0, 53},
		{NOTE_MELODY, PCM_REV, 9, 31, 0, SWEEP_NONE, 0, 0, 8, 127, 0, 0, 0, 80},
		{NOTE_DRUM, SQUARE, 0, 31, 127, SWEEP_UP, 30, 3, 8, 50, 0, 5, 0, 62},
		{NOTE_DRUM, WHITE_NOISE, 0, 31, 0, SWEEP_NONE, 10, 8, 8, 50, 127, 5, 0, 36},
		{NOTE_MELODY, WHITE_NOISE, 0, 0, 127, SWEEP_NONE, 0, 8, 8, 50, 0, 5, 0, 88},
		{NOTE_MELODY, SINE, 0, 0, 0, SWEEP_UP, 100, 8, 8, 50, 0, 5, 0, 43},
		{NOTE_MELODY, SINE, 0, 0, 127, SWEEP_NONE, 0, 0, 0, 127, 0, 0, 0, 43},
		{NOTE_MELODY, WHITE_NOISE, 0, 1, 127, SWEEP_NONE, 0, 127, 0, 127, 0, 50, 0, 33},
		{NOTE_MELODY, WHITE_NOISE, 0, 3, 127, SWEEP_NONE, 0, 127, 0, 127, 0, 30, 0, 33},
		{NOTE_MELODY, WHITE_NOISE, 0, 10, 127, SWEEP_NONE, 0, 0, 10, 100, 40, 3, 0, 39},

};

//GM1 Patch for drums.
const MidiPatch patch_drum[128] = {
		// note type  wave  wavenum  rm  rg  Sweeptyp  spd A  D   SL  SR  R  Nt  Gain
		{NOTE_DRUM, SQUARE, 0, 31, 127, SWEEP_DOWN, 64, 0, 0, 127, 0, 0, 38, 60},
		{NOTE_DRUM, SQUARE, 0, 31, 127, SWEEP_DOWN, 64, 0, 0, 127, 0, 0, 38, 60},
		{NOTE_DRUM, SQUARE, 0, 31, 127, SWEEP_DOWN, 64, 0, 0, 127, 0, 0, 38, 60},
		{NOTE_DRUM, SQUARE, 0, 31, 127, SWEEP_DOWN, 64, 0, 0, 127, 0, 0, 38, 60},
		{NOTE_DRUM, SQUARE, 0, 31, 127, SWEEP_DOWN, 64, 0, 0, 127, 0, 0, 38, 60},
		{NOTE_DRUM, SQUARE, 0, 31, 127, SWEEP_DOWN, 64, 0, 0, 127, 0, 0, 38, 60},
		{NOTE_DRUM, SQUARE, 0, 31, 127, SWEEP_DOWN, 64, 0, 0, 127, 0, 0, 38, 60},
		{NOTE_DRUM, SQUARE, 0, 31, 127, SWEEP_DOWN, 64, 0, 0, 127, 0, 0, 38, 60},
		{NOTE_DRUM, SQUARE, 0, 31, 127, SWEEP_DOWN, 64, 0, 0, 127, 0, 0, 38, 60},
		{NOTE_DRUM, SQUARE, 0, 31, 127, SWEEP_DOWN, 64, 0, 0, 127, 0, 0, 38, 60},
		{NOTE_DRUM, SQUARE, 0, 31, 127, SWEEP_DOWN, 64, 0, 0, 127, 0, 0, 38, 60},
		{NOTE_DRUM, SQUARE, 0, 31, 127, SWEEP_DOWN, 64, 0, 0, 127, 0, 0, 38, 60},
		{NOTE_DRUM, SQUARE, 0, 31, 127, SWEEP_DOWN, 64, 0, 0, 127, 0, 0, 38, 60},
		{NOTE_DRUM, SQUARE, 0, 31, 127, SWEEP_DOWN, 64, 0, 0, 127, 0, 0, 38, 60},
		{NOTE_DRUM, SQUARE, 0, 31, 127, SWEEP_DOWN, 64, 0, 0, 127, 0, 0, 38, 60},
		{NOTE_DRUM, SQUARE, 0, 31, 127, SWEEP_DOWN, 64, 0, 0, 127, 0, 0, 38, 60},
		{NOTE_DRUM, SQUARE, 0, 31, 127, SWEEP_DOWN, 64, 0, 0, 127, 0, 0, 38, 60},
		{NOTE_DRUM, SQUARE, 0, 31, 127, SWEEP_DOWN, 64, 0, 0, 127, 0, 0, 38, 60},
		{NOTE_DRUM, SQUARE, 0, 31, 127, SWEEP_DOWN, 64, 0, 0, 127, 0, 0, 38, 60},
		{NOTE_DRUM, SQUARE, 0, 31, 127, SWEEP_DOWN, 64, 0, 0, 127, 0, 0, 38, 60},
		{NOTE_DRUM, SQUARE, 0, 31, 127, SWEEP_DOWN, 64, 0, 0, 127, 0, 0, 38, 60},
		{NOTE_DRUM, SQUARE, 0, 31, 127, SWEEP_DOWN, 64, 0, 0, 127, 0, 0, 38, 60},
		{NOTE_DRUM, SQUARE, 0, 31, 127, SWEEP_DOWN, 64, 0, 0, 127, 0, 0, 38, 60},
		{NOTE_DRUM, SQUARE, 0, 31, 127, SWEEP_DOWN, 64, 0, 0, 127, 0, 0, 38, 60},
		{NOTE_DRUM, SQUARE, 0, 31, 127, SWEEP_DOWN, 64, 0, 0, 127, 0, 0, 38, 60},
		{NOTE_DRUM, SQUARE, 0, 31, 127, SWEEP_DOWN, 64, 0, 0, 127, 0, 0, 38, 60},
		{NOTE_DRUM, SQUARE, 0, 31, 127, SWEEP_DOWN, 64, 0, 0, 127, 0, 0, 38, 60},
		{NOTE_DRUM, SQUARE, 0, 31, 127, SWEEP_DOWN, 64, 0, 0, 127, 0, 0, 38, 60},
		{NOTE_DRUM, SQUARE, 0, 31, 127, SWEEP_DOWN, 64, 0, 0, 127, 0, 0, 38, 60},
		{NOTE_DRUM, SQUARE, 0, 31, 127, SWEEP_DOWN, 64, 0, 0, 127, 0, 0, 38, 60},
		{NOTE_DRUM, SQUARE, 0, 31, 127, SWEEP_DOWN, 64, 0, 0, 127, 0, 0, 38, 60},
		{NOTE_DRUM, SQUARE, 0, 31, 127, SWEEP_DOWN, 64, 0, 0, 127, 0, 0, 38, 60},
		{NOTE_DRUM, SQUARE, 0, 31, 127, SWEEP_DOWN, 64, 0, 0, 127, 0, 0, 38, 60},
		{NOTE_DRUM, SQUARE, 0, 31, 127, SWEEP_DOWN, 64, 0, 0, 127, 0, 0, 38, 60},
		{NOTE_DRUM, SQUARE, 0, 31, 127, SWEEP_DOWN, 64, 0, 0, 127, 0, 0, 38, 60},
		{NOTE_DRUM, PCM, 4, 31, 0, SWEEP_NONE, 0, 0, 0, 127, 0, 0, 69, 78},
		{NOTE_DRUM, PCM, 5, 31, 0, SWEEP_NONE, 0, 0, 0, 127, 0, 0, 69, 78},
		{NOTE_DRUM, PCM, 3, 31, 0, SWEEP_NONE, 0, 0, 0, 127, 0, 0, 69, 78},
		{NOTE_DRUM, PCM, 1, 31, 0, SWEEP_NONE, 0, 0, 0, 127, 0, 0, 69, 78},
		{NOTE_DRUM, PCM, 6, 31, 0, SWEEP_NONE, 0, 0, 0, 127, 0, 0, 65, 78},
		{NOTE_DRUM, PCM, 2, 31, 0, SWEEP_NONE, 0, 0, 0, 127, 0, 0, 69, 65},
		{NOTE_DRUM, PCM, 0, 31, 0, SWEEP_NONE, 0, 0, 0, 127, 0, 0, 60, 65},
		{NOTE_DRUM, PCM, 11, 31, 0, SWEEP_NONE, 0, 0, 0, 127, 0, 0, 69, 91},
		{NOTE_DRUM, PCM, 0, 31, 0, SWEEP_NONE, 0, 0, 0, 127, 0, 0, 63, 65},
		{NOTE_DRUM, PCM, 12, 31, 0, SWEEP_NONE, 0, 0, 0, 127, 0, 0, 69, 78},
		{NOTE_DRUM, PCM, 0, 31, 0, SWEEP_NONE, 0, 0, 0, 127, 0, 0, 66, 65},
		{NOTE_DRUM, PCM, 10, 31, 0, SWEEP_NONE, 0, 0, 0, 127, 0, 0, 69, 58},
		{NOTE_DRUM, PCM, 0, 31, 0, SWEEP_NONE, 0, 0, 0, 127, 0, 0, 69, 65},
		{NOTE_DRUM, PCM, 0, 31, 0, SWEEP_NONE, 0, 0, 0, 127, 0, 0, 72, 65},
		{NOTE_DRUM, PCM, 9, 31, 0, SWEEP_NONE, 0, 0, 0, 127, 0, 0, 69, 80},
		{NOTE_DRUM, PCM, 0, 31, 0, SWEEP_NONE, 0, 0, 0, 127, 0, 0, 75, 50},
		{NOTE_DRUM, PCM, 8, 31, 0, SWEEP_NONE, 0, 0, 0, 127, 0, 0, 65, 91},
		{NOTE_DRUM, PCM, 9, 31, 0, SWEEP_NONE, 0, 0, 0, 127, 0, 0, 55, 80},
		{NOTE_DRUM, PCM, 8, 31, 0, SWEEP_NONE, 0, 0, 0, 127, 0, 0, 67, 91},
		{NOTE_DRUM, WHITE_NOISE, 0, 31, 80, SWEEP_NONE, 0, 0, 6, 25, 127, 0, 80, 60},
		{NOTE_DRUM, PCM, 7, 31, 0, SWEEP_NONE, 0, 0, 0, 127, 0, 0, 69, 65},
		{NOTE_DRUM, TRIANGLE, 0, 118, 120, SWEEP_NONE, 0, 1, 3, 25, 70, 0, 71, 60},
		{NOTE_DRUM, PCM, 9, 31, 0, SWEEP_NONE, 0, 0, 0, 127, 0, 0, 72, 80},
		{NOTE_DRUM, WHITE_NOISE, 0, 20, 127, SWEEP_NONE, 0, 1, 3, 75, 37, 0, 15, 45},
		{NOTE_DRUM, PCM, 8, 31, 0, SWEEP_NONE, 0, 0, 0, 127, 0, 0, 69, 91},
		{NOTE_DRUM, SINE, 0, 55, 127, SWEEP_NONE, 0, 0, 13, 10, 40, 0, 71, 45},
		{NOTE_DRUM, SINE, 0, 55, 127, SWEEP_NONE, 0, 0, 13, 10, 40, 0, 60, 60},
		{NOTE_DRUM, SINE, 0, 55, 127, SWEEP_NONE, 0, 0, 5, 15, 50, 0, 56, 60},
		{NOTE_DRUM, SINE, 0, 55, 127, SWEEP_NONE, 0, 0, 5, 15, 40, 0, 64, 60},
		{NOTE_DRUM, SINE, 0, 55, 127, SWEEP_NONE, 0, 0, 2, 25, 80, 0, 60, 60},
		{NOTE_DRUM, TRIANGLE, 0, 70, 127, SWEEP_NONE, 0, 0, 2, 45, 120, 0, 62, 60},
		{NOTE_DRUM, TRIANGLE, 0, 70, 127, SWEEP_NONE, 0, 0, 2, 45, 120, 0, 59, 60},
		{NOTE_DRUM, TRIANGLE, 0, 122, 127, SWEEP_NONE, 0, 0, 3, 30, 127, 0, 81, 60},
		{NOTE_DRUM, TRIANGLE, 0, 122, 127, SWEEP_NONE, 0, 0, 3, 30, 127, 0, 75, 60},
		{NOTE_DRUM, WHITE_NOISE, 0, 127, 127, SWEEP_NONE, 0, 6, 3, 25, 127, 0, 75, 45},
		{NOTE_DRUM, WHITE_NOISE, 0, 127, 127, SWEEP_DOWN, 127, 2, 3, 25, 127, 0, 75, 45},
		{NOTE_DRUM, SAW, 0, 97, 127, SWEEP_NONE, 0, 2, 3, 25, 127, 0, 87, 60},
		{NOTE_DRUM, SAW, 0, 97, 127, SWEEP_NONE, 17, 2, 3, 25, 10, 0, 83, 60},
		{NOTE_DRUM, WHITE_NOISE, 0, 120, 127, SWEEP_NONE, 45, 1, 3, 25, 10, 0, 19, 45},
		{NOTE_DRUM, WHITE_NOISE, 0, 120, 127, SWEEP_UP, 17, 1, 3, 25, 10, 0, 19, 60},
		{NOTE_DRUM, SINE, 0, 100, 127, SWEEP_NONE, 110, 0, 1, 50, 0, 0, 84, 45},
		{NOTE_DRUM, TRIANGLE, 0, 88, 120, SWEEP_NONE, 60, 0, 1, 40, 127, 0, 82, 60},
		{NOTE_DRUM, TRIANGLE, 0, 88, 120, SWEEP_NONE, 60, 0, 1, 40, 127, 0, 77, 60},
		{NOTE_DRUM, SQUARE, 0, 100, 127, SWEEP_NONE, 45, 1, 5, 30, 80, 0, 77, 38},
		{NOTE_DRUM, SQUARE, 0, 100, 127, SWEEP_NONE, 45, 1, 5, 30, 80, 0, 71, 38},
		{NOTE_DRUM, SQUARE, 0, 86, 120, SWEEP_NONE, 80, 0, 3, 30, 0, 0, 90, 38},
		{NOTE_DRUM, SQUARE, 0, 86, 120, SWEEP_NONE, 0, 0, 3, 30, 20, 0, 90, 38},
		{NOTE_DRUM, SQUARE, 0, 31, 127, SWEEP_DOWN, 64, 0, 0, 127, 0, 0, 38, 60},
		{NOTE_DRUM, SQUARE, 0, 31, 127, SWEEP_DOWN, 64, 0, 0, 127, 0, 0, 38, 60},
		{NOTE_DRUM, SQUARE, 0, 31, 127, SWEEP_DOWN, 64, 0, 0, 127, 0, 0, 38, 60},
		{NOTE_DRUM, SQUARE, 0, 31, 127, SWEEP_DOWN, 64, 0, 0, 127, 0, 0, 38, 60},
		{NOTE_DRUM, SQUARE, 0, 31, 127, SWEEP_DOWN, 64, 0, 0, 127, 0, 0, 38, 60},
		{NOTE_DRUM, SQUARE, 0, 31, 127, SWEEP_DOWN, 64, 0, 0, 127, 0, 0, 38, 60},
		{NOTE_DRUM, SQUARE, 0, 31, 127, SWEEP_DOWN, 64, 0, 0, 127, 0, 0, 38, 60},
		{NOTE_DRUM, SQUARE, 0, 31, 127, SWEEP_DOWN, 64, 0, 0, 127, 0, 0, 38, 60},
		{NOTE_DRUM, SQUARE, 0, 31, 127, SWEEP_DOWN, 64, 0, 0, 127, 0, 0, 38, 60},
		{NOTE_DRUM, SQUARE, 0, 31, 127, SWEEP_DOWN, 64, 0, 0, 127, 0, 0, 38, 60},
		{NOTE_DRUM, SQUARE, 0, 31, 127, SWEEP_DOWN, 64, 0, 0, 127, 0, 0, 38, 60},
		{NOTE_DRUM, SQUARE, 0, 31, 127, SWEEP_DOWN, 64, 0, 0, 127, 0, 0, 38, 60},
		{NOTE_DRUM, SQUARE, 0, 31, 127, SWEEP_DOWN, 64, 0, 0, 127, 0, 0, 38, 60},
		{NOTE_DRUM, SQUARE, 0, 31, 127, SWEEP_DOWN, 64, 0, 0, 127, 0, 0, 38, 60},
		{NOTE_DRUM, SQUARE, 0, 31, 127, SWEEP_DOWN, 64, 0, 0, 127, 0, 0, 38, 60},
		{NOTE_DRUM, SQUARE, 0, 31, 127, SWEEP_DOWN, 64, 0, 0, 127, 0, 0, 38, 60},
		{NOTE_DRUM, SQUARE, 0, 31, 127, SWEEP_DOWN, 64, 0, 0, 127, 0, 0, 38, 60},
		{NOTE_DRUM, SQUARE, 0, 31, 127, SWEEP_DOWN, 64, 0, 0, 127, 0, 0, 38, 60},
		{NOTE_DRUM, SQUARE, 0, 31, 127, SWEEP_DOWN, 64, 0, 0, 127, 0, 0, 38, 60},
		{NOTE_DRUM, SQUARE, 0, 31, 127, SWEEP_DOWN, 64, 0, 0, 127, 0, 0, 38, 60},
		{NOTE_DRUM, SQUARE, 0, 31, 127, SWEEP_DOWN, 64, 0, 0, 127, 0, 0, 38, 60},
		{NOTE_DRUM, SQUARE, 0, 31, 127, SWEEP_DOWN, 64, 0, 0, 127, 0, 0, 38, 60},
		{NOTE_DRUM, SQUARE, 0, 31, 127, SWEEP_DOWN, 64, 0, 0, 127, 0, 0, 38, 60},
		{NOTE_DRUM, SQUARE, 0, 31, 127, SWEEP_DOWN, 64, 0, 0, 127, 0, 0, 38, 60},
		{NOTE_DRUM, SQUARE, 0, 31, 127, SWEEP_DOWN, 64, 0, 0, 127, 0, 0, 38, 60},
		{NOTE_DRUM, SQUARE, 0, 31, 127, SWEEP_DOWN, 64, 0, 0, 127, 0, 0, 38, 60},
		{NOTE_DRUM, SQUARE, 0, 31, 127, SWEEP_DOWN, 64, 0, 0, 127, 0, 0, 38, 60},
		{NOTE_DRUM, SQUARE, 0, 31, 127, SWEEP_DOWN, 64, 0, 0, 127, 0, 0, 38, 60},
		{NOTE_DRUM, SQUARE, 0, 31, 127, SWEEP_DOWN, 64, 0, 0, 127, 0, 0, 38, 60},
		{NOTE_DRUM, SQUARE, 0, 31, 127, SWEEP_DOWN, 64, 0, 0, 127, 0, 0, 38, 60},
		{NOTE_DRUM, SQUARE, 0, 31, 127, SWEEP_DOWN, 64, 0, 0, 127, 0, 0, 38, 60},
		{NOTE_DRUM, SQUARE, 0, 31, 127, SWEEP_DOWN, 64, 0, 0, 127, 0, 0, 38, 60},
		{NOTE_DRUM, SQUARE, 0, 31, 127, SWEEP_DOWN, 64, 0, 0, 127, 0, 0, 38, 60},
		{NOTE_DRUM, SQUARE, 0, 31, 127, SWEEP_DOWN, 64, 0, 0, 127, 0, 0, 38, 60},
		{NOTE_DRUM, SQUARE, 0, 31, 127, SWEEP_DOWN, 64, 0, 0, 127, 0, 0, 38, 60},
		{NOTE_DRUM, SQUARE, 0, 31, 127, SWEEP_DOWN, 64, 0, 0, 127, 0, 0, 38, 60},
		{NOTE_DRUM, SQUARE, 0, 31, 127, SWEEP_DOWN, 64, 0, 0, 127, 0, 0, 38, 60},
		{NOTE_DRUM, SQUARE, 0, 31, 127, SWEEP_DOWN, 64, 0, 0, 127, 0, 0, 38, 60},
		{NOTE_DRUM, SQUARE, 0, 31, 127, SWEEP_DOWN, 64, 0, 0, 127, 0, 0, 38, 60},
		{NOTE_DRUM, SQUARE, 0, 31, 127, SWEEP_DOWN, 64, 0, 0, 127, 0, 0, 38, 60},
		{NOTE_DRUM, SQUARE, 0, 31, 127, SWEEP_DOWN, 64, 0, 0, 127, 0, 0, 38, 60},
		{NOTE_DRUM, SQUARE, 0, 31, 127, SWEEP_DOWN, 64, 0, 0, 127, 0, 0, 38, 60},
		{NOTE_DRUM, SQUARE, 0, 31, 127, SWEEP_DOWN, 64, 0, 0, 127, 0, 0, 38, 60},
		{NOTE_DRUM, SQUARE, 0, 31, 127, SWEEP_DOWN, 64, 0, 0, 127, 0, 0, 38, 60},
		{NOTE_DRUM, SQUARE, 0, 31, 127, SWEEP_DOWN, 64, 0, 0, 127, 0, 0, 38, 60},
		{NOTE_DRUM, SQUARE, 0, 31, 127, SWEEP_DOWN, 64, 0, 0, 127, 0, 0, 38, 60},
};


#endif /* CURELIB_INC_PATCHLIST_H_ */
