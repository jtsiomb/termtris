Termtris - tetris game for ANSI/VT220 terminals
===============================================

![shot](http://nuclear.mutantstargoat.com/sw/termtris/img/termtris_shot-thumb.png)
![vt420](http://nuclear.mutantstargoat.com/sw/termtris/img/termtris_vt420-thumb.png)

About
-----
Termtris runs on UNIX systems and requires an ANSI or VT220-compatible terminal
for input and output. It has been tested with a VT420, xterm, rxvt, the Linux
console, and kermit running on MS-DOS over a serial link, but it should work on
any terminal capable of interpreting ANSI escape sequences for cursor
addressing, character set selection, and color attributes.

To run on a terminal other than the controlling terminal, simply specify the
device using the `-t` commandline option.

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

Additionally on GNU/Linux systems any joystick can be used to control the game.
The first available joystick is used automatically, or you can specify a
joystick device with the `-j` commandline option.

When using a joystick, even axes are mapped to left/right, Odd axes are mapped
to up/down, the first four buttons rotate, and the rest of the buttons pause
and unpause.

There is no way to remap the controls without changing the source code at this
time.

License
-------
Copyright (C) 2019-2022 John Tsiombikas <nuclear@member.fsf.org>

This program is free software. Feel free to use, modify, and/or redistribute it
under the terms of the GNU General Public License version 3, or at your option,
any later version published by the Free Software Foundation. See COPYING for
details.

Download
--------
Current release (1.5): http://nuclear.mutantstargoat.com/sw/termtris/termtris-1.5.tar.gz

Alternatively you may clone the latest version of the source code from the git
repository:

    git clone https://github.com/jtsiomb/termtris

Pre-compiled binaries might be available in your package management system of
choice, but please do not submit any bug reports without first trying to build
termtris yourself from the latest git source code.
  - Debian/Ubuntu/Mint: `apt-get install termtris`

Build
-----
There are no external dependencies. Simply type `make` to build, and `make
install` if you wish to install termtris.

Default installation prefix is `/usr/local`. Change the first line of the
`Makefile` if you'd rather install somewhere else, or simply invoke the install
rule like so:

    make PREFIX=/some/other/prefix install
