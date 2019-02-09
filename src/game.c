#include <stdio.h>
#include <string.h>
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

enum { BLACK, BLUE, GREEN, CYAN, RED, MAGENTA, YELLOW, WHITE };

/* dimensions of the whole screen */
#define SCR_ROWS	20
#define SCR_COLS	20

/* dimensions of the playfield */
#define PF_ROWS		18
#define PF_COLS		10
/* offset of the playfield from the left side of the screen */
#define PF_XOFFS	2

#define CHAR(c, fg, bg) \
	((uint16_t)(c) | ((uint16_t)(fg) << 12) | ((uint16_t)(bg) << 8))

uint16_t scr[SCR_COLS * SCR_ROWS];

static void wrchar(uint16_t c);


int init_game(void)
{
	int i, j;
	uint16_t *row = scr;

	tick_interval = 1000;

	ansi_clearscr();
	ansi_cursor(0);

	/* fill the screen buffer, and draw */
	for(i=0; i<SCR_ROWS; i++) {
		ansi_setcursor(i, 0);

		for(j=0; j<SCR_COLS; j++) {
			if(i > PF_ROWS || j < PF_XOFFS - 1 || j > PF_XOFFS + PF_COLS) {
				row[j] = CHAR(' ', WHITE, BLACK);
			} else if((i == PF_ROWS && j >= PF_XOFFS && j < PF_XOFFS + PF_COLS) ||
					j == PF_XOFFS - 1 || j == PF_XOFFS + PF_COLS) {
				row[j] = CHAR(G_CHECKER, WHITE, BLACK);
			} else {
				row[j] = CHAR(' ', BLACK, WHITE);
			}

			wrchar(row[j]);
		}

		row += SCR_COLS;
	}
	fflush(stdout);

	return 0;
}

void cleanup_game(void)
{
	ansi_reset();
}

static int pos[2] = {0, PF_COLS / 2};
static int next_pos[2] = {0, PF_COLS / 2};

long update(long msec)
{
	static long prev_tick;
	long dt;

	dt = msec - prev_tick;

	/* fall */
	while(dt >= tick_interval) {
		next_pos[0] = (pos[0] + 1) % PF_ROWS;
		dt -= tick_interval;
		prev_tick = msec;
	}

	if(memcmp(pos, next_pos, sizeof pos) != 0) {
		ansi_setcursor(pos[0], (PF_XOFFS + pos[1]) * 2);
		wrchar(CHAR(' ', BLACK, WHITE));

		memcpy(pos, next_pos, sizeof pos);
		ansi_setcursor(pos[0], (PF_XOFFS + pos[1]) * 2);
		wrchar(CHAR(' ', RED, RED));

		fflush(stdout);
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
		if(pos[1] > 0) {
			next_pos[1] = pos[1] - 1;
		}
		break;

	case 'd':
		if(pos[1] < PF_COLS - 1) {
			next_pos[1] = pos[1] + 1;
		}
		break;

	default:
		break;
	}
}

static void wrchar(uint16_t c)
{
	unsigned char cc = c & 0xff;
	unsigned char ca = c >> 8;

	ansi_ibmchar(cc, ca);
	ansi_ibmchar(cc, ca);
}
