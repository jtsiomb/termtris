/*
Termtris - a tetris game for ANSI/VT100 terminals
Copyright (C) 2019-2026  John Tsiombikas <nuclear@member.fsf.org>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#ifndef JOY_H_
#define JOY_H_

#include "inttypes.h"

enum {
	JOY_LEFT	= 0x01,
	JOY_RIGHT	= 0x02,
	JOY_UP		= 0x04,
	JOY_DOWN	= 0x08,
	JOY_BN0		= 0x10,
	JOY_BN1		= 0x20,
	JOY_BN2		= 0x40,
	JOY_BN3		= 0x80
};

#define JOY_BN_ANY	0xfff0

extern int have_joy;
extern unsigned int joy_bnstate, joy_bndiff, joy_bnpress;
extern int16_t joy_pos[2];

int joy_detect(void);
void joy_update(void);
void joy_keyemu(void);

#ifdef MSDOS
extern int cal_min[2], cal_cent[2], cal_max[2];
extern int rawcnt[2];
#endif

#endif	/* JOY_H_ */
