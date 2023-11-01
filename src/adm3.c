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
#include "game.h"
#include "term.h"

void adm3_reset(void);
void adm3_clearscr(void);
void adm3_setcursor(int row, int col);
void adm3_cursor(int show);
void adm3_setcolor(int fg, int bg);
void adm3_ibmchar(unsigned char c, unsigned char attr);

void adm3_init(void)
{
	/* TODO: allow line drawing characters for ADM31 with the appropriate option
	 * ROM installed
	 */
	onlyascii = 1;

	term_type = TERM_ADM3;
	term_reset = adm3_reset;
	term_clearscr = adm3_clearscr;
	term_setcursor = adm3_setcursor;
	term_cursor = adm3_cursor;
	term_setcolor = adm3_setcolor;
	term_ibmchar = adm3_ibmchar;
}

void adm3_reset(void)
{
	adm3_clearscr();
}

void adm3_clearscr(void)
{
	fputc(032, stdout);	/* SUB: clearscr */
}

void adm3_setcursor(int row, int col)
{
	if(!(row | col)) {
		fputc(036, stdout);	/* RS: home */
	} else {
		printf("\033=%c%c", row + 32, col + 32);
	}
}

void adm3_cursor(int show)
{
}

void adm3_setcolor(int fg, int bg)
{
}

void adm3_ibmchar(unsigned char c, unsigned char attr)
{
	fputc(c, stdout);
}
