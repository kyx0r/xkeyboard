/* xkeycaps.h
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

#include "backslash.h"
#include "backspace.h"
#include "capslock.h"
#include "control.h"
#include "darkkey.h"
#include "enter.h"
#include "keypadlong.h"
#include "keypadzero.h"
#include "lightkey.h"
#include "shiftleft.h"
#include "shiftright.h"
#include "spacebar.h"
#include "tab.h"
#include "glassdown.h"
#include "glassup.h"
#include "tmlogo.h"
#include "texture.h"
#include "closewindow_up.h"
#include "closewindow_down.h"
#include "lockdown.h"
#include "lockup.h"
#include "move_down.h"
#include "move_up.h"
#include "lbcorner.h"
#include "fmode.h"
#include "gmode.h"
#include "mouse_up.h"
#include "mouse_down.h"

struct scaledata
{
	int w, h;
	struct pgmdata *pgm;
};

struct scaledata mykclist[] = {
	{48, 32, &backslashpgm},
	{64, 32, &capslockpgm},	
	{40, 32, &controlpgm},	
	{32, 32, &darkkeypgm},
	{64, 32, &enterpgm},
	{32, 64, &keypadlongpgm},		
	{32, 32, &lightkeypgm},
	{80, 32, &shiftleftpgm},
	{80, 32, &shiftrightpgm},
	{200,32, &spacebarpgm},
	{64, 32, &backspacepgm},
	{64, 32, &keypadzeropgm},
	{48, 32, &tabpgm},
};
#define NUM_KEYCAPS	(sizeof (mykclist) / sizeof(struct scaledata))

struct scaledata cpgmlist[] = {
	{40, 40, &mousedownpgm},	
	{40, 40, &mouseuppgm},	
	{40, 40, &fmodepgm},	
	{40, 40, &gmodepgm},	
	{16, 16, &lbcornerpgm},
	{40, 40, &movedownpgm},	
	{40, 40, &moveuppgm},	
	{20, 20, &lockuppgm},	
	{20, 20, &lockdownpgm},	
	{16, 16, &closewindow_downpgm},
	{16, 16, &closewindow_uppgm},
	{40, 40, &glassdownpgm},
	{40, 40, &glassuppgm},	
	{140,32, &tmlogopgm},	
	{32, 32, &texturepgm},	
};
#define NUM_CONSTPGM	(sizeof (cpgmlist) / sizeof(struct scaledata))
#define BORDER 0

#endif
