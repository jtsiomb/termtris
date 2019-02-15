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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <inttypes.h>
#include <assert.h>
#include "game.h"
#include "pieces.h"
#include "ansi.h"
#include "scoredb.h"

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

enum { ERASE_PIECE, DRAW_PIECE };

enum { BLACK, BLUE, GREEN, CYAN, RED, MAGENTA, YELLOW, WHITE };

/* dimensions of the whole screen */
#define SCR_ROWS	20
#define SCR_COLS	20

/* dimensions of the playfield */
#define PF_ROWS		18
#define PF_COLS		10
/* offset of the playfield from the left side of the screen */
#define PF_XOFFS	2
#define PF_YOFFS	0

#define CHAR(c, fg, bg) \
	((uint16_t)(c) | ((uint16_t)(fg) << 12) | ((uint16_t)(bg) << 8))

int scr[SCR_COLS * SCR_ROWS];

static void addscore(int nlines);
static void print_numbers(void);
static int spawn(void);
static int collision(int piece, const int *pos);
static void stick(int piece, const int *pos);
static void erase_completed(void);
static void draw_piece(int piece, const int *pos, int rot, int mode);
static void drawbg(void);
static void drawpf(void);
static void draw_line(int row, int blink);
static void wrtile(int tileid);


static int pos[2], next_pos[2];
static int cur_piece, next_piece;
static int cur_rot, prev_rot;
static int complines[4];
static int num_complines;
static int gameover;
static int pause;
static int score, level, lines;
static int just_spawned;

enum {
	TILE_BLACK,
	TILE_PF,
	TILE_PFSEP,
	TILE_GAMEOVER,
	TILE_IPIECE,
	TILE_OPIECE,
	TILE_JPIECE,
	TILE_LPIECE,
	TILE_SPIECE,
	TILE_TPIECE,
	TILE_ZPIECE,
	TILE_FRM_TL,
	TILE_FRM_TR,
	TILE_FRM_BL,
	TILE_FRM_BR,
	TILE_FRM_LTEE,
	TILE_FRM_RTEE,
	TILE_FRM_HLINE,
	TILE_FRM_LVLINE,
	TILE_FRM_RVLINE
};
#define FIRST_PIECE_TILE	TILE_IPIECE

static uint16_t tiles[][2] = {
	{ CHAR(' ', BLACK, BLACK), CHAR(' ', BLACK, BLACK) },			/* black tile */
	{ CHAR(' ', WHITE, WHITE), CHAR(' ', WHITE, WHITE) },			/* playfield background */
	{ CHAR(G_CHECKER, WHITE, BLACK), CHAR(G_CHECKER, WHITE, BLACK) },	/* well separator */
	{ CHAR(G_CROSS, RED, BLACK), CHAR(G_CROSS, RED, BLACK) },		/* gameover fill */
	{ CHAR('[', WHITE, YELLOW), CHAR(']', WHITE, YELLOW) },			/* L */
	{ CHAR('[', WHITE, GREEN), CHAR(']', WHITE, GREEN) },			/* J */
	{ CHAR('[', WHITE, CYAN), CHAR(']', WHITE, CYAN) },				/* I */
	{ CHAR('[', WHITE, BLUE), CHAR(']', WHITE, BLUE) },				/* O */
	{ CHAR('[', WHITE, RED), CHAR(']', WHITE, RED) },				/* Z */
	{ CHAR('[', WHITE, MAGENTA), CHAR(']', WHITE, MAGENTA) },		/* S */
	{ CHAR('[', WHITE, BLACK), CHAR(']', WHITE, BLACK) },			/* T */
	/* frame tiles */
	{ CHAR(G_UL_CORNER, WHITE, BLACK), CHAR(G_HLINE, WHITE, BLACK) },	/* top-left corner */
	{ CHAR(G_HLINE, WHITE, BLACK), CHAR(G_UR_CORNER, WHITE, BLACK) },	/* top-right corner */
	{ CHAR(G_LL_CORNER, WHITE, BLACK), CHAR(G_HLINE, WHITE, BLACK) },	/* bottom-left corner */
	{ CHAR(G_HLINE, WHITE, BLACK), CHAR(G_LR_CORNER, WHITE, BLACK) },	/* bottom-right corner */
	{ CHAR(G_L_TEE, WHITE, BLACK), CHAR(G_HLINE, WHITE, BLACK) },		/* left-T */
	{ CHAR(G_HLINE, WHITE, BLACK), CHAR(G_R_TEE, WHITE, BLACK) },		/* right-T */
	{ CHAR(G_HLINE, WHITE, BLACK), CHAR(G_HLINE, WHITE, BLACK) },		/* horizontal line */
	{ CHAR(G_VLINE, WHITE, BLACK), CHAR(' ', WHITE, BLACK) },			/* left vertical line */
	{ CHAR(' ', WHITE, BLACK), CHAR(G_VLINE, WHITE, BLACK) }			/* right vertical line */
};

