/* xkeyboard.c 
 *
 * Copyright (C) 1999-2020 
 *
 * original written by Nathan Laredo <nlaredo@transmeta.com>
 * improved by Kyryl Melekhin <https://kyryl.tk>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/time.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/cursorfont.h>
#include <X11/extensions/XTest.h>
#include "xkeyboard.h"
#ifdef NO_GRAPHICS
#include "nullkeycaps.h"
#else
#include "xkeycaps.h"
#endif

#define BUTTONMASK              (ButtonPressMask|ButtonReleaseMask)
#define MOUSEMASK               (BUTTONMASK|PointerMotionMask)

char *scanname[8][256];
int modifiermap[8][16];
int modindex[8];

Display *dpy;

Window mywindow, mykey[MAX_KEYS], 
zoomwin, closewindow,
movewin, root, lbcornerwin, modewin, mousewin;

Window myborder[NUM_BORDERS];
XWindowAttributes attribs;
GC mygc, mygcled, mygcred, myact;
XGCValues gcvals;
XEvent myevent;
int myscreen;
Cursor mycursor, closecursor;
XSizeHints myhint;
XFontStruct *font_struct;
unsigned long myforeground, mybackground;
XSetWindowAttributes myattribs;	/* for window creation */
long myevents;
int keystate;
char tmpbuf[16];
int zoom = 0, ox, oy, sn, sd;
int screenwidth, screenheight;
int lockmod;
int sendkey;
Pixmap mykc[NUM_KEYCAPS];
Pixmap cpgm[NUM_CONSTPGM];
enum { NORMAL, CLOSING } closingstate;
int xkeyoffs, ykeyoffs;
#define p(...) fprintf(stderr, __VA_ARGS__); \
fprintf(stderr, "\n"); \

static int handle_error(Display *d, XErrorEvent *event)
{
	char errortext[96];
	XGetErrorText(dpy, event->error_code, errortext, 96);

	fprintf(stderr, "xkeyboard: %s (%d/%d)\n", errortext,
		event->request_code, event->minor_code);
	return 0;
}

void init_display(int argc, char **argv)
{
	XColor mycolor;

	if (!getenv("DISPLAY")) {
		fprintf(stderr, "DISPLAY not set.\n");
		exit(1);
	}
	if (!(dpy = XOpenDisplay(getenv("DISPLAY")))) {
		fprintf(stderr, "Unable to open display.\n");
		exit(1);
	}
	XSetErrorHandler(handle_error);
	XFlush(dpy);
#if 0
	/* for acting as a window manager */
	XSelectInput(dpy, DefaultRootWindow(dpy),
		SubstructureRedirectMask);
#endif
	myscreen = DefaultScreen(dpy);
	screenwidth = DisplayWidth(dpy, myscreen);
	screenheight = DisplayHeight(dpy, myscreen);
	root = DefaultRootWindow(dpy);
	mybackground = BlackPixel(dpy, myscreen);
	myforeground = WhitePixel(dpy, myscreen);
	myhint.width = 16;
	myhint.height = 16;
	myhint.x = 0;
	myhint.y = screenheight - 16;
	myhint.flags = PSize | PMinSize | PMaxSize | PPosition;
	myattribs.event_mask = ButtonPressMask | VisibilityChangeMask |
		EnterWindowMask;
	myattribs.background_pixel = mybackground;
	myattribs.override_redirect = True;
	mywindow = XCreateWindow(dpy, DefaultRootWindow(dpy),
		myhint.x, myhint.y, myhint.width, myhint.height, 0,
		attribs.depth, InputOutput, attribs.visual,
		CWEventMask | CWBackPixel | CWOverrideRedirect, &myattribs);
	strcpy(tmpbuf, "xkeyboard");
	myattribs.event_mask = ButtonPressMask | VisibilityChangeMask |
		EnterWindowMask;
	XSetStandardProperties(dpy, mywindow, tmpbuf, tmpbuf, None,
	       argv, argc, &myhint);
	gcvals.foreground = mybackground;
	gcvals.background = myforeground;
	mygc = XCreateGC(dpy, mywindow, GCForeground | GCBackground,
		&gcvals);
	font_struct = XLoadQueryFont(dpy,
		"-*-*-medium-r-*-*-*-100-*-*-*-*-*-*");
	if(!font_struct)
	{
		fprintf(stderr, "Missing fonts.\n");
		exit(1);
	}
	XSetFont(dpy, mygc, font_struct->fid);
	mycolor.red = 0xFFFF;
	mycolor.green = 0xFFFF;
	mycolor.blue = 0;
	mycolor.flags = DoRed | DoGreen | DoBlue;
	XAllocColor(dpy, DefaultColormap(dpy, myscreen), &mycolor);
	gcvals.foreground = mycolor.pixel;
	mygcled = XCreateGC(dpy, mywindow, GCForeground |
		GCBackground, &gcvals);
	XSetFont(dpy, mygcled, font_struct->fid);
	mycolor.green = 0;
	XAllocColor(dpy, DefaultColormap(dpy, myscreen), &mycolor);
	gcvals.foreground = mycolor.pixel;
	mygcred = XCreateGC(dpy, mywindow, GCForeground |
		GCBackground, &gcvals);
	XSetFont(dpy, mygcred, font_struct->fid);

	gcvals.foreground = mybackground;
	XMapRaised(dpy, mywindow);
}

