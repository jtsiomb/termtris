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
#include <errno.h>
#include <signal.h>
#include <windows.h>
#include "game.h"
#include "scoredb.h"

int init(void);
void cleanup(void);
int parse_args(int argc, char **argv);
void print_usage(const char *argv0);
long get_msec(void);
void sighandler(int s);

static const char *termfile;
static long start_time;

extern int no_autogfx;		/* defined in ansi.c */
extern char *username;		/* defined in scoredb.c */


int main(int argc, char **argv)
{
	long msec, next;
	static unsigned char buf[128];

	if(parse_args(argc, argv) == -1) {
		return 1;
	}

	if(init() == -1) {
		return 1;
	}

	start_time = GetTickCount();

	for(;;) {
		/*
		if(res > 0) {
			if(FD_ISSET(0, &rdset)) {
				int rd = read(0, buf, sizeof buf);
				for(i=0; i<rd; i++) {
					game_input(buf[i]);
					if(quit) goto end;
				}
			}
		}
		*/

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
	/*
	close(0);
	close(1);
	close(2);
	dup(fd);
	dup(fd);

	umask(002);
	open("/tmp/termtris.log", O_WRONLY | O_CREAT | O_TRUNC, 0664);
	*/

	term_width = 80;
	term_height = 24;

	if(init_game() == -1) {
		return -1;
	}

	return 0;
}

void cleanup(void)
{
	cleanup_game();
}

int parse_args(int argc, char **argv)
{
	int i;

	for(i=1; i<argc; i++) {
		if(argv[i][0] == '-') {
			if(argv[i][2] == 0) {
				switch(argv[i][1]) {
				case 't':
					termfile = argv[++i];
					break;

				case 'b':
					use_bell = 1;
					break;

				case 'm':
					monochrome = 1;
					break;

				case 'j':
					fprintf(stderr, "invalid option: %s: not built with joystick support\n", argv[i]);
					return -1;

				case 'g':
				case 'G':
					use_gfxchar = 1;
					break;

				case 'T':
					use_gfxchar = 0;
					no_autogfx = 1;
					break;

				case 'a':
					onlyascii = 1;
					break;

				case 'u':
					if(!argv[++i]) {
						fprintf(stderr, "-u must be followed by a user name\n");
						return -1;
					}
					username = argv[i];
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
	printf("  -t <dev>: terminal device (default: /dev/tty)\n");
	printf("  -b: use bell for sound cues (default: off)\n");
	printf("  -m: monochrome output (default: off)\n");
	printf("  -g: use custom block graphics (default: auto)\n");
	printf("  -T: don't use custom block graphics, inhibit auto-detection\n");
	printf("  -a: use only ASCII characters\n");
	printf("  -u <name>: override username for high scores\n");
	printf("  -r: reverse (counter-clockwise) rotation\n");
	printf("  -s: print top 10 high-scores and exit\n");
	printf("  -h: print usage information and exit\n");
	printf("Controls:\n");
	printf("  left/right/down arrow key moves the block left, right, or down\n");
	printf("  <A>/<D>/<S> also moves the block left, right, or down\n");
	printf("  <up arrow>, <W>, or <space> rotates the block\n");
	printf("  <enter>, <tab>, or <0> drops the block immediately\n");
	printf("  <P> pauses and unpauses the game\n");
	printf("  <backspace>, or <delete> starts a new game\n");
	printf("  <Q> or hitting escape twice, quits immediately\n");
	printf("  <H> shows or hides the help panel which lists these keybindings.\n");
	printf("  <R> shows or hides the high score table.\n");
}

long get_msec(void)
{
	return GetTickCount() - start_time;
}