static const char *bgdata[SCR_ROWS] = {
	" #..........#{-----}",
	" #..........#(.....)",
	" #..........#[-----]",
	" #..........#.......",
	" #..........#       ",
	" #..........#{-----}",
	" #..........#(.....)",
	" #..........#(.....)",
	" #..........#>-----<",
	" #..........#(.....)",
	" #..........#(.....)",
	" #..........#[-----]",
	" #..........# {----}",
	" #..........# (....)",
	" #..........# (....)",
	" #..........# (....)",
	" #..........# (....)",
	" #..........# [----]",
	" ############       ",
	"                    "
};

#define NUM_LEVELS	21
static const long level_speed[NUM_LEVELS] = {
	887, 820, 753, 686, 619, 552, 469, 368, 285, 184,
	167, 151, 134, 117, 107, 98, 88, 79, 69, 60, 50
};

int init_game(void)
{
	int i, j;
	int *row = scr;

	srand(time(0));

	pause = 0;
	gameover = 0;
	num_complines = 0;
	score = level = lines = 0;
	tick_interval = level_speed[0];
	cur_piece = -1;
	next_piece = rand() % NUM_PIECES;

	ansi_setcolor(WHITE, BLACK);
	ansi_clearscr();
	ansi_cursor(0);

	/* fill the screen buffer, and draw */
	for(i=0; i<SCR_ROWS; i++) {
		for(j=0; j<SCR_COLS; j++) {
			int tile;
			switch(bgdata[i][j]) {
			case '#':
				tile = TILE_PFSEP;
				break;
			case '.':
				tile = TILE_PF;
				break;
			case '{':
				tile = TILE_FRM_TL;
				break;
			case '}':
				tile = TILE_FRM_TR;
				break;
			case '[':
				tile = TILE_FRM_BL;
				break;
			case ']':
				tile = TILE_FRM_BR;
				break;
			case '>':
				tile = TILE_FRM_LTEE;
				break;
			case '<':
				tile = TILE_FRM_RTEE;
				break;
			case '-':
				tile = TILE_FRM_HLINE;
				break;
			case '(':
				tile = TILE_FRM_LVLINE;
				break;
			case ')':
				tile = TILE_FRM_RVLINE;
				break;
			case ' ':
			default:
				tile = TILE_BLACK;
			}
			row[j] = tile;
		}
		row += SCR_COLS;
	}

	drawbg();

	ansi_setcursor(1, 14 * 2);
	ansi_setcolor(BLACK, WHITE);
	fputs("S C O R E", stdout);

	ansi_setcursor(6, 14 * 2);
	fputs("L E V E L", stdout);

	ansi_setcursor(9, 14 * 2);
	fputs("L I N E S", stdout);

	print_numbers();
	fflush(stdout);

	return 0;
}

void cleanup_game(void)
{
	if(score) {
		save_score(score, lines, level);
	}
	ansi_reset();
}

#define BLINK_UPD_RATE	100
#define GAMEOVER_FILL_RATE	50
#define WAIT_INF	0x7fffffff

long update(long msec)
{
	static long prev_tick;
	long dt;

	if(pause) {
		prev_tick = msec;
		return WAIT_INF;
	}

	dt = msec - prev_tick;

	if(gameover) {
		int i, row = PF_ROWS - gameover;
		int *ptr;

		if(row >= 0) {
			ptr = scr + (row + PF_YOFFS) * SCR_COLS + PF_XOFFS;
			for(i=0; i<PF_COLS; i++) {
				*ptr++ = TILE_GAMEOVER;
			}
			draw_line(row, 1);
			fflush(stdout);

			gameover++;
			return GAMEOVER_FILL_RATE;
		}

		return WAIT_INF;
	}

	if(num_complines) {
		/* lines where completed, we're in blinking mode */
		int i, blink = dt >> 8;

		if(blink > 6) {
			erase_completed();
			num_complines = 0;
			return 0;
		}

		for(i=0; i<num_complines; i++) {
			draw_line(complines[i], blink & 1);
		}
		fflush(stdout);
		return BLINK_UPD_RATE;
	}


	/* fall */
	while(dt >= tick_interval) {
		if(cur_piece >= 0) {
			just_spawned = 0;
			next_pos[0] = pos[0] + 1;
			if(collision(cur_piece, next_pos)) {
				next_pos[0] = pos[0];
				stick(cur_piece, next_pos);
				cur_piece = -1;
				return 0;
			}
		} else {
			/* respawn */
			if(spawn() == -1) {
				gameover = 1;
				return 0;
			}
		}

		dt -= tick_interval;
		prev_tick = msec;
	}

	if(cur_piece >= 0 && (memcmp(pos, next_pos, sizeof pos) != 0 || cur_rot != prev_rot)) {
		draw_piece(cur_piece, pos, prev_rot, ERASE_PIECE);
		draw_piece(cur_piece, next_pos, cur_rot, DRAW_PIECE);
		memcpy(pos, next_pos, sizeof pos);
		prev_rot = cur_rot;
	}

	return tick_interval - dt;
}