Pixmap scale_pic(struct scaledata *p, int kw, int kh, int num)
{
	Pixmap px;
	int x, y, w, h, pel, bpp, sx, sy, o, r, g, b;
	XImage *myimage;

	w = kw * sn / sd;
	h = kh * sn / sd;

	px = XCreatePixmap(dpy, mywindow, w, h,
		DefaultDepth(dpy, myscreen));
	if(!(myimage = XGetImage(dpy, px,
	   0, 0, w, h, AllPlanes, ZPixmap))) {
		fprintf(stderr, "XGetImage failed for num %d {%d, %d}\n", num, kw, kh);
		exit(1);
	}
	/* 
	   to not resize
	   sx = sy = 256;
	 */
	sx = p->pgm->width * 256 / w;
	sy = p->pgm->height * 256 / h; 
	bpp = myimage->bits_per_pixel / 8;

	for (y = 0; y < h; y++)
		for (x = 0; x < w; x++) {
			o = myimage->bytes_per_line * y + x * bpp;
			pel = p->pgm->data[((sy * y)>>8) * p->pgm->width +
				((sx * x)>>8)];
			if (getenv("COLORKEYS") &&
			    (!num || num == 6 || num == 9 || num == 11)) {
				r = pel * 9 / 8;
				g = pel * 9 / 8;
				b = pel * 11 / 8;
				if (b > 255)
					b = 255;
				if (r > 255)
					r = 255;
				if (g > 255)
					g = 255;
			} else {
				r = g = b = pel;
			}
			if (myimage->depth == 16) {
				pel = ((r<<8)&63488)|((g<<3)&2016)|
					((b>>3)&31);
				myimage->data[o] = pel;
				myimage->data[o + 1] = (pel >> 8);
			} else if (myimage->depth == 15) {
				pel = ((r<<7)&31744)|((g<<2)&992)|
					((b>>3)&31);
				myimage->data[o] = pel;
				myimage->data[o + 1] = (pel >> 8);
			} else if (myimage->depth >= 24 ) {	/* 24/32 bpp */
				myimage->data[o] = r;
				myimage->data[o + 1] = g;
				myimage->data[o + 2] = b;
			} else {	/* 8bpp */
				myimage->data[o] = pel;
			}
	}
	XPutImage(dpy, px, mygc, myimage, 0, 0, 0, 0, w, h);
	XDestroyImage(myimage);
	return px;
}

void create_pixmaps(void)
{
	static int initialized = 0;
	int i;

	if (initialized) {
		for (i = 0; i < NUM_KEYCAPS; i++) {
			XFreePixmap(dpy, mykc[i]);
		}
	}
	else
	{
		for (i = 0; i < NUM_CONSTPGM; i++) {
			if(cpgmlist[i].pgm)
			{
				cpgm[i] = scale_pic(
						&cpgmlist[i], 
						cpgmlist[i].w,
						cpgmlist[i].h, 
						i);
			}
		}
	}
	for (i = 0; i < NUM_KEYCAPS; i++) {
		if(mykclist[i].pgm)
		{
			mykc[i] = scale_pic(
					&mykclist[i], 
					mykclist[i].w,
					mykclist[i].h, 
					i);
		}
	}
	initialized = 1;
}

void move_constkeys()
{
	XMoveWindow(dpy, zoomwin, myhint.width - 45, 16);
	XMoveWindow(dpy, modewin, myhint.width - 85, 16);
	XMoveWindow(dpy, movewin, myhint.width - 125, 16);
	XMoveWindow(dpy, mousewin, myhint.width - 165, 16);
	XMoveWindow(dpy, lbcornerwin, myhint.width - 16, myhint.height - 16);
	XMoveWindow(dpy, closewindow, 0, myhint.height - closewindow_uppgm.height);
}

