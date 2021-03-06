/* xkeyboard.h
 *
 * Copyright (C) 1999 Transmeta Corporation
 *
 * written by Nathan Laredo <nlaredo@transmeta.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __XKEYBOARD_H__
#define __XKEYBOARD_H__

struct mykeys
{
	int scancode, x, y, w, h, p;
};

struct myborders
{
	int x, y, w, h;
};

#define NUM_BORDERS	19
struct myborders mb[NUM_BORDERS] = {
	{   0,  32,  36,   0},
	{  32,   0,   0,  36},
	{  64,  32, 132,   0},
	{ 192,   0,   0,  36},
	{ 208,  32, 132,   0},
	{ 336,   0,   0,  36},
	{ 352,  32, 132,   0},
	{ 480,   0,   0,  36},
	{ 496,  32, 100,   0},
	{ 592,   0,   0,  36},
	{   0, 208, 484,   0},
	{ 480,  48,   0, 164},
	{ 496, 112,  98,   0},
	{ 592,  48,   0,  66},
	{ 496, 208, 100,   0},
	{ 592, 176,   0,  36},
	{ 560, 144,   0,  32},
	{ 608, 208, 132,   0},
	{ 736,  48,   0, 164}
};

#define MAX_KEYS	105

struct mykeys xk[MAX_KEYS] = {
	{   9,   0,   0,  32,  32, 3},
	{  67,  64,   0,  32,  32, 6},
	{  68,  96,   0,  32,  32, 6},
	{  69, 128,   0,  32,  32, 6},
	{  70, 160,   0,  32,  32, 6},
	{  71, 208,   0,  32,  32, 3},
	{  72, 240,   0,  32,  32, 3},
	{  73, 272,   0,  32,  32, 3},
	{  74, 304,   0,  32,  32, 3},
	{  75, 352,   0,  32,  32, 6},
	{  76, 384,   0,  32,  32, 6},
	{  95, 416,   0,  32,  32, 6},
	{  96, 448,   0,  32,  32, 6},
	{ 111, 496,   0,  32,  32, 3},
	{  78, 528,   0,  32,  32, 3},
	{ 110, 560,   0,  32,  32, 3},
	{  49,   0,  48,  32,  32, 6},
	{  10,  32,  48,  32,  32, 6},
	{  11,  64,  48,  32,  32, 6},
	{  12,  96,  48,  32,  32, 6},
	{  13, 128,  48,  32,  32, 6},
	{  14, 160,  48,  32,  32, 6},
	{  15, 192,  48,  32,  32, 6},
	{  16, 224,  48,  32,  32, 6},
	{  17, 256,  48,  32,  32, 6},
	{  18, 288,  48,  32,  32, 6},
	{  19, 320,  48,  32,  32, 6},
	{  20, 352,  48,  32,  32, 6},
	{  21, 384,  48,  32,  32, 6},
	{  22, 416,  48,  64,  32, 10},
	{ 106, 496,  48,  32,  32, 3},
	{  97, 528,  48,  32,  32, 3},
	{  99, 560,  48,  32,  32, 3},
	{  77, 608,  48,  32,  32, 3},
	{ 112, 640,  48,  32,  32, 3},
	{  63, 672,  48,  32,  32, 3},
	{  82, 704,  48,  32,  32, 3},
	{  23,   0,  80,  48,  32, 12},
	{  24,  48,  80,  32,  32, 6},
	{  25,  80,  80,  32,  32, 6},
	{  26, 112,  80,  32,  32, 6},
	{  27, 144,  80,  32,  32, 6},
	{  28, 176,  80,  32,  32, 6},
	{  29, 208,  80,  32,  32, 6},
	{  30, 240,  80,  32,  32, 6},
	{  31, 272,  80,  32,  32, 6},
	{  32, 304,  80,  32,  32, 6},
	{  33, 336,  80,  32,  32, 6},
	{  34, 368,  80,  32,  32, 6},
	{  35, 400,  80,  32,  32, 6},
	{  51, 432,  80,  48,  32, 0},
	{ 107, 496,  80,  32,  32, 3},
	{ 103, 528,  80,  32,  32, 3},
	{ 105, 560,  80,  32,  32, 3},
	{  79, 608,  80,  32,  32, 6},
	{  80, 640,  80,  32,  32, 6},
	{  81, 672,  80,  32,  32, 6},
	{  66,   0, 112,  64,  32, 1},
	{  38,  64, 112,  32,  32, 6},
	{  39,  96, 112,  32,  32, 6},
	{  40, 128, 112,  32,  32, 6},
	{  41, 160, 112,  32,  32, 6},
	{  42, 192, 112,  32,  32, 6},
	{  43, 224, 112,  32,  32, 6},
	{  44, 256, 112,  32,  32, 6},
	{  45, 288, 112,  32,  32, 6},
	{  46, 320, 112,  32,  32, 6},
	{  47, 352, 112,  32,  32, 6},
	{  48, 384, 112,  32,  32, 6},
	{  36, 416, 112,  64,  32, 4},
	{  83, 608, 112,  32,  32, 6},
	{  84, 640, 112,  32,  32, 6},
	{  85, 672, 112,  32,  32, 6},
	{  50,   0, 144,  80,  32, 8},
	{  94,  48, 144,  32,  32, 6},
	{  52,  80, 144,  32,  32, 6},
	{  53, 112, 144,  32,  32, 6},
	{  54, 144, 144,  32,  32, 6},
	{  55, 176, 144,  32,  32, 6},
	{  56, 208, 144,  32,  32, 6},
	{  57, 240, 144,  32,  32, 6},
	{  58, 272, 144,  32,  32, 6},
	{  59, 304, 144,  32,  32, 6},
	{  60, 336, 144,  32,  32, 6},
	{  61, 368, 144,  32,  32, 6},
	{  62, 400, 144,  80,  32, 7},
	{  98, 528, 144,  32,  32, 3},
	{  87, 608, 144,  32,  32, 6},
	{  88, 640, 144,  32,  32, 6},
	{  89, 672, 144,  32,  32, 6},
	{  37,   0, 176,  40,  32, 2},
	{ 115,  40, 176,  40,  32, 2},
	{  64,  80, 176,  40,  32, 2},
	{  65, 120, 176, 200,  32, 9},
	{ 113, 320, 176,  40,  32, 2},
	{ 116, 360, 176,  40,  32, 2},
	{ 117, 400, 176,  40,  32, 2},
	{ 109, 440, 176,  40,  32, 2},
	{ 100, 496, 176,  32,  32, 3},
	{ 104, 528, 176,  32,  32, 3},
	{ 102, 560, 176,  32,  32, 3},
	{  90, 608, 176,  64,  32, 11},
	{  91, 672, 176,  32,  32, 6},
	{  86, 704,  80,  32,  64, 5},
	{ 108, 704, 144,  32,  64, 5},
};
#endif
