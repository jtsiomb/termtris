/*
Termtris - a tetris game for ANSI/VT100 terminals
Copyright (C) 2019-2023  John Tsiombikas <nuclear@member.fsf.org>

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
#include <string.h>
#include "term.h"

void vt52_reset(void);
void vt52_clearscr(void);
void vt52_setcursor(int row, int col);
void vt52_cursor(int show);
void vt52_setcolor(int fg, int bg);
void vt52_ibmchar(unsigned char c, unsigned char attr);


static int gmode;


void vt52_init(void)
{
	term_reset = vt52_reset;
	term_clearscr = vt52_clearscr;
	term_setcursor = vt52_setcursor;
	term_cursor = vt52_cursor;
	term_setcolor = vt52_setcolor;
	term_ibmchar = vt52_ibmchar;
}

void vt52_reset(void)
{
	/* make sure we leave the terminal in ASCII mode */
	fputs("\033G", stdout);
	fflush(stdout);
}

void vt52_clearscr(void)
{
	fputs("\033H\033J", stdout);	/* home + erase to end of screen */
}

void vt52_setcursor(int row, int col)
{
	if((row | col) == 0) {
		fputs("\033H", stdout);
	} else {
		printf("\033Y%c%c", row + 32, col + 32);
	}
}

void vt52_cursor(int show)
{
}

void vt52_setcolor(int fg, int bg)
{
}

void vt52_ibmchar(unsigned char c, unsigned char attr)
{
	char cmd[32];
	char *ptr = cmd;

	if(c == G_CHECKER || c == G_HLINE) {
		if(!gmode) {
			gmode = 1;
			memcpy(ptr, "\033F", 2);
			ptr += 2;
		}
	} else {
		if(gmode) {
			gmode = 0;
			memcpy(ptr, "\033G", 2);
			ptr += 2;
		}
	}

	switch(c) {
	case G_CHECKER:
		c = 'a';	/* combined with switch to graphics mode above */
		break;
	case G_HLINE:
		c = 'p';
		break;
	case G_LR_CORNER:
	case G_UR_CORNER:
	case G_UL_CORNER:
	case G_LL_CORNER:
	case G_L_TEE:
	case G_R_TEE:
	case G_B_TEE:
	case G_T_TEE:
		c = '+';
		break;
	case G_CROSS:
		c = 'X';
		break;
	case G_VLINE:
		c = '|';
		break;
	case G_CDOT:
		c = '.';
		break;
	default:
		break;
	}

	*ptr++ = c;
	*ptr = 0;
	fputs(cmd, stdout);
}