void create_constkeys()
{
	zoomwin = XCreateSimpleWindow(dpy, mywindow,
		myhint.width - 45, 16, 40, 40, 1, mybackground, myforeground);
	modewin = XCreateSimpleWindow(dpy, mywindow,
		myhint.width - 85, 16, 40, 40, 1, mybackground, myforeground);
	movewin = XCreateSimpleWindow(dpy, mywindow,
		myhint.width - 125, 16, 40, 40, 1, mybackground, myforeground);
	mousewin = XCreateSimpleWindow(dpy, mywindow,
		myhint.width - 165, 16, 40, 40, 1, mybackground, myforeground);

	lbcornerwin = XCreateSimpleWindow(dpy, mywindow,
		myhint.width-16, myhint.height-16, 16, 16, 0, mybackground, myforeground);
	closewindow = XCreateSimpleWindow(dpy, mywindow,
		0, myhint.height - closewindow_uppgm.height, 16, 16, 0, mybackground, myforeground);

	XSetWindowBackgroundPixmap(dpy, mywindow, cpgm[NUM_CONSTPGM - 1]);
	XClearWindow(dpy, mywindow);

	XSetWindowBackgroundPixmap(dpy, zoomwin, cpgm[NUM_CONSTPGM - 3]);
	XClearWindow(dpy, zoomwin);
	XSelectInput(dpy, zoomwin, ButtonPressMask | ButtonReleaseMask);
	XMapRaised(dpy, zoomwin);

	XSetWindowBackgroundPixmap(dpy, modewin, cpgm[NUM_CONSTPGM - 12]);
	XClearWindow(dpy, modewin);
	XSelectInput(dpy, modewin, ButtonPressMask | ButtonReleaseMask);
	XMapRaised(dpy, modewin);

	XSetWindowBackgroundPixmap(dpy, mousewin, cpgm[NUM_CONSTPGM - 14]);
	XClearWindow(dpy, mousewin);
	XSelectInput(dpy, mousewin, ButtonPressMask | ButtonReleaseMask);
	XMapRaised(dpy, mousewin);

	XSetWindowBackgroundPixmap(dpy, movewin, cpgm[NUM_CONSTPGM - 9]);
	XClearWindow(dpy, movewin);
	XSelectInput(dpy, movewin, ButtonPressMask | ButtonReleaseMask);
	XMapRaised(dpy, movewin);

	XSetWindowBackgroundPixmap(dpy, lbcornerwin, cpgm[NUM_CONSTPGM - 11]);
	XClearWindow(dpy, lbcornerwin);
	XSelectInput(dpy, lbcornerwin, ButtonPressMask | ButtonReleaseMask);
	XMapRaised(dpy, lbcornerwin);

	XSetWindowBackgroundPixmap(dpy, closewindow, cpgm[NUM_CONSTPGM - 5]);
	XClearWindow(dpy, closewindow);
	XSelectInput(dpy, closewindow, ButtonPressMask);
	XMapRaised(dpy, closewindow);
}

void create_keys(void)
{
	int i, j, w, h;

	create_pixmaps();
	for (i = 0; i < MAX_KEYS; i++) {
		mykey[i] = XCreateSimpleWindow(dpy, mywindow,
			(ox + xk[i].x) * sn / sd, (oy + xk[i].y) * sn / sd,
			xk[i].w * sn / sd, xk[i].h * sn / sd,
			BORDER, mybackground, mybackground);
		XSetWindowBackgroundPixmap(dpy, mykey[i], mykc[xk[i].p]);
		XMapRaised(dpy, mykey[i]);
		XClearArea(dpy, mykey[i], 0, 0, 0, 0, True);
		XSelectInput(dpy, mykey[i], ButtonPressMask |
			ButtonReleaseMask | ExposureMask);
	}
	for (i = 0; i < NUM_BORDERS; i++) {
		w = mb[i].w * sn / sd;
		h = mb[i].h * sn / sd;
		if (!w)
			w = 4;
		if (!h)
			h = 4;
		myborder[i] = XCreateSimpleWindow(dpy, mywindow,
			(ox + mb[i].x) * sn / sd, (oy + mb[i].y) * sn / sd,
			w, h, 0, mybackground, mybackground);
		XMapRaised(dpy, myborder[i]);
	}
	keystate = 0;
	for (j = 0; j < 8; j++) {
		for (i = 0; i < 256; i++)
			scanname[j][i] = NULL;
		modindex[j] = 0;
	}
	create_constkeys();
}

int get_keyboard_mapping(void)
{
	int i, j;
	int min_keycode, max_keycode, keysyms_per_keycode, keycode_count;
	XModifierKeymap *map;
	KeySym *keymap;
	char *tmp, *s;

	for (j = 0; j < 8; j++)
		for (i = 0; i < 16; i++)
			modifiermap[j][i] = 0;
	map = XGetModifierMapping(dpy);
	keycode_count = map->max_keypermod;
	for (i = 0; i < 8; i++) {
		for (j = 0; j < keycode_count && j < 16; j++) {
			modifiermap[i][j] =
				map->modifiermap[i * keycode_count + j];
		}
	}

	for (j = 0; j < 8; j++)
		for (i = 0; i < 256; i++)
			if (scanname[j][i] != NULL) {
				free(scanname[j][i]);
				scanname[j][i] = NULL;
			}
	XDisplayKeycodes(dpy, &min_keycode, &max_keycode);
	keycode_count = (max_keycode - min_keycode) + 1;
	if (!(keymap = XGetKeyboardMapping(dpy, min_keycode,
		keycode_count, &keysyms_per_keycode))) {
		fprintf(stderr, "Unable to get keyboard mapping table\n");
		return 1;
	}
	myevent.type = KeyPress;
	myevent.xkey.display = dpy;
	myevent.xkey.window = mywindow;
	myevent.xkey.same_screen = True;
	for (j = 0; j < keycode_count; j++) {
		for (i = 0; i < keysyms_per_keycode; i++) {
			KeySym ks = keymap[j * keysyms_per_keycode + i];
			if (ks != NoSymbol) {
				if ((s = XKeysymToString(ks)) != NULL) {
					tmp = malloc(16);
					memcpy(tmp, s, 16);
					tmp[15] = 0;
					scanname[i][j + min_keycode] = tmp;
					myevent.xkey.keycode = j + min_keycode;
					myevent.xkey.state = i & 3;
					XLookupString((XKeyEvent *) &myevent,
						tmpbuf, 16, &ks, NULL);
					if (tmpbuf[0] >= ' ' &&
					    tmpbuf[0] != 0x7f)
						memcpy(tmp, tmpbuf, 16);
					if ((tmp = strchr(&tmp[1], '_'))!=NULL)
						*tmp = 0;
				}
			}
		}
	}
	XFree((char *) keymap);
	return 0;
}

