Termtris - tetris game for ANSI/VT220 terminals
===============================================

![shots](http://nuclear.mutantstargoat.com/sw/termtris/img/termtris-banner.png)

About
-----
Termtris runs on UNIX systems and requires an ANSI or VT220-compatible terminal
for input and output. It has been tested with a VT420, xterm, rxvt, the Linux
console, and kermit running on MS-DOS over a serial link, but it should work on
any terminal capable of interpreting ANSI escape sequences for cursor
addressing, character set selection, and color attributes.

To run on a terminal other than the controlling terminal, simply specify the
device using the `-t` commandline option.

Termtris also runs on DOS now. It will run on any IBM PC compatible with any
graphics card (MDA, CGA, EGA, VGA, SVGA).

I always liked the GameBoy tetris best. So I made termtris as close as possible
to that version gameplay-wise.

  - website: http://nuclear.mutantstargoat.com/sw/termtris
  - source repository: https://github.com/jtsiomb/termtris

Controls
--------
  - `A` or *left arrow* moves the block left.
  - `D` or *right arrow* moves the block right.
  - `S` or *down arrow* drops the block faster.
  - `W` or *up arrow* rotates the block.
  - *enter*, *tab*, or `0` drops the block immediately.
  - `P` pauses and unpauses the game.
  - *backspace*, or *delete* starts a new game.
  - `Q` or hitting escape twice, quits immediately.
  - `H` shows or hides the help panel which lists these keybindings.

Additionally on GNU/Linux systems any joystick can be used to control the game.
The first available joystick is used automatically, or you can specify a
joystick device with the `-j` commandline option.

When using a joystick, even axes are mapped to left/right, Odd axes are mapped
to up/down, the first four buttons rotate, and the rest of the buttons pause
and unpause.

There is no way to remap the controls without changing the source code at this
time.

Custom character set
--------------------
For terminals with support for custom loadable character sets, you can instruct
termtris to use graphical blocks instead of the bracket characters with the `-g`
or `-G` command-line arguments.

This is an new experimental feature, and for now `-g` enables the use of
VT220-compatible block replacment characters, which are not very nice, while
`-G` enables the use of larger VT420 block characters which give a nice bevelled
3D look to the blocks. At some point terminal capability detection will be
implemented which will make it possible to use a single switch to enable
graphical blocks, and the size and format will be determined automatically.

License
-------
Copyright (C) 2019-2023 John Tsiombikas <nuclear@mutantstargoat.com>

This program is free software. Feel free to use, modify, and/or redistribute it
under the terms of the GNU General Public License version 3, or at your option,
any later version published by the Free Software Foundation. See COPYING for
details.

Download
--------
Current release (1.7): http://nuclear.mutantstargoat.com/sw/termtris/termtris-1.7.tar.gz

Alternatively you may clone the latest version of the source code from the git
repository:

    git clone https://github.com/jtsiomb/termtris

Pre-compiled binaries might be available in your package management system of
choice, but please do not submit any bug reports without first trying to build
termtris yourself from the latest git source code.
  - Debian/Ubuntu/Mint: `apt-get install termtris`

Build (UNIX)
------------
There are no external dependencies. Simply type `make` to build, and `make
install` if you wish to install termtris.

Default installation prefix is `/usr/local`. Change the first line of the
`Makefile` if you'd rather install somewhere else, or simply invoke the install
rule like so:

    make PREFIX=/some/other/prefix install

Build (DOS)
-----------
To build the DOS version of termtris you'll need some version of the Watcom or
OpenWatcom C compiler, and NASM. Build by typing `make -f Makefile.wat`.
