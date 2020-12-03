
XKEYBOARD
=========

Xkeyboard is a minimal inscreen keyboard that
will let you control everything on your X11 from 
touchscreen.

USAGE
-----

Make sure your system has any font that will
match the size required for the keys. Nothing
will work otherwise. 

I expect this to only work for standard X11 keymap,
however it's not limited to that as the program depends
on what X11 server returns.

The mouse icon button will do a left click by default,
double tap on the 'G' icon will change it to right click.

Double click/tap on any area that are not icons/keys will hide
the keyboard into left bottom corner, into small 16x16 icon.

The icon 'G' will change the input method between XsendEvent
and Xtest. You may need to change between those sometimes because
different apps react differently and may not have the effect you need.

If the key text is missing that means you don't have the font of
the pixel size required.

You can move an resize the keyboard at any time.

Depending on your system, keyboard needs an X11 window to be set
in input mode by some other program, for example your window manager or
desktop enviroment (most likely you already have one of those). 
But by default the input may also be set by X server if you click on the
window.

Keyboard will work and perform any kind of keybind combination, unlike many
other onscreen keyboards who can't do that. In 'G' mode it will be almost like
a real hardware keyboard.

![A Snapshot](snapshot.png?raw=true "Snapshot")