void redraw_window(Window exposed)
{
	int i, j, mod = 0, s;
	GC keygc;
	char *name;

	for (i = 0; i < MAX_KEYS; i++) {
		if (exposed == mykey[i]) {
			break;
		}
	}
	if (i == MAX_KEYS) {
		return;
	}
	s = xk[i].scancode;
	for (j = 0; j < 8; j++) {
		if (modifiermap[j][0] == s)
			break;
		if (modifiermap[j][1] == s)
			break;
		if (modifiermap[j][2] == s)
			break;
		if (modifiermap[j][3] == s)
			break;
	}
	keygc = mygc;
	if (j < 8) {
		if ((keystate & (1<< j)) && modindex[j] == i)
			keygc = mygcled;
	}
	for (mod = 0; mod < 4; mod++) {
		name = scanname[mod][s];
		if (!mod && !name && s == 94) {
			XUnmapWindow(dpy, mykey[i]);
		} else {
			if (!name)
				continue;
			if (islower(*name))
				continue;
			if (!mod && s == 94)
				XMapWindow(dpy, mykey[i]);
			if (name[0] == 'K' && name[1] == 'P')
				name += 3;
			int len = strlen(name);
			int dummy[3];
			XCharStruct schar;
redo:
			XQueryTextExtents(dpy, font_struct->fid, name, len,
					&dummy[0], &dummy[1], &dummy[2], &schar);
			if(schar.width > xk[i].w * sn / sd)
			{
				len--;
				goto redo;
			}
			XDrawString(dpy, mykey[i], keygc,
				8 * sn / sd + 8 * (mod & 2),
				26 * sn / sd - (11 * (mod & 1)) * sn / sd,
				name, len);
			break;
		}
	}
}

void resize_keys(void)
{
	int i, w, h;

	create_pixmaps();
	for (i = 0; i < MAX_KEYS; i++) {
		XMoveResizeWindow(dpy, mykey[i],
			(ox + xk[i].x) * sn / sd, (oy + xk[i].y) * sn / sd,
			xk[i].w * sn / sd, xk[i].h * sn / sd);
		XSetWindowBackgroundPixmap(dpy, mykey[i], mykc[xk[i].p]);
		/* generate a new expose event for window */
		XClearArea(dpy, mykey[i], 0, 0, 0, 0, True);
	}
	for (i = 0; i < NUM_BORDERS; i++) {
		w = mb[i].w * sn / sd;
		h = mb[i].h * sn / sd;
		if (!w)
			w = 4;
		if (!h)
			h = 4;
		XMoveResizeWindow(dpy, myborder[i],
			(ox + mb[i].x) * sn / sd, (oy + mb[i].y) * sn / sd,
			w, h);
	}
	XSetWindowBackgroundPixmap(dpy, zoomwin, cpgm[NUM_CONSTPGM - 3]);
	XSetWindowBackgroundPixmap(dpy, mywindow, cpgm[NUM_CONSTPGM - 1]);
	XClearWindow(dpy, mywindow);
	XClearWindow(dpy, zoomwin);
}

void do_control(void)
{
	XFontStruct *font;
	zoom &= 3;

	if (zoom == 0) {
		font = XLoadQueryFont(dpy,
			"-*-*-medium-r-*-*-*-100-*-*-*-*-*-*");
		if(!font)
		{
			return;
		}
		ox = 32; oy = 16; sn = 1; sd = 1;
		XFreeFont(dpy, font_struct);
		font_struct = font;
		XSetFont(dpy, mygc, font_struct->fid);
		XSetFont(dpy, mygcled, font_struct->fid);
	} else if (zoom == 1) {
		font = XLoadQueryFont(dpy,
			"-*-*-medium-r-*-*-*-120-*-*-*-*-*-*");
		if(!font)
		{
			return;
		}
		ox = 0; oy = -48; sn = 3; sd = 2;
		XFreeFont(dpy, font_struct);
		font_struct = font;
		XSetFont(dpy, mygc, font_struct->fid);
		XSetFont(dpy, mygcled, font_struct->fid);
	} else if (zoom == 3) {
		font = XLoadQueryFont(dpy,
			"-*-*-medium-r-*-*-*-180-*-*-*-*-*-*");
		if(!font)
		{
			return;
		}
		ox = -48; oy = -64; sn = 2; sd = 1;
		XFreeFont(dpy, font_struct);
		font_struct = font;
		XSetFont(dpy, mygc, font_struct->fid);
		XSetFont(dpy, mygcled, font_struct->fid);
	} else if (zoom == 2) {
		font = XLoadQueryFont(dpy,
			"-*-*-medium-r-*-*-*-120-*-*-*-*-*-*");
		if(!font)
		{
			return;
		}
		ox = -12; oy = -64; sn = 7; sd = 4;
		XFreeFont(dpy, font_struct);
		font_struct = font;
		XSetFont(dpy, mygc, font_struct->fid);
		XSetFont(dpy, mygcled, font_struct->fid);
	}
	resize_keys();
}

