/*
Termtris - a tetris game for ANSI/VT220 terminals
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
#ifndef GAME_H_
#define GAME_H_

extern int quit;
extern long tick_interval;
extern int use_bell;
extern int monochrome;
extern int use_gfxchar;
extern int onlyascii;

extern int term_width, term_height;

int init_game(void);
void cleanup_game(void);

long update(long msec);
void game_input(int c);

/* wait for any pending drawing to be completed before proceeding
 * implemented in main.c
 */
void wait_display(void);

#endif	/* GAME_H_ */