static void addscore(int nlines)
{
	static const int stab[] = {40, 100, 300, 1200};	/* bonus per line completed */

	assert(nlines < 5);

	score += stab[nlines - 1] * (level + 1);
	lines += nlines;

	level = lines / 10;
	if(level > NUM_LEVELS - 1) level = NUM_LEVELS - 1;

	tick_interval = level_speed[level];

	print_numbers();
}

static void print_numbers(void)
{
	ansi_setcolor(BLACK, WHITE);

	ansi_setcursor(3, 14 * 2);
	printf("%10d", score);

	ansi_setcursor(7, 17 * 2);
	printf("%2d", level);

	ansi_setcursor(10, 14 * 2);
	printf("%8d", lines);
	fflush(stdout);
}

#define C0	0x9b
#define SS3	0x8f

static void runesc(int csi, char *buf)
{
	if(csi != C0) return;

	if(buf[1] == 0) {
		switch(buf[0]) {
		case 'A':
			game_input('w');	/* up */
			break;
		case 'B':
			game_input('s');	/* down */
			break;
		case 'C':
			game_input('d');	/* right */
			break;
		case 'D':
			game_input('a');	/* left */
			break;
		default:
			break;
		}
	}
}

void game_input(int c)
{
	static int esc, csi;
	static int esctop;
	static char escbuf[64];

	if(esc) {
		esc = 0;
		if(c == 27) {
			quit = 1;
			return;
		}

		switch(c) {
		case '[':
			csi = C0;
			return;
		case 'O':
			csi = SS3;
			return;
		default:
			break;
		}
	}

	if(csi) {
		if(c < 0x20 || c >= 0x80) {
			csi = 0;
			esctop = 0;
		}

		escbuf[esctop++] = c;

		if(c >= 0x40) {
			int prevcsi = csi;
			escbuf[esctop] = 0;
			csi = 0;
			esctop = 0;
			runesc(prevcsi, escbuf);
		}
		return;
	}

	switch(c) {
	case 27:
		esc = 1;
		break;

	case C0:
		esc = 1;
		csi = C0;
		break;

	case 'q':
		quit = 1;
		break;

	case 'a':
		next_pos[1] = pos[1] - 1;
		if(collision(cur_piece, next_pos)) {
			next_pos[1] = pos[1];
		}
		break;

	case 'd':
		next_pos[1] = pos[1] + 1;
		if(collision(cur_piece, next_pos)) {
			next_pos[1] = pos[1];
		}
		break;

	case 'w':
	case ' ':
		prev_rot = cur_rot;
		cur_rot = (cur_rot + 1) & 3;
		if(collision(cur_piece, next_pos)) {
			cur_rot = prev_rot;
		}
		break;

	case 's':
		/* ignore drops until the first update after a spawn */
		if(!just_spawned) {
			next_pos[0] = pos[0] + 1;
			if(collision(cur_piece, next_pos)) {
				next_pos[0] = pos[0];
			}
		}
		break;

	case '\t':
	case '\n':
	case '\r':
	case '0':
		next_pos[0] = pos[0] + 1;
		while(!collision(cur_piece, next_pos)) {
			next_pos[0]++;
		}
		next_pos[0]--;
		break;

	case 'p':
		if(gameover) {
			if(score) {
				save_score(score, lines, level);
			}
			init_game();
		} else {
			pause ^= 1;
		}
		break;

	case '\b':
	case 127:
		if(score) {
			save_score(score, lines, level);
		}
		init_game();
		break;

	default:
		fprintf(stderr, "unhandled input: %x\n", c);
		break;
	}
}

static int spawn(void)
{
	static const int preview_pos[] = {13, 13};
	int r, tries = 2;

	do {
		r = rand() % NUM_PIECES;
	} while(tries-- > 0 && (r | cur_piece | next_piece) == cur_piece);

	draw_piece(next_piece, preview_pos, 0, ERASE_PIECE);
	draw_piece(r, preview_pos, 0, DRAW_PIECE);

	cur_piece = next_piece;
	next_piece = r;

	prev_rot = cur_rot = 0;
	pos[0] = piece_spawnpos[cur_piece][0];
	next_pos[0] = pos[0] + 1;
	pos[1] = next_pos[1] = PF_COLS / 2 + piece_spawnpos[cur_piece][1];

	if(collision(cur_piece, next_pos)) {
		return -1;
	}

	just_spawned = 1;
	return 0;
}

