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
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "game.h"

enum { CS_ASCII, CS_GRAPH, CS_CUSTOM };

#define GMAP_FIRST	0xb0
#define GMAP_LAST	0xda

unsigned char gmap[] = {
	0x61, 0x61, 0x61,		/* checker */
	0x78,					/* vline */
	0x75, 0x75, 0x75,		/* right T */
	0x6b, 0x6b,				/* upper-right corner */
	0x75,					/* right T */
	0x78,					/* vline */
	0x6b,					/* upper-right corner */
	0x6a, 0x6a, 0x6a,		/* lower-right corner */
	0x6b,					/* upper-right corner */
	0x6d,					/* lower-left corner */
	0x76,					/* bottom T */
	0x77,					/* top T */
	0x74,					/* left T */
	0x71,					/* hline */
	0x6e,					/* cross */
	0x74, 0x74,				/* left T */
	0x6d,					/* lower-left corner */
	0x6c,					/* upper-left corner */
	0x76,					/* bottom T */
	0x77,					/* top T */
	0x74,					/* left T */
	0x71,					/* hline */
	0x6e,					/* cross */
	0x76, 0x76,				/* bottom T */
	0x77, 0x77,				/* top T */
	0x6d, 0x6d,				/* lower-left corner */
	0x6c, 0x6c,				/* upper-left corner */
	0x6e, 0x6e,				/* cross */
	0x6a,					/* lower-right corner */
	0x6c					/* upper-left corner */
};

static unsigned char cmap[] = {0, 4, 2, 6, 1, 5, 3, 7};

static unsigned char cur_attr = 0xff;
static int cur_cs = CS_ASCII;

static const char *sixels7x10[] = {
	"~~@@@pH/NNGGGGH",
	"Hp@@@~~/HGGGGNN"
};
static const char *sixels10x16[] = {
	"~~f^jVzf^j/~~mXfYtmjY/FFDAHBGBAH",
	"VZvNzfZvR@/d\\jTyTmtGA/AHB@IBGB?A"
};
static const char *sixels15x12[] = {
	"~~JtJdZdJt~~dYd/~~TITIDYDQ^^IDI",
	"tJdZdJtJdZYdQlQ/QLQDYDQLQHHEHAL"
};
static char custom_char[] = {"[]"};
#define NUM_CUSTOM	(sizeof sixels7x10 / sizeof *sixels7x10)

void ansi_init(void)
{
	int i, val, vtclass = -1;
	char buf[64], *env;

	if(use_gfxchar) {
		/* detect the terminal type */
		/* if there is a TERM env var with "vtxxx" where xxx >= 200, heed that */
		if((env = getenv("TERM")) && tolower(env[0]) == 'v' && tolower(env[1]) == 't'
				&& (val = atoi(env + 2)) >= 200 && val < 999) {
			vtclass = 60 + val / 100;
		} else {
			/* otherwise try asking for the device attributes string */
			printf("\033[c\n");
			fflush(stdout);
			if(fgets(buf, sizeof buf, stdin) && sscanf(buf, "\033[?%d", &val) == 1) {
				vtclass = val;
			}

		}
		if(vtclass == -1) {
			vtclass = 64;	/* default to VT420 */
		}


		/* upload custom character set */
		printf("Uploading custom character set (VT%dx0) ... ", vtclass % 10);
		fflush(stdout);
		for(i=0; i<NUM_CUSTOM; i++) {
			switch(vtclass) {
			case 62:
				/* VT220 mode, 7x10 */
				printf("\033P1;%d;1;4;0;2{ @%s\033\\", (int)custom_char[i] - 32,
						sixels7x10[i]);
				break;
			case 63:
				/* VT320 mode, 15x12 */
				printf("\033P1;%d;1;15;0;2;12{ @%s\033\\", (int)custom_char[i] - 32,
						sixels15x12[i]);
				break;
			case 64:
			default:
				/* VT420 mode, 10x16 */
				printf("\033P1;%d;1;10;0;2;16{ @%s\033\\", (int)custom_char[i] - 32,
						sixels10x16[i]);
			}
		}
		printf("done\n");
	}
}

void ansi_recall(void)
{
	fputs("\033c", stdout);
	fflush(stdout);
}

void ansi_reset(void)
{
	fputs("\033[0m", stdout);
	fputs("\033[!p", stdout);
	fflush(stdout);
}

void ansi_clearscr(void)
{
	fputs("\033[H\033[2J", stdout);
}

void ansi_setcursor(int row, int col)
{
	printf("\033[%d;%dH", row + 1, col + 1);
}

void ansi_cursor(int show)
{
	printf("\033[?25%c", show ? 'h' : 'l');
	fflush(stdout);
}

void ansi_setcolor(int fg, int bg)
{
	if(monochrome) return;

	cur_attr = (fg << 4) | bg;
	fg = cmap[fg];
	bg = cmap[bg];

	printf("\033[;%d;%dm", fg + 30, bg + 40);
}

void ansi_ibmchar(unsigned char c, unsigned char attr)
{
	char cmd[32];
	char *ptr = cmd;

	if(c >= GMAP_FIRST && c <= GMAP_LAST) {
		if(cur_cs != CS_GRAPH) {
			memcpy(ptr, "\033(0", 3);
			ptr += 3;
			cur_cs = CS_GRAPH;
		}

		c = gmap[c - GMAP_FIRST];
	} else if(use_gfxchar && (c == '[' || c == ']')) {
		if(cur_cs != CS_CUSTOM) {
			memcpy(ptr, "\033( @", 4);
			ptr += 4;
			cur_cs = CS_CUSTOM;
		}
	} else {
		if(cur_cs != CS_ASCII) {
			memcpy(ptr, "\033(B", 3);
			ptr += 3;
			cur_cs = CS_ASCII;
		}
	}

	if(monochrome) {
		attr &= 0x80;
	}

	if(attr != cur_attr) {
		int bold = attr & 0x80 ? 1 : 0;

		if(monochrome) {
			ptr += sprintf(ptr, "\033[%dm", bold);
		} else {
			unsigned char bg = cmap[attr & 7];
			unsigned char fg = cmap[(attr >> 4) & 7];

			ptr += sprintf(ptr, "\033[%d;%d;%dm", bold, fg + 30, bg + 40);
		}
		cur_attr = attr;
	}

	*ptr++ = c;
	*ptr = 0;

	fputs(cmd, stdout);
}

void ansi_putstr(const char *s, unsigned char attr)
{
	while(*s) {
		ansi_ibmchar(*s++, attr);
	}
}