void do_button_event(void)
{
	int i, j, mod, sc, lockmap = 0;
	char *name;
	static Window lastkeywin = 0;
	static Window lastmodkeywin = 0;
	Window win = myevent.xbutton.window;
	static Time laststamp = 0;
	Time stamp = myevent.xbutton.time;

	if (lastkeywin && myevent.type == ButtonRelease) {
		XSetWindowBorderWidth(dpy, lastkeywin, BORDER);
		win = lastkeywin;
		lastkeywin = 0;
	}
	if (lastkeywin) {	/* sanity */
		XSetWindowBorderWidth(dpy, lastkeywin, BORDER);
		lastkeywin = 0;
	}
	for (i = 0; i < MAX_KEYS; i++) {
		if (win == mykey[i])
			break;
	}
	if (i == MAX_KEYS) {
		/* unexpected event window */
		return;
	}
	sc = xk[i].scancode;
	for (j = 0; j < 8; j++) {
		if (modifiermap[j][0] == sc)
			break;
		if (modifiermap[j][1] == sc)
			break;
		if (modifiermap[j][2] == sc)
			break;
		if (modifiermap[j][3] == sc)
			break;
	}
	mod = 0;
	if (j < 8) {
		if (myevent.type == ButtonRelease)
			return;
		if(win == lastmodkeywin && stamp - laststamp < 500)
		{
			lockmod = !lockmod ? 1 : 0;
			GC tmp = mygcled;
			mygcled = mygcred;
			mygcred = tmp;
			if(lockmod)
			{
				name = scanname[0][sc];
				if(name)
					XDrawString(dpy, mykey[i], mygcled,
							8 * sn / sd, 26 * sn / sd,
							name, strlen(name));
			}
			do_control();
			return;
		} 
		laststamp = stamp;
		lastmodkeywin = mykey[i];
		mod = (1 << j);
		if (keystate & mod) {
                       keystate &= ~mod;
                       lockmap |= mod;
                       myevent.type = ButtonRelease;
                       i = modindex[j];
                       sc = xk[i].scancode;
                       name = scanname[0][sc];
                       if (name)
                               XDrawString(dpy, mykey[i], mygc,
                                       8 * sn / sd, 26 * sn / sd,
                                       name, strlen(name));
		} else { 
			keystate |= mod; 
			lockmap |= mod;
			modindex[j] = i;
			name = scanname[0][sc];
			if (name)
			{
				XDrawString(dpy, mykey[i], mygcled,
						8 * sn / sd, 26 * sn / sd,
						name, strlen(name));
			}
		}
	}
	if (myevent.type == ButtonPress) {
		if (!mod)
			lastkeywin = mykey[i];
		myevent.type = KeyPress;
		XSetWindowBorderWidth(dpy, mykey[i], 2);
	} else {
		myevent.type = KeyRelease;
		XSetWindowBorderWidth(dpy, mykey[i], BORDER);
	}
	if ((lockmap & 0x92) && myevent.type == KeyRelease)
		myevent.type = KeyPress;

	if(sendkey)
	{
		int revert;
		Window focus;
		int statemask = 0;

		if (keystate & 0x01) { 	/* shift */
			statemask |= ShiftMask;
		}
		if (keystate & 0x02) { 	/* caps */
			statemask |= LockMask;
		}
		if (keystate & 0x04) {	/* control */
			statemask |= ControlMask;
		}
		if (keystate & 0x08) { 	/* alt */
			statemask |= Mod1Mask;
		}
		if (keystate & 0x0f) {
			statemask |= Mod2Mask;
		}
		if (keystate & 0x20) { 
			statemask |= Mod3Mask;
		}
		if (keystate & 0x40) { 	/* win key */
			statemask |= Mod4Mask;
		}
		if (keystate & 0x80) { 	
			statemask |= Mod5Mask;
		}

		XGetInputFocus(dpy, &focus, &revert);

		XEvent event;
		event.xkey.type = myevent.type;
		event.xkey.window = focus;
		event.xkey.root = event.xkey.subwindow = None;
		event.xkey.time = 0;
		event.xkey.send_event = 1;
		event.xkey.keycode = sc;
		event.xkey.state = statemask;
		XSendEvent(dpy, focus, True, KeyPressMask | KeyReleaseMask, &event);
		XSync(dpy, 1);
	}
	else
	{
		XTestFakeKeyEvent(dpy, sc, (myevent.type == KeyPress), 0);
	}
	if (myevent.type == KeyPress && !(mod)) {
		/* automagically turn off some modifiers */
		if(!lockmod)
		{
			if (keystate & 0x01) { 	/* shift */
				i = modindex[0];
				sc = xk[i].scancode;
				XSetWindowBorderWidth(dpy, mykey[i], BORDER);
				XTestFakeKeyEvent(dpy, sc, 0, 0);
				name = scanname[0][sc];
				if (name)
					XDrawString(dpy, mykey[i], mygc,
							8 * sn / sd, 26 * sn / sd,
							name, strlen(name));
			}
			if (keystate & 0x04) {	/* control */
				i = modindex[2];
				sc = xk[i].scancode;
				XSetWindowBorderWidth(dpy, mykey[i], BORDER);
				XTestFakeKeyEvent(dpy, sc, 0, 0);
				name = scanname[0][sc];
				if (name)
					XDrawString(dpy, mykey[i], mygc,
							8 * sn / sd, 26 * sn / sd,
							name, strlen(name));
			}
			if (keystate & 0x08) { 	/* alt */
				i = modindex[3];
				sc = xk[i].scancode;
				XSetWindowBorderWidth(dpy, mykey[i], BORDER);
				XTestFakeKeyEvent(dpy, sc, 0, 0);
				name = scanname[0][sc];
				if (name)
					XDrawString(dpy, mykey[i], mygc,
							8 * sn / sd, 26 * sn / sd,
							name, strlen(name));
			}
			keystate &= ~0x0d;
		}
	}
	if (lockmap & 0x92) {	/* Need to generate keyup events */
		if (lockmap & 0x02) {	/* Lock */
			XTestFakeKeyEvent(dpy,
				xk[modindex[1]].scancode, 0, 0);
		}
		if (lockmap & 0x10) {	/* Mod1 */
			XTestFakeKeyEvent(dpy,
				xk[modindex[4]].scancode, 0, 0);
		}
		if (lockmap & 0x80) {	/* Mod5 */
			XTestFakeKeyEvent(dpy,
				xk[modindex[7]].scancode, 0, 0);
		}
	}
	XFlush(dpy);
	return;
}

