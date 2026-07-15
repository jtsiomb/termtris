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
#include <stdio.h>
#include <conio.h>
#include <dos.h>
#include "joy.h"
#include "game.h"

#define MAX_COUNT	2048
#define MIN_COUNT	32

#define REP_START		7
#define REP_TICKS		1

static unsigned int read_joy(int *xret, int *yret);

int have_joy;
unsigned int joy_bnstate, joy_bndiff, joy_bnpress;
int16_t joy_pos[2];

int cal_min[2], cal_cent[2], cal_max[2];
int rawcnt[2];

extern long timer_ticks;	/* in main.c */


int joy_detect(void)
{
	int xcnt, ycnt;

	read_joy(&xcnt, &ycnt);
	if(!(have_joy = (xcnt < MAX_COUNT || ycnt < MAX_COUNT))) {
		return 0;
	}
	printf("Joystick found!\n");
	return 1;
}

void joy_update(void)
{
	int i;
	long rng;
	unsigned int prev_bnstate;

	prev_bnstate = joy_bnstate;
	joy_bnstate = read_joy(rawcnt, rawcnt + 1);

	for(i=0; i<2; i++) {
		if(rawcnt[i] < MAX_COUNT) {
			if(cal_cent[i] == 0) cal_cent[i] = rawcnt[i];
			if(cal_min[i] == 0 || rawcnt[i] < cal_min[i]) cal_min[i] = rawcnt[i];
			if(rawcnt[i] > cal_max[i]) cal_max[i] = rawcnt[i];

			if(rawcnt[i] < cal_cent[i]) {
				if((rng = cal_cent[i] - cal_min[i])) {
					joy_pos[i] = ((long)(rawcnt[i] - cal_min[i]) << 15) / rng - 0x8000;
				} else {
					joy_pos[i] = 0;
				}
			} else {
				if((rng = cal_max[i] - cal_cent[i] + 1)) {
					joy_pos[i] = ((long)(rawcnt[i] - cal_cent[i]) << 15) / rng;
				} else {
					joy_pos[i] = 0;
				}
			}
		} else {
			joy_pos[i] = 0;
		}

		/* if the range is too short, discard any movement and center */
		rng = cal_max[i] - cal_min[i];
		if(rng < MIN_COUNT) {
			joy_pos[i] = 0;
		}

		if(joy_pos[i] <= -0x4000) {
			joy_bnstate |= JOY_LEFT << (i << 1);
		} else if(joy_pos[i] > 0x4000) {
			joy_bnstate |= JOY_RIGHT << (i << 1);
		}
	}

	joy_bndiff = joy_bnstate ^ prev_bnstate;
	joy_bnpress = joy_bnstate & joy_bndiff;
}

static unsigned int rep[3];

static void repeat(int idx, long tm, char c)
{
	static long last[3];

	if(rep[idx] > 1) {
		if(tm - last[idx] < REP_TICKS) return;
	} else if(rep[idx] > 0) {
		if(tm - last[idx] < REP_START) return;
	}

	switch(idx) {
	case 0:
		rep[1] = rep[2] = 0;
		break;
	case 1:
		rep[0] = rep[2] = 0;
		break;
	case 2:
		rep[0] = rep[1] = 0;
		break;
	}
	rep[idx]++;
	last[idx] = tm;

	game_input(c);
}

void joy_keyemu(void)
{
	long tm;

	if(!have_joy) {
		return;	/* no joy ... */
	}

	_disable();
	tm = timer_ticks;
	_enable();


	if(joy_bnstate & JOY_LEFT) {
		repeat(0, tm, 'a');
	} else {
		rep[0] = 0;
	}
	if(joy_bnstate & JOY_RIGHT) {
		repeat(1, tm, 'd');
	} else {
		rep[1] = 0;
	}
	if(joy_bnstate & JOY_DOWN) {
		repeat(2, tm, 's');
	} else {
		rep[2] = 0;
	}
	if(joy_bnpress & JOY_BN0) {
		game_input('w');
	}
	if(joy_bnpress & (JOY_BN1 | JOY_UP)) {
		game_input('\n');
	}
	if(joy_bnpress & JOY_BN2) {
		game_input('p');
	}
	if(joy_bnpress & JOY_BN3) {
		game_input('\b');
	}
}


#define JOY_PORT	0x201

#ifdef __DJGPP__
#define outp(p, v)	outportb(p, v)
#define inp(p)		inportb(p)
#endif

static unsigned int read_joy(int *xret, int *yret)
{
	int i, pending = 2;
	unsigned char val = 0xff, diff, prev;

	*xret = *yret = MAX_COUNT;

	_disable();

	outp(JOY_PORT, 0xff);
	for(i=0; i<MAX_COUNT; i++) {
		prev = val;
		val = inp(JOY_PORT);
		diff = val ^ prev;

		if(diff & 1) {
			*xret = i;
			if(--pending <= 0) break;
		}
		if(diff & 2) {
			*yret = i;
			if(--pending <= 0) break;
		}
	}

	_enable();

	return ~val & 0xf0;
}
