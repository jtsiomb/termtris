/*
Termtris - a tetris game for ANSI/VT220 terminals
Copyright (C) 2019  John Tsiombikas <nuclear@member.fsf.org>

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

void ansi_reset(void);
void ansi_clearscr(void);

void ansi_setcursor(int row, int col);
void ansi_cursor(int show);

/* convert a PC cga/ega/vga char+attr to an ANSI sequence and write it to stdout */
void ansi_ibmchar(unsigned char c, unsigned char attr);

#endif	/* ANSI_H_ */