static int collision(int piece, const int *pos)
{
	int i;
	unsigned char *p = pieces[piece][cur_rot];

	for(i=0; i<4; i++) {
		int x = PF_XOFFS + pos[1] + BLKX(*p);
		int y = PF_YOFFS + pos[0] + BLKY(*p);
		p++;

		if(y < 0) continue;

		if(scr[y * SCR_COLS + x] != TILE_PF) return 1;
	}

	return 0;
}

static void stick(int piece, const int *pos)
{
	int i, j, nblank;
	int *pfline;
	unsigned char *p = pieces[piece][cur_rot];

	num_complines = 0;

	for(i=0; i<4; i++) {
		int x = pos[1] + BLKX(*p);
		int y = pos[0] + BLKY(*p);
		p++;

		pfline = scr + (y + PF_YOFFS) * SCR_COLS + PF_XOFFS;
		pfline[x] = piece + FIRST_PIECE_TILE;

		nblank = 0;
		for(j=0; j<PF_COLS; j++) {
			if(pfline[j] == TILE_PF) {
				nblank++;
			}
		}

		if(nblank == 0) {
			complines[num_complines++] = y;
		}
	}

	if(use_bell) {
		putchar('\a');
		fflush(stdout);
	}

	if(num_complines) {
		addscore(num_complines);
	}
}

static void erase_completed(void)
{
	int i, j, srow, drow;
	int *pfstart = scr + PF_YOFFS * SCR_COLS + PF_XOFFS;
	int *dptr;

	/* sort completed lines from highest to lowest row number */
	for(i=0; i<num_complines-1; i++) {
		for(j=i+1; j<num_complines; j++) {
			if(complines[j] > complines[i]) {
				int tmp = complines[j];
				complines[j] = complines[i];
				complines[i] = tmp;
			}
		}
	}

	srow = drow = PF_ROWS - 1;
	dptr = pfstart + drow * SCR_COLS;

	for(i=0; i<PF_ROWS; i++) {
		for(j=0; j<num_complines; j++) {
			if(complines[j] == srow) {
				srow--;
			}
		}

		if(srow < 0) {
			for(j=0; j<PF_COLS; j++) {
				dptr[j] = TILE_PF;
			}

		} else if(srow != drow) {
			int *sptr = pfstart + srow * SCR_COLS;
			memcpy(dptr, sptr, PF_COLS * sizeof *dptr);
		}

		srow--;
		drow--;
		dptr -= SCR_COLS;
	}

	drawpf();
	fflush(stdout);
}

static void draw_piece(int piece, const int *pos, int rot, int mode)
{
	int i;
	int tile = mode == ERASE_PIECE ? TILE_PF : FIRST_PIECE_TILE + piece;
	unsigned char *p = pieces[piece][rot];

	for(i=0; i<4; i++) {
		int x = PF_XOFFS + pos[1] + BLKX(*p);
		int y = PF_YOFFS + pos[0] + BLKY(*p);
		p++;

		if(y < 0) continue;

		ansi_setcursor(y, x * 2);
		wrtile(tile);
	}
	fflush(stdout);
}

static void drawbg(void)
{
	int i, j;
	int *sptr = scr;

	for(i=0; i<SCR_ROWS; i++) {
		ansi_setcursor(i, 0);
		for(j=0; j<SCR_COLS; j++) {
			wrtile(*sptr++);
		}
	}
}

static void drawpf(void)
{
	int i, j;
	int *sptr = scr + PF_YOFFS * SCR_COLS + PF_XOFFS;

	for(i=0; i<PF_ROWS; i++) {
		ansi_setcursor(i + PF_YOFFS, PF_XOFFS * 2);
		for(j=0; j<PF_COLS; j++) {
			wrtile(sptr[j]);
		}
		sptr += SCR_COLS;
	}
}

static void draw_line(int row, int blink)
{
	int i;

	ansi_setcursor(row + PF_YOFFS, PF_XOFFS * 2);

	if(blink) {
		int *sptr = scr + (row + PF_YOFFS) * SCR_COLS + PF_XOFFS;

		for(i=0; i<PF_COLS; i++) {
			wrtile(*sptr++);
		}
	} else {
		for(i=0; i<PF_COLS; i++) {
			wrtile(TILE_PF);
		}
	}
}

static void wrtile(int tileid)
{
	int i;

	for(i=0; i<2; i++) {
		uint16_t c = tiles[tileid][i];
		unsigned char cc = c & 0xff;
		unsigned char ca = c >> 8;

		ansi_ibmchar(cc, ca);
	}
}
