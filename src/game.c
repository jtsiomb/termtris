#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <inttypes.h>
#include "game.h"
#include "pieces.h"
#include "ansi.h"

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

static int collision(int piece, const int *pos);
static void stick(int piece, const int *pos);
static void draw_piece(int piece, const int *pos, int rot, int mode);
static void drawbg(void);
static void wrtile(int tileid);


static int pos[2], next_pos[2];
static int cur_piece = -1;
static int cur_rot, prev_rot;

enum {
	TILE_BLACK,
	TILE_PF,
	TILE_PFSEP,
	TILE_IPIECE,
	TILE_OPIECE,
	TILE_JPIECE,
	TILE_LPIECE,
	TILE_SPIECE,
	TILE_TPIECE,
	TILE_ZPIECE
};
#define FIRST_PIECE_TILE	TILE_IPIECE

static uint16_t tiles[][2] = {
	{ CHAR(' ', BLACK, BLACK), CHAR(' ', BLACK, BLACK) },			/* black tile */
	{ CHAR(' ', WHITE, WHITE), CHAR(' ', WHITE, WHITE) },			/* playfield background */
	{ CHAR(G_CHECKER, WHITE, BLACK), CHAR(G_CHECKER, WHITE, BLACK) },	/* well separator */
	{ CHAR(' ', CYAN, CYAN), CHAR(' ', CYAN, CYAN) },				/* straight */
	{ CHAR(' ', BLUE, BLUE), CHAR(' ', BLUE, BLUE) },				/* box */
	{ CHAR(' ', GREEN, GREEN), CHAR(' ', GREEN, GREEN) },			/* J */
	{ CHAR(' ', YELLOW, YELLOW), CHAR(' ', YELLOW, YELLOW) },		/* L */
	{ CHAR(' ', MAGENTA, MAGENTA), CHAR(' ', MAGENTA, MAGENTA) },	/* S */
	{ CHAR(' ', RED, BLACK), CHAR(' ', RED, BLACK) },		/* T */
	{ CHAR(' ', RED, RED), CHAR(' ', RED, RED) },					/* Z */
};


int init_game(void)
{
	int i, j;
	int *row = scr;

	srand(time(0));

	tick_interval = 1000;

	ansi_clearscr();
	ansi_cursor(0);

	/* fill the screen buffer, and draw */
	for(i=0; i<SCR_ROWS; i++) {
		for(j=0; j<SCR_COLS; j++) {
			if(i > PF_ROWS || j < PF_XOFFS - 1 || j > PF_XOFFS + PF_COLS) {
				row[j] = TILE_BLACK;
			} else if((i == PF_ROWS && j >= PF_XOFFS && j < PF_XOFFS + PF_COLS) ||
					j == PF_XOFFS - 1 || j == PF_XOFFS + PF_COLS) {
				row[j] = TILE_PFSEP;
			} else {
				row[j] = TILE_PF;
			}
		}
		row += SCR_COLS;
	}

	drawbg();
	fflush(stdout);

	return 0;
}

void cleanup_game(void)
{
	ansi_reset();
}

long update(long msec)
{
	static long prev_tick;
	long dt;

	dt = msec - prev_tick;

	/* fall */
	while(dt >= tick_interval) {
		if(cur_piece >= 0) {
			next_pos[0] = pos[0] + 1;
			if(collision(cur_piece, next_pos)) {
				next_pos[0] = pos[0];
				fprintf(stderr, "stick at row %d col %d\n", pos[0], pos[1]);
				stick(cur_piece, next_pos);
				cur_piece = -1;
			}
		} else {
			cur_piece = rand() % NUM_PIECES;
			fprintf(stderr, "spawn: %d\n", cur_piece);
			pos[0] = next_pos[0] = piece_spawnpos[cur_piece][0];
			pos[1] = next_pos[1] = PF_COLS / 2 + piece_spawnpos[cur_piece][1];
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

void game_input(int c)
{
	switch(c) {
	case 27:
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
		next_pos[0] = pos[0] + 1;
		if(collision(cur_piece, next_pos)) {
			next_pos[0] = pos[0];
		}
		break;

	default:
		break;
	}
}

static int collision(int piece, const int *pos)
{
	int i;
	unsigned char *p = pieces[piece][cur_rot];

	for(i=0; i<4; i++) {
		int x = PF_XOFFS + pos[1] + BLKX(*p);
		int y = PF_YOFFS + pos[0] + BLKY(*p);
		p++;

		if(scr[y * SCR_COLS + x] != TILE_PF) return 1;
	}

	return 0;
}

static void stick(int piece, const int *pos)
{
	int i;
	unsigned char *p = pieces[piece][cur_rot];

	for(i=0; i<4; i++) {
		int x = PF_XOFFS + pos[1] + BLKX(*p);
		int y = PF_YOFFS + pos[0] + BLKY(*p);
		p++;

		scr[y * SCR_COLS + x] = piece + FIRST_PIECE_TILE;
	}
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
