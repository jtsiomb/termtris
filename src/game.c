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
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <inttypes.h>
#include "game.h"
#include "pieces.h"
#include "term.h"
#include "scoredb.h"


int quit;
long tick_interval;
int use_bell;
int monochrome;
int use_gfxchar;
int onlyascii;
int blackpf;
int rotstep = 1;
int term_width, term_height;


enum { ERASE_PIECE, DRAW_PIECE };

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

static void update_cur_piece(void);
static void addscore(int nlines);
static void print_numbers(void);
static void print_help(void);
static void print_slist(void);
static void full_redraw(void);
static int spawn(void);
static int collision(int piece, const int *pos);
static void stick(int piece, const int *pos);
static void erase_completed(void);
static void draw_piece(int piece, const int *pos, int rot, int mode);
static void clear(void);
static void drawbg(void);
static void drawpf(int start_row);
static void draw_line(int row, int blink);
static void wrtile(int tileid);


static int pos[2], next_pos[2];
static int cur_piece, next_piece, prev_piece;
static int cur_rot, prev_rot;
static int complines[4];
static int num_complines;
static int gameover;
static int pause;
static int just_spawned;
static int show_help, show_highscores;

static struct score_entry *scores;
static struct score_entry cur_score;

static int term_xoffs = 20, term_yoffs = 0;	/* TODO detect terminal size to set offsets */

static const int preview_pos[] = {13, 13};

enum {
	TILE_BLACK,
	TILE_PF,
	TILE_PFSEP,
	TILE_GAMEOVER,
	TILE_LPIECE,
	TILE_JPIECE,
	TILE_IPIECE,
	TILE_OPIECE,
	TILE_ZPIECE,
	TILE_SPIECE,
	TILE_TPIECE,
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
#define FIRST_PIECE_TILE	TILE_LPIECE

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

static const char *helpstr[] = {
	"   Toggle (H)elp",
	"",
	"left,right,down/asd:",
	"     move piece",
	" up/w/space: rotate",
	"enter/tab/0: drop",
	" backsp/del: restart",
	"          P: pause",
	"          Q: quit",
	"",
	"   High scores:",
	"      termtris -s"
};

int init_game(void)
{
	int i, j;
	int *row = scr;

	scores = read_scores(0, 10);
	memset(&cur_score, 0, sizeof cur_score);

	srand(time(0));

	pause = 0;
	gameover = 0;
	num_complines = 0;
	tick_interval = level_speed[0];
	cur_piece = -1;
	prev_piece = 0;
	next_piece = rand() % NUM_PIECES;

	term_init();
	term_setcolor(WHITE, BLACK);
	term_clearscr();
	term_cursor(0);

	if(blackpf) {
		tiles[TILE_PF][0] = CHAR(' ', BLACK, BLACK);
		tiles[TILE_PF][1] = CHAR(' ', BLACK, BLACK);
		tiles[TILE_TPIECE][0] = CHAR('[', BLACK, WHITE);
		tiles[TILE_TPIECE][1] = CHAR(']', BLACK, WHITE);
	}

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

	if(onlyascii) {
		for(i=0; i<sizeof tiles/sizeof *tiles; i++) {
			for(j=0; j<2; j++) {
				int c = tiles[i][j] & 0xff;
				switch(c) {
				case G_CHECKER:
					c = '#';
					break;
				case G_LR_CORNER:
				case G_UR_CORNER:
				case G_UL_CORNER:
				case G_LL_CORNER:
				case G_L_TEE:
				case G_R_TEE:
				case G_B_TEE:
				case G_T_TEE:
					c = '+';
					break;
				case G_CROSS:
					c = 'X';
					break;
				case G_HLINE:
					c = '-';
					break;
				case G_VLINE:
					c = '|';
					break;
				case G_CDOT:
					c = '.';
					break;
				default:
					break;
				}
				tiles[i][j] = (tiles[i][j] & 0xff00) | c;
			}
		}
	}


	clear();
	print_help();
	drawbg();
	print_slist();
	print_numbers();
	fflush(stdout);
	return 0;
}

void cleanup_game(void)
{
	if(cur_score.score) {
		save_score(&cur_score);
	}
	term_cursor(1);
	term_reset();
#if !defined(MSDOS) && !defined(__COM__)
	/* don't call this on DOS because it will call term_init again */
	term_clearscr();
#endif
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

		if(cur_score.score) {
			save_score(&cur_score);
			full_redraw();
			memset(&cur_score, 0, sizeof cur_score);
		}
		return WAIT_INF;
	}

