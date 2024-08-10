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
#include <conio.h>
#include <i86.h>
#include "game.h"
#include "scoredb.h"


int init(void);
void cleanup(void);
int parse_args(int argc, char **argv);
void print_usage(const char *argv0);
long get_msec(void);

long timer_ticks;
const char *progpath;

/* defined in video.asm */
void detect_video(void);

/* defined in timer.asm */
void init_timer(void);
void cleanup_timer(void);


int main(int argc, char **argv)
{
	int i, res, maxfd;
	long msec, next;
	int key;

	detect_video();

	if(parse_args(argc, argv) == -1) {
		return 1;
	}

	if(init() == -1) {
		return 1;
	}

	for(;;) {
		while(kbhit()) {
			key = getch();
			switch(key) {
			case 0x4b:
				game_input('a');
				break;
			case 0x4d:
				game_input('d');
				break;
			case 0x48:
				game_input('w');
				break;
			case 0x50:
				game_input('s');
				break;
			default:
				game_input(key);
			}
			if(quit) goto end;
		}

		msec = get_msec();
		next = update(msec);
	}

end:
	cleanup();
	return 0;
}

void wait_display(void)
{
}

int init(void)
{
	term_width = 80;
	term_height = 25;

	init_timer();

	if(init_game() == -1) {
		return -1;
	}
	return 0;
}

void cleanup(void)
{
	cleanup_game();
	cleanup_timer();
}

int parse_args(int argc, char **argv)
{
	int i;

	progpath = argv[0];

	for(i=1; i<argc; i++) {
		if(argv[i][0] == '-') {
			if(argv[i][2] == 0) {
				switch(argv[i][1]) {
				case 'm':
					monochrome = 1;
					break;

				case 'g':
				case 'G':
					use_gfxchar = 1;
					break;
				case 'T':
					use_gfxchar = 0;
					break;

				case 'd':
					blackpf = 1;
					break;

				case 'r':
					rotstep = 3;
					break;

				case 's':
					printf("High Scores\n-----------\n");
					print_scores(10);
					exit(0);

				case 'h':
					print_usage(argv[0]);
					exit(0);

				default:
					fprintf(stderr, "invalid option: %s\n", argv[i]);
					print_usage(argv[0]);
					return -1;
				}
			} else {
				fprintf(stderr, "invalid option: %s\n", argv[i]);
				print_usage(argv[0]);
				return -1;
			}
		} else {
			fprintf(stderr, "unexpected argument: %s\n", argv[i]);
			print_usage(argv[0]);
			return -1;
		}
	}
	return 0;
}

void print_usage(const char *argv0)
{
	printf("Usage: %s [options]\n", argv0);
	printf("Options:\n");
	printf(" -m: monochrome output (default: off)\n");
	printf(" -g: use graphical blocks (default: EGA/VGA)\n");
	printf(" -T: use text blocks (default: MDA/CGA)\n");
	printf(" -d: use dark playfield in color mode (default: white)\n");
	printf(" -r: reverse (counter-clockwise) rotation\n");
	printf(" -s: print top 10 high-scores and exit\n");
	printf(" -h: print usage information and exit\n\n");

	printf("Controls:\n");
	printf(" left/right/down arrow key moves the block left, right, or down\n");
	printf(" <A>/<D>/<S> also moves the block left, right, or down\n");
	printf(" <up arrow>, <W>, or <space> rotates the block\n");
	printf(" <enter>, <tab>, or <0> drops the block immediately\n");
	printf(" <P> pauses and unpauses the game\n");
	printf(" <backspace>, or <delete> starts a new game\n");
	printf(" <Q> or hitting escape twice, quits immediately\n");
	printf(" <H> shows or hides the help panel which lists these keybindings.\n");
	printf(" <R> shows or hides the high score table.\n");
}

long get_msec(void)
{
	return timer_ticks * 1000 / 18;
}
