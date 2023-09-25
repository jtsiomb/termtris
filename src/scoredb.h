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
#ifndef SCOREDB_H_
#define SCOREDB_H_

#include <stdio.h>

#if defined(MSDOS) || defined(__COM__)
/* the DOS version of score_entry is simpler, because scores are embedded in
 * the executable itself in binary format.
 */
#define MAX_NAME	36
struct score_entry {
	char user[MAX_NAME];
	long score, lines, level;
	struct score_entry *next;
};
#else
struct score_entry {
	char *user;
	long score, lines, level;
	struct score_entry *next;
};
#endif

struct score_entry *read_scores(FILE *fp, int max_scores);
int save_score(struct score_entry *sc);
int print_scores(int num);

#endif	/* SCOREDB_H_ */