void hide()
{
	XMoveWindow(dpy, closewindow, 0, 0);
	XMoveResizeWindow(dpy, mywindow, 0, screenheight-16, 16, 16);
}

void show()
{
	if(myhint.y + myhint.height > screenheight)
	{
		myhint.y -= myhint.height-closewindow_uppgm.height;
	}
	XMoveResizeWindow(dpy, mywindow, myhint.x, myhint.y, myhint.width, myhint.height);
	move_constkeys();
}

int
getrootptr(int *x, int *y)
{
	int di;
	unsigned int dui;
	Window dummy;

	return XQueryPointer(dpy, root, &dummy, &dummy, x, y, &di, &di, &dui);
}

/* position relative to the background window */
void
movebutton(int xpos, int ypos)
{
	int x, y, nx = 0, ny = 0;
	XEvent ev;
	Time lasttime = 0;

	XSetWindowBackgroundPixmap(dpy,
			movewin,
			cpgm[NUM_CONSTPGM - 10]);
	XClearWindow(dpy, movewin);

	if (XGrabPointer(dpy, root, False, MOUSEMASK, GrabModeAsync, GrabModeAsync,
		None, None, CurrentTime) != GrabSuccess)
		return;
	if (!getrootptr(&x, &y))
		return;
	do {
		XMaskEvent(dpy, MOUSEMASK|ExposureMask|SubstructureRedirectMask, &ev);
		switch(ev.type) {
		case ConfigureRequest:
		case Expose:
		case MapRequest:
			break;
		case MotionNotify:
			if ((ev.xmotion.time - lasttime) <= (1000 / 60))
				continue;
			lasttime = ev.xmotion.time;
			nx = ev.xmotion.x;
			ny = ev.xmotion.y;
			XMoveWindow(dpy, mywindow, nx-xpos, ny-ypos);
			break;
		}
	} while (ev.type != ButtonRelease);
	myhint.x = nx-xpos;
	myhint.y = ny-ypos;
	XSetWindowBackgroundPixmap(dpy,
			movewin,
			cpgm[NUM_CONSTPGM - 9]);
	XClearWindow(dpy, movewin);
	do_control(); //redraw the fonts
	XUngrabPointer(dpy, CurrentTime);
}

void resize_win()
{
	int x, y, nx, ny = 0; 
	XEvent ev;
	Time lasttime = 0;

	if (XGrabPointer(dpy, root, False, MOUSEMASK, GrabModeAsync, GrabModeAsync,
		None, None, CurrentTime) != GrabSuccess)
	if (!getrootptr(&x, &y))
		return;
	do {
		XMaskEvent(dpy, MOUSEMASK|ExposureMask|SubstructureRedirectMask, &ev);
		switch(ev.type) {
		case ConfigureRequest:
		case Expose:
		case MapRequest:
			break;
		case MotionNotify:
			if ((ev.xmotion.time - lasttime) <= (1000 / 60))
				continue;
			lasttime = ev.xmotion.time;
			nx = ev.xmotion.x;
			ny = ev.xmotion.y;
			myhint.width = nx - myhint.x;
			myhint.height = ny - myhint.y;
			XMoveResizeWindow(dpy, mywindow, myhint.x, myhint.y, myhint.width, myhint.height);
			move_constkeys();
			break;
		}
	} while (ev.type != ButtonRelease);
	do_control(); //redraw the fonts
	XUngrabPointer(dpy, CurrentTime);
}

