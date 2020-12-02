#
# Copyright (C) 1999-2000 Transmeta Corporation
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

override CFLAGS		+= -Wall -I./keypics -g
override LDFLAGS	+= -L$(TOOLS)/usr/X11R6/lib -lXtst -lXext -lX11

xkeyboard: xkeyboard.o

xkeyboard.o: xkeyboard.h xkeycaps.h nullkeycaps.h

clean::
	$(RM) xkeyboard *.o $~ core
install: xkeyboard
	cp -f xkeyboard /bin
