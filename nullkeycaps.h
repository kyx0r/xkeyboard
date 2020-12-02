/* nullkeycaps.h
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

#ifndef __XKEYCAPS_H__
#define __XKEYCAPS_H__
#include "glassdown.h"
#include "glassup.h"

unsigned char nullkeydata[2] = {
	0xa5, 0xa5
};

struct pgmdata nullkeypgm = {
	1, /* width */
	1, /* height */
	nullkeydata
};

struct pgmdata *mykclist[] = {
	&nullkeypgm,	/* 0 */
	&nullkeypgm,	/* 1 */
	&nullkeypgm,	/* 2 */
	&nullkeypgm,	/* 3 */
	&nullkeypgm,	/* 4 */
	&nullkeypgm,	/* 5 */
	&nullkeypgm,	/* 6 */
	&nullkeypgm,	/* 7 */
	&nullkeypgm,	/* 8 */
	&nullkeypgm,	/* 9 */
	&nullkeypgm,	/* 10 */
	&nullkeypgm,	/* 11 */
	&nullkeypgm,	/* 12 */
	&glassdownpgm,	/* NUM_KEYCAPS - 4 */
	&glassuppgm,	/* NUM_KEYCAPS - 3 */
	&nullkeypgm,	/* NUM_KEYCAPS - 2 */
	&nullkeypgm,	/* NUM_KEYCAPS - 1 */
};
#define NUM_KEYCAPS	(sizeof (mykclist) / sizeof(struct pgmdata *))
#define BORDER 1

#endif