void reset_keyspos()
{
	int i;
	for (i = 0; i < MAX_KEYS; i++) {
		xk[i].x -= xkeyoffs; 
		xk[i].y -= ykeyoffs;
	}
	for (i = 0; i < NUM_BORDERS; i++) {
		mb[i].x	-= xkeyoffs;
		mb[i].y -= ykeyoffs;	
	}
	xkeyoffs = 0;
	ykeyoffs = 0;
}

void move_keys()
{
	int x, y, nx = 0, ny = 0, i; 
	XEvent ev;
	Time lasttime = 0;

	if (XGrabPointer(dpy, root, False, MOUSEMASK, GrabModeAsync, GrabModeAsync,
		None, None, CurrentTime) != GrabSuccess)
		return;
	if (!getrootptr(&x, &y))
		return;
	do {
		XMaskEvent(dpy, MOUSEMASK|ExposureMask|SubstructureRedirectMask, &ev);
		switch(ev.type) {
		case ConfigureRequest:
		case Expose:
		case MapRequest:
			break;
		case MotionNotify:
			if ((ev.xmotion.time - lasttime) <= (1000 / 60))
				continue;
			lasttime = ev.xmotion.time;
			nx = (ev.xmotion.x - x);
			ny = (ev.xmotion.y - y);

			for (i = 0; i < MAX_KEYS; i++) {
				XMoveWindow(dpy, mykey[i], 
				(ox+(xk[i].x+nx))*sn/sd, (oy+(xk[i].y+ny))*sn/sd); 
			}
			for (i = 0; i < NUM_BORDERS; i++) {
				XMoveWindow(dpy, myborder[i],
				(ox+(mb[i].x+nx))*sn/sd, (oy+(mb[i].y+ny))*sn/sd); 
			}
			break;
		}
	} while (ev.type != ButtonRelease);
	XUngrabPointer(dpy, CurrentTime);
	xkeyoffs += nx;
	ykeyoffs += ny;
	for (i = 0; i < MAX_KEYS; i++) {
		xk[i].x += nx;
		xk[i].y += ny;
	}
	for (i = 0; i < NUM_BORDERS; i++) {
		mb[i].x	+= nx;
		mb[i].y += ny;	
	}
	do_control(); 
	return;
}

void wait_btn_press()
{
	XEvent ev;
	if (XGrabPointer(dpy, root, False, MOUSEMASK, GrabModeAsync, GrabModeAsync,
		None, None, CurrentTime) != GrabSuccess)
		return;
	do {
		XMaskEvent(dpy, MOUSEMASK|ExposureMask|SubstructureRedirectMask, &ev);
	} while (ev.type != ButtonPress);
	XUngrabPointer(dpy, CurrentTime);
}

void wait_btn_release()
{
	XEvent ev;
	if (XGrabPointer(dpy, root, False, MOUSEMASK, GrabModeAsync, GrabModeAsync,
		None, None, CurrentTime) != GrabSuccess)
		return;
	do {
		XMaskEvent(dpy, MOUSEMASK|ExposureMask|SubstructureRedirectMask, &ev);
	} while (ev.type != ButtonRelease);
	XUngrabPointer(dpy, CurrentTime);
}

int doubleclick(unsigned int ms)
{
	static Time laststamp = 0;
	Time stamp = myevent.xbutton.time;
	if(stamp - laststamp < ms)
	{
		return 1;
	} 
	laststamp = stamp;
	return 0;
}

void btn_press(Display *display, int button)
{
	// Create and setting up the event
	XEvent event;
	memset (&event, 0, sizeof (event));
	event.xbutton.button = button;
	event.xbutton.same_screen = True;
	event.xbutton.subwindow = DefaultRootWindow (display);
	while (event.xbutton.subwindow)
	{
		event.xbutton.window = event.xbutton.subwindow;
		XQueryPointer (display, event.xbutton.window,
				&event.xbutton.root, &event.xbutton.subwindow,
				&event.xbutton.x_root, &event.xbutton.y_root,
				&event.xbutton.x, &event.xbutton.y,
				&event.xbutton.state);
	}
	// Press
	if(event.xbutton.window != mousewin)
	{
		if(sendkey)
		{
			event.type = ButtonPress;
			if (XSendEvent (display, PointerWindow, True, ButtonPressMask, &event) == 0)
				fprintf (stderr, "Error to send the event!\n");
			XFlush (display);
		} else {
			XTestFakeButtonEvent(dpy, button, True, CurrentTime);
		}
	}
}

