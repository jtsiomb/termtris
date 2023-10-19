/*
Termtris - a tetris game for ANSI/VT100 terminals
Copyright (C) 2019-2022  John Tsiombikas <nuclear@member.fsf.org>

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
#ifndef ANSI_H_
#define ANSI_H_

enum {
	G_DIAMOND	= 0x04,
	G_CHECKER	= 0xb1,
	G_LR_CORNER	= 0xd9,
	G_UR_CORNER	= 0xbf,
	G_UL_CORNER	= 0xda,
	G_LL_CORNER	= 0xc0,
	G_CROSS		= 0xc5,
	G_HLINE		= 0xc4,
	G_L_TEE		= 0xc3,
	G_R_TEE		= 0xb4,
	G_B_TEE		= 0xc1,
	G_T_TEE		= 0xc2,
	G_VLINE		= 0xb3,
	G_CDOT		= 0xf8
};

enum { BLACK, BLUE, GREEN, CYAN, RED, MAGENTA, YELLOW, WHITE };
#define BOLD	8

void ansi_init(void);

void ansi_reset(void);
void ansi_clearscr(void);

void ansi_setcursor(int row, int col);
void ansi_cursor(int show);

void ansi_setcolor(int fg, int bg);

/* convert a PC cga/ega/vga char+attr to an ANSI sequence and write it to stdout */
void ansi_ibmchar(unsigned char c, unsigned char attr);
void ansi_putstr(const char *s, unsigned char attr);

#endif	/* ANSI_H_ */
