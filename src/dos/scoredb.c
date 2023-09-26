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
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <conio.h>
#include "scoredb.h"
#include "ansi.h"

#define SCORES_OFFS	(((long)scores - 256) & 0xffff)

static int name_dialog(char *buf);

extern const char *progpath;

#define NUM_SCORES	10
static struct score_entry scores[NUM_SCORES] = {{"\0dummy"}};

struct score_entry *read_scores(FILE *fp, int max_scores)
{
	int i;

	if(!fp) {
		if(!(fp = fopen(progpath, "r+b"))) {
			return 0;
		}
	}
	fseek(fp, SCORES_OFFS, SEEK_SET);
	fread(scores, 1, sizeof scores, fp);
	fclose(fp);

	if(!*scores[0].user) {
		return 0;
	}

	for(i=0; i<NUM_SCORES; i++) {
		if(i < NUM_SCORES - 1 && *scores[i + 1].user) {
			scores[i].next = scores + i + 1;
		} else {
			scores[i].next = 0;
		}
	}
	return scores;
}

int save_score(struct score_entry *sc)
{
	int i, idx, rest;
	FILE *fp;
	struct score_entry newsc[NUM_SCORES];

	idx = -1;
	for(i=0; i<NUM_SCORES; i++) {
		if(!*scores[i].user || sc->score > scores[i].score) {
			idx = i;
			break;
		}
	}

	if(idx == -1) return 0;
	rest = NUM_SCORES - idx - 1;

	if(idx > 0) {
		memcpy(newsc, scores, idx * sizeof *newsc);
	}
	if(rest > 0) {
		memcpy(newsc + idx + 1, scores + idx, rest * sizeof *newsc);
	}

	/* get name */
	name_dialog(sc->user);
	if(!*sc->user) {
		strcpy(sc->user, "dosuser");
	}
	newsc[idx] = *sc;

	/* save the new scores table */
	if(!(fp = fopen(progpath, "r+b"))) {
		return -1;
	}
	fseek(fp, SCORES_OFFS, SEEK_SET);
	fwrite(newsc, 1, sizeof newsc, fp);
	fclose(fp);

	/* update the next pointers */
	for(i=1; i<10; i++) {
		if(*newsc[i].user) {
			newsc[i-1].next = newsc + i;
		} else {
			newsc[i-1].next = 0;
		}
	}
	newsc[9].next = 0;

	return 0;
}

int print_scores(int num)
{
	int i;
	struct score_entry *sc = scores;
	/*unsigned long offs = ((long)scores - 256) & 0xffff;

	printf("calculated offset %lx (from: %p)\n", offs, (void*)scores);
	printf("actual address: %p\n", (void*)scores);*/

	for(i=0; i<NUM_SCORES; i++) {
		if(!*sc->user) break;

		printf("%2d. %s - %ld pts  (%ld lines)\n", i + 1, sc->user, sc->score,
				sc->lines);
		sc++;
	}
	return 0;
}

#define DLG_W	(MAX_NAME + 2)
#define DLG_X	((80 - DLG_W) / 2)
#define ATTR	7
#define VALID(x)	(isalnum(x) || (x) == ' ' || (x) == '_' || (x) == '-')

static int name_dialog(char *buf)
{
	int i, j, key, cur;
	char *s;

	ansi_setcursor(8, DLG_X);
	ansi_ibmchar(G_UL_CORNER, ATTR);
	for(i=1; i<DLG_W-1; i++) {
		ansi_ibmchar(G_HLINE, ATTR);
	}
	ansi_ibmchar(G_UR_CORNER, ATTR);

	ansi_setcursor(9, DLG_X);
	ansi_ibmchar(G_VLINE, ATTR);
	for(i=1; i<DLG_W-1; i++) {
		ansi_ibmchar(' ', 0x70);
	}
	ansi_ibmchar(G_VLINE, ATTR);
	ansi_ibmchar(G_CHECKER, 0x70);

	ansi_setcursor(10, DLG_X);
	ansi_ibmchar(G_LL_CORNER, ATTR);
	for(i=1; i<DLG_W-1; i++) {
		ansi_ibmchar(G_HLINE, ATTR);
	}
	ansi_ibmchar(G_LR_CORNER, ATTR);
	ansi_ibmchar(G_CHECKER, 0x70);

	ansi_setcursor(11, DLG_X + 1);
	for(i=0; i<DLG_W; i++) {
		ansi_ibmchar(G_CHECKER, 0x70);
	}

	ansi_setcursor(8, DLG_X + 1);
	ansi_putstr(" High score! ", ATTR);
	ansi_setcursor(9, DLG_X + 1);
	ansi_cursor(1);

	cur = 0;
	for(;;) {
		key = getch();
		switch(key) {
		case 27:
			ansi_cursor(0);
			return -1;

		case '\r':
		case '\n':
			return 0;

		case '\b':
			if(cur > 0) {
				buf[--cur] = 0;
			}
			break;

		default:
			if(VALID(key) && cur < MAX_NAME - 1) {
				buf[cur++] = key;
				buf[cur] = 0;
			}
		}

		s = buf;
		ansi_setcursor(9, DLG_X + 1);
		for(i=0; i<DLG_W-2; i++) {
			if(*s) {
				ansi_ibmchar(*s++, 0x70);
			} else {
				ansi_ibmchar(' ', 0x70);
			}
		}
		ansi_setcursor(9, DLG_X + cur + 1);
	}

	ansi_cursor(0);
	return 0;
}