void btn_release(Display *display, int button)
{
	// Create and setting up the event
	XEvent event;
	memset (&event, 0, sizeof (event));
	event.xbutton.button = button;
	event.xbutton.same_screen = True;
	event.xbutton.subwindow = DefaultRootWindow (display);
	while (event.xbutton.subwindow)
	{
		event.xbutton.window = event.xbutton.subwindow;
		XQueryPointer (display, event.xbutton.window,
				&event.xbutton.root, &event.xbutton.subwindow,
				&event.xbutton.x_root, &event.xbutton.y_root,
				&event.xbutton.x, &event.xbutton.y,
				&event.xbutton.state);
	}
	if(event.xbutton.window != mousewin)
	{
		if(sendkey)
		{
			event.type = ButtonRelease;
			if (XSendEvent (display, PointerWindow, True, ButtonReleaseMask, &event) == 0)
				fprintf (stderr, "Error to send the event!\n");
			XFlush (display);
		} else {
			XTestFakeButtonEvent(dpy, button, False, CurrentTime);
		}
	}
}

int main(int argc, char **argv)
{
	int done = 0;
	int keyboard_active = 0;
	int rightclick = 0;

	init_display(argc, argv);
	ox = 32; oy = 16; sn = 1; sd = 1;
	create_keys();
	closingstate = NORMAL;
	mycursor = XCreateFontCursor(dpy, XC_hand2);
	closecursor = XCreateFontCursor(dpy, XC_pirate);
	if (get_keyboard_mapping())
		done++;
	/* Main event loop */
	while (!done) {
		XNextEvent(dpy, &myevent);
		switch (myevent.type) {
			case EnterNotify:
				break;
			case ButtonPress:
				if(myevent.xbutton.window == mywindow)
				{
					if(doubleclick(500))
					{
						keyboard_active ^= keyboard_active;
						hide();
						break;
					}
					move_keys();
					break;
				}
				if (myevent.xbutton.window == closewindow) {
					if(keyboard_active)
					{
						myhint.width = 800;
						myhint.height = 240;
						myhint.x = 0;
						myhint.y = screenheight - 240;
						reset_keyspos();
						zoom ^= zoom; do_control();//set zoom to default.
					}
					keyboard_active = 1;
					show();
					break;
				}
				if (myevent.xbutton.window == zoomwin) {
					XSetWindowBackgroundPixmap(dpy,
						zoomwin,
						cpgm[NUM_CONSTPGM - 4]);
					XClearWindow(dpy, zoomwin);
					break;
				}
				if (myevent.xbutton.window == modewin) {
					if(doubleclick(500))
					{
						rightclick = !rightclick;
					}
					if(!sendkey)
					{
						XSetWindowBackgroundPixmap(dpy,
								modewin,
								cpgm[NUM_CONSTPGM - 13]);
						sendkey = 1;
					}
					else
					{
						XSetWindowBackgroundPixmap(dpy,
								modewin,
								cpgm[NUM_CONSTPGM - 12]);
						sendkey = 0;
					}
					XClearWindow(dpy, modewin);
					break;	
				}
				if (myevent.xbutton.window == movewin) {
					int mx = myevent.xbutton.x;
					int my = myevent.xbutton.y;
					movebutton((myhint.width - 125)+mx, 16 + my);
					break;
				}
				if (myevent.xbutton.window == mousewin) {
					XSetWindowBackgroundPixmap(dpy,
							mousewin,
							cpgm[NUM_CONSTPGM - 15]);
					XClearWindow(dpy, mousewin);
					wait_btn_press();
					btn_press(dpy, rightclick ? Button3 : Button1);
					wait_btn_release();
					btn_release(dpy, rightclick ? Button3 : Button1);
					XSetWindowBackgroundPixmap(dpy,
							mousewin,
							cpgm[NUM_CONSTPGM - 14]);
					XClearWindow(dpy, mousewin);
					break;
				}
				if (myevent.xbutton.window == lbcornerwin) {
				 	resize_win();	
					break;
				}
			case ButtonRelease:
				if (myevent.xbutton.window == zoomwin) {
					zoom++;
					do_control();
					break;
				}
				do_button_event();
				break;
			case Expose:
				if (!myevent.xexpose.count)
					redraw_window(myevent.xexpose.window);
				break;
			case VisibilityNotify:
				XRaiseWindow(dpy, mywindow);
				break;
			case UnmapNotify:
				break;
			case CirculateNotify:
				break;
			case ConfigureNotify:
				break;
			case CreateNotify:
				break;
			case MapNotify:
				break;
			case ReparentNotify:
				break;
			case DestroyNotify:
				fprintf(stderr, "WM Terminated\n");
				done++;
				break;
			case MappingNotify:
				XRefreshKeyboardMapping(
					(XMappingEvent *) &myevent);
				if (get_keyboard_mapping())
					done++;
				redraw_window(mywindow);
				break;
		}
	}
	XFreeGC(dpy, mygc);
	XDestroyWindow(dpy, mywindow);
	XCloseDisplay(dpy);
	exit(0);
}