	if(num_complines) {
		/* lines where completed, we're in blinking mode */
		int i, blink = dt >> 8;

		if(blink > 6) {
			erase_completed();
			wait_display();
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

	update_cur_piece();
	return tick_interval - dt;
}

#define MAX_TILE_OPS	32
struct tileop {
	int op;
	int x, y;
};

static int diff_piece(int pp, const int *ppos, int prot, int np, const int *npos, int nrot, struct tileop *ops)
{
	int i, j, num, inval = 0;
	unsigned char *p = pieces[pp][prot];
	struct tileop tmpop, *op = ops;

	for(i=0; i<4; i++) {
		op->op = ERASE_PIECE;
		op->x = ppos[1] + BLKX(*p);
		op->y = ppos[0] + BLKY(*p);
		if(op->y >= 0) {
			op++;
		}
		p++;
	}
	p = pieces[np][nrot];
	for(i=0; i<4; i++) {
		op->op = DRAW_PIECE;
		op->x = npos[1] + BLKX(*p);
		op->y = npos[0] + BLKY(*p);
		if(op->y >= 0) {
			op++;
		}
		p++;
	}

	/* find double-changes to the same tile */
	num = op - ops;
	for(i=0; i<num; i++) {
		for(j=i+1; j<num; j++) {
			if(ops[j].x == ops[i].x && ops[j].y == ops[i].y) {
				ops[i].op = -1;	/* cancel the first (erase) */
				inval++;
			}
		}
	}

	/* sort by op-valid,row,col */
#define OP_ORDER(o)	((o).op == -1 ? 0x7fff : (((o).y << 7) | (o).x))
	for(i=0; i<num-1; i++) {
		for(j=i+1; j<num; j++) {
			unsigned int ordi = OP_ORDER(ops[i]);
			unsigned int ordj = OP_ORDER(ops[j]);
			if(ordj < ordi) {
				tmpop = ops[i];
				ops[i] = ops[j];
				ops[j] = tmpop;
			}
		}
	}

	/* drop the invalid ops at the end */
	return num - inval;
}

static void draw_tileops(int piece, struct tileop *ops, int num)
{
	int i, tile, x = -1, y = -1;
	struct tileop *op = ops;

	for(i=0; i<num; i++) {
		if(op->op == -1) {
			op++;
			continue;
		}

		/* if the new op is not under the cursor, handle cursor movement */
		if(op->x != x || op->y != y) {
			term_setcursor(term_yoffs + op->y + PF_YOFFS, term_xoffs +
					(op->x + PF_XOFFS) * 2);
			x = op->x;
			y = op->y;
		}
		tile = op->op == ERASE_PIECE ? TILE_PF : FIRST_PIECE_TILE + piece;
		wrtile(tile);
		x++;
		op++;
	}
}

static void update_cur_piece(void)
{
	int numops;
	struct tileop ops[MAX_TILE_OPS];

	if(cur_piece < 0) return;

	if((numops = diff_piece(cur_piece, pos, prev_rot, cur_piece, next_pos, cur_rot, ops)) > 0) {
		draw_tileops(cur_piece, ops, numops);

		/* for terminals which can't hide the cursor, move it out of the way */
		term_setcursor(0, 0);
		fflush(stdout);

		memcpy(pos, next_pos, sizeof pos);
		prev_rot = cur_rot;
	}
}


static void addscore(int nlines)
{
	static const int stab[] = {40, 100, 300, 1200};	/* bonus per line completed */

	cur_score.score += stab[nlines - 1] * (cur_score.level + 1);
	cur_score.lines += nlines;

	cur_score.level = cur_score.lines / 10;
	if(cur_score.level > NUM_LEVELS - 1) {
		cur_score.level = NUM_LEVELS - 1;
	}

	tick_interval = level_speed[cur_score.level];

	print_numbers();

	if(show_highscores) {
		print_slist();
	}
}

static void print_numbers(void)
{
	char buf[16];

	term_setcolor(BLACK, WHITE);

	term_setcursor(term_yoffs + 3, term_xoffs + 14 * 2);
	sprintf(buf, "%10ld", cur_score.score);
	term_putstr(buf, 7);

	term_setcursor(term_yoffs + 7, term_xoffs + 17 * 2);
	sprintf(buf, "%2ld", cur_score.level);
	term_putstr(buf, 7);

	term_setcursor(term_yoffs + 10, term_xoffs + 14 * 2);
	sprintf(buf, "%8ld", cur_score.lines);
	term_putstr(buf, 7);
}

static void print_help(void)
{
	int i;

	for(i=0; i<sizeof helpstr/sizeof *helpstr; i++) {
		term_setcursor(i + 1, 0);
		if(!i || show_help) {
			term_putstr(helpstr[i], 0x70);
		} else {
			term_putstr("                     ", 0x70);
		}
	}
}

static char *clampstr(const char *s, int len)
{
	int i;
	static char buf[64];

	if(len > 63) len = 63;

	if(len <= 0) {
		buf[0] = 0;
		return buf;
	}

	if(strlen(s) <= len) {
		strcpy(buf, s);
		return buf;
	}

	if(len < 3) {
		memset(buf, ' ', len);
		buf[len] = 0;
		return buf;
	}

	for(i=0; i<len-2; i++) {
		buf[i] = *s++;
	}
	strcpy(buf + i, "..");
	return buf;
}

static void print_slist(void)
{
	int i, x, len, namelen, sclen, maxlen, color, printed_cur = 0;
	struct score_entry *sc;
	char fmtbuf[128], fmt[32], *user;

	x = term_xoffs + SCR_COLS * 2 + 1;
	maxlen = term_width - 1 - x;

	if(maxlen < 8) return;
	if(maxlen > 127) maxlen = 127;

	term_setcursor(1, x);
	if(maxlen < 15) {
		term_putstr("Sco(r)es", 0x70);
	} else {
		term_putstr("Toggle Sco(r)es", 0x70);
	}

	sc = scores;
	for(i=0; i<10; i++) {
		term_setcursor(i + 3, x);
		if(!show_highscores) {
fillblank:	memset(fmtbuf, ' ', maxlen);
			fmtbuf[maxlen] = 0;
			term_putstr(fmtbuf, 0x70);
			if(sc) sc = sc->next;
			continue;
		}

		color = 0x70;
		if(!printed_cur) {
			if(!sc || cur_score.score > sc->score) {
				cur_score.next = sc;
				sc = &cur_score;
				printed_cur = 1;
				color |= 0x80;
			}
		} else {
			if(!sc) goto fillblank;
		}

		user = (sc->user && *sc->user) ? sc->user : "***";

		sclen = sprintf(fmtbuf, "%ld", sc->score);
		if(sclen > maxlen - 5) goto fillblank;
		namelen = maxlen - sclen - 5;

		if(strlen(user) <= namelen) {
			sprintf(fmt, "%%2d. %%-%ds %%ld", namelen);
			len = sprintf(fmtbuf, fmt, i + 1, user, sc->score);
		} else {
			len = sprintf(fmtbuf, "%2d. %s %ld", i + 1, clampstr(user, namelen), sc->score);
		}
		if(len < maxlen) {
			memset(fmtbuf + len, ' ', maxlen - len);
		}
		fmtbuf[maxlen] = 0;
		sc = sc->next;

		term_putstr(fmtbuf, color);
	}
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
		return;
	}

	if(strcmp(buf, "28~") == 0) {
		/* help key */
		game_input('h');
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
		if(!pause) {
			next_pos[1] = pos[1] - 1;
			if(collision(cur_piece, next_pos)) {
				next_pos[1] = pos[1];
			}
		}
		break;

	case 'd':
		if(!pause) {
			next_pos[1] = pos[1] + 1;
			if(collision(cur_piece, next_pos)) {
				next_pos[1] = pos[1];
			}
		}
		break;

	case 'w':
	case ' ':
		if(!pause) {
			prev_rot = cur_rot;
			cur_rot = (cur_rot + rotstep) & 3;
			if(collision(cur_piece, next_pos)) {
				cur_rot = prev_rot;
			}
		}
		break;

	case 's':
		/* ignore drops until the first update after a spawn */
		if(cur_piece >= 0 && !just_spawned && !pause && !gameover) {
			next_pos[0] = pos[0] + 1;
			if(collision(cur_piece, next_pos)) {
				next_pos[0] = pos[0];
				update_cur_piece();
				stick(cur_piece, next_pos);	/* stick immediately */
			}
		}
		break;

	case '\t':
	case '\n':
	case '\r':
	case '0':
		if(!pause && !gameover && cur_piece >= 0) {
			next_pos[0] = pos[0] + 1;
			while(!collision(cur_piece, next_pos)) {
				next_pos[0]++;
			}
			next_pos[0]--;
			update_cur_piece();
			stick(cur_piece, next_pos);	/* stick immediately */
		}
		break;

	case 'p':
		if(gameover) {
			init_game();
		} else {
			pause ^= 1;
		}
		break;

	case '\b':
	case 127:
		init_game();
		break;

	case 'h':
		show_help ^= 1;
		print_help();
		fflush(stdout);
		break;

	case 'r':
		show_highscores ^= 1;
		print_slist();
		fflush(stdout);
		break;

	case '`':
		full_redraw();
		break;

	default:
		/*fprintf(stderr, "unhandled input: %x\n", c);*/
		break;
	}
}

static void full_redraw(void)
{
	clear();
	print_help();
	drawbg();
	print_slist();
	print_numbers();
	drawpf(0);
	if(!gameover) {
		draw_piece(next_piece, preview_pos, 0, DRAW_PIECE);
		draw_piece(cur_piece, next_pos, cur_rot, DRAW_PIECE);
		term_setcursor(0, 0);
		fflush(stdout);
	}
	wait_display();
}

static int spawn(void)
{
	int r, tries = 2;
	int numops;
	struct tileop ops[MAX_TILE_OPS];

	do {
		r = rand() % NUM_PIECES;
	} while(tries-- > 0 && (r | prev_piece | next_piece) == prev_piece);

	if((numops = diff_piece(next_piece, preview_pos, 0, r, preview_pos, 0, ops)) > 0) {
		draw_tileops(r, ops, numops);
		term_setcursor(0, 0);
		fflush(stdout);
	}

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
	prev_piece = cur_piece;	/* used by the spawn routine */
	cur_piece = -1;

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
	int i, j, srow, drow, toprow;
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

	toprow = srow = drow = PF_ROWS - 1;
	dptr = pfstart + drow * SCR_COLS;

	for(i=0; i<PF_ROWS; i++) {
		/* keep track of the topmost non-empty row */
		for(j=0; j<PF_COLS; j++) {
			if(dptr[j] != TILE_PF) {
				toprow = drow;
				break;
			}
		}

		for(j=0; j<num_complines; j++) {
			/* if this row is one of the completed lines, skip it during the copy */
			if(complines[j] == srow) {
				srow--;
			}
		}

		if(srow < 0) {
			/* when source row falls outside of the top, fill with empty tiles */
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

	drawpf(toprow);
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

		term_setcursor(term_yoffs + y, term_xoffs + x * 2);
		wrtile(tile);
	}
}

static void clear(void)
{
	term_setcolor(WHITE, BLACK);
	term_clearscr();
	term_cursor(0);
}

static void drawbg(void)
{
	int i, j;
	int *sptr = scr;

	term_xoffs = term_width / 2 - SCR_COLS;

	for(i=0; i<SCR_ROWS; i++) {
		term_setcursor(term_yoffs + i, term_xoffs + 0);
		for(j=0; j<SCR_COLS; j++) {
			wrtile(*sptr++);
		}
	}

	term_setcursor(term_yoffs + 1, term_xoffs + 14 * 2);
	term_setcolor(BLACK, WHITE);
	term_putstr("S C O R E", 7);

	term_setcursor(term_yoffs + 6, term_xoffs + 14 * 2);
	term_putstr("L E V E L", 7);

	term_setcursor(term_yoffs + 9, term_xoffs + 14 * 2);
	term_putstr("L I N E S", 7);
}

static void drawpf(int start_row)
{
	int i, j;
	int *sptr = scr + (PF_YOFFS + start_row) * SCR_COLS + PF_XOFFS;

	for(i=start_row; i<PF_ROWS; i++) {
		term_setcursor(term_yoffs + i + PF_YOFFS, term_xoffs + PF_XOFFS * 2);
		for(j=0; j<PF_COLS; j++) {
			wrtile(sptr[j]);
		}
		sptr += SCR_COLS;
	}
}

static void draw_line(int row, int blink)
{
	int i;

	term_setcursor(term_yoffs + row + PF_YOFFS, term_xoffs + PF_XOFFS * 2);

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

	if(tileid < 0 || tileid >= sizeof tiles / sizeof *tiles) {
		return;
	}

	for(i=0; i<2; i++) {
		uint16_t c = tiles[tileid][i];
		unsigned char cc = c & 0xff;
		unsigned char ca = c >> 8;

		if(use_gfxchar && (cc == '[' || cc == ']')) {
			ca <<= 4;	/* for gfx blocks bg->fg and bg=0 */
#if defined(MSDOS) || defined(__COM__)
			if(term_type == TERM_PCBIOS) {
				if(!ca) ca = 0x80;	/* special case for T which has black bg */
			} else
#endif
			if(!ca) ca = 0x70;	/* special case for T which has black bg */
		}

		term_ibmchar(cc, ca);
	}
}
