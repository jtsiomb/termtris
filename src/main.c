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
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/time.h>
#include "game.h"
#include "scoredb.h"

#ifdef __linux__
#include <sys/ioctl.h>
#include <linux/joystick.h>
#define USE_JOYSTICK
#endif

int init(void);
void cleanup(void);
int parse_args(int argc, char **argv);
void print_usage(const char *argv0);
long get_msec(void);

static const char *termfile = "/dev/tty";
static struct termios saved_term;
static struct timeval tv0;


#ifdef USE_JOYSTICK
enum {
	BN_LEFT		= 0x1000000,
	BN_RIGHT	= 0x2000000,
	BN_UP		= 0x4000000,
	BN_DOWN		= 0x8000000
};
static unsigned int jstate;
static int autorepeat;

static const char *jsdevfile;
static int jsdev = -1;

static void read_joystick(void);
static void update_joystick(void);
#endif

int main(int argc, char **argv)
{
	int i, res, maxfd;
	long msec, next;
	struct timeval tv;
	static unsigned char buf[128];

	if(parse_args(argc, argv) == -1) {
		return 1;
	}

	if(init() == -1) {
		return 1;
	}

	gettimeofday(&tv0, 0);

	tv.tv_sec = tick_interval / 1000;
	tv.tv_usec = (tick_interval % 1000) * 1000;

	for(;;) {
		fd_set rdset;
		FD_ZERO(&rdset);
		FD_SET(0, &rdset);
		maxfd = 0;

#ifdef USE_JOYSTICK
		if(jsdev != -1) {
			FD_SET(jsdev, &rdset);
			maxfd = jsdev + 1;
		}

		if(autorepeat) {
			tv.tv_sec = autorepeat <= 2 ? 0 : 0;
			tv.tv_usec = autorepeat <= 2 ? 500000 : 50000;
		}
#endif

		while((res = select(maxfd + 1, &rdset, 0, 0, &tv)) == -1 && errno == EINTR);

		if(res > 0) {
			if(FD_ISSET(0, &rdset)) {
				int rd = read(0, buf, sizeof buf);
				for(i=0; i<rd; i++) {
					game_input(buf[i]);
					if(quit) goto end;
				}
			}

#ifdef USE_JOYSTICK
			if(FD_ISSET(jsdev, &rdset)) {
				read_joystick();
			}
#endif
		}
#ifdef USE_JOYSTICK
		update_joystick();
#endif

		msec = get_msec();
		next = update(msec);

		tv.tv_sec = next / 1000;
		tv.tv_usec = (next % 1000) * 1000;
	}

end:
	cleanup();
	return 0;
}

int init(void)
{
	int fd;
	struct termios term;

#ifdef USE_JOYSTICK
	static const char *def_jsdevfile = "/dev/input/js0";
	if((jsdev = open(jsdevfile ? jsdevfile : def_jsdevfile, O_RDONLY | O_NONBLOCK)) == -1 && jsdevfile) {
		fprintf(stderr, "failed to open joystick device: %s: %s\n", jsdevfile, strerror(errno));
		return -1;
	}
	if(!jsdevfile) jsdevfile = def_jsdevfile;
#endif

	if((fd = open(termfile, O_RDWR)) == -1) {
		fprintf(stderr, "failed to open terminal device: %s: %s\n", termfile, strerror(errno));
		return -1;
	}

	if(tcgetattr(fd, &term) == -1) {
		fprintf(stderr, "failed to get terminal attributes: %s\n", strerror(errno));
		return -1;
	}
	saved_term = term;
	term.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
	term.c_oflag &= ~OPOST;
	term.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
	term.c_cflag = (term.c_cflag & ~(CSIZE | PARENB)) | CS8;
	term.c_cc[VMIN] = 0;
	term.c_cc[VTIME] = 1;

	if(tcsetattr(fd, TCSAFLUSH, &term) == -1) {
		fprintf(stderr, "failed to change terminal attributes: %s\n", strerror(errno));
		return -1;
	}

	close(0);
	close(1);
	close(2);
	dup(fd);
	dup(fd);

	umask(002);
	open("/tmp/termtris.log", O_WRONLY | O_CREAT | O_TRUNC, 0664);

#ifdef USE_JOYSTICK
	if(jsdev != -1) {
		char name[256];
		if(ioctl(jsdev, JSIOCGNAME(sizeof name), name) != -1) {
			fprintf(stderr, "Using joystick %s: %s\n", jsdevfile, name);
		}
	}
#endif

	if(init_game() == -1) {
		return -1;
	}

	return 0;
}

void cleanup(void)
{
	cleanup_game();
	tcsetattr(0, TCSAFLUSH, &saved_term);
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
#ifdef USE_JOYSTICK
					jsdevfile = argv[++i];
#else
					fprintf(stderr, "invalid option: %s: not built with joystick support\n", argv[i]);
					return -1;
#endif
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
	printf(" -t <dev>: terminal device (default: /dev/tty)\n");
	printf(" -b: use bell for sound ques (default: off)\n");
	printf(" -m: monochrome output (default: off)\n");
#ifdef USE_JOYSTICK
	printf(" -j <dev>: use joystick device for input\n");
#endif
	printf(" -s: print top 10 high-scores and exit\n");
	printf(" -h: print usage information and exit\n\n");

	printf("Controls:\n");
	printf(" <A> or <left arrow> moves the block left\n");
	printf(" <D> or <right arrow> moves the block right\n");
	printf(" <S> or <down arrow> drops the block faster\n");
	printf(" <W> or <up arrow> rotates the block\n");
	printf(" <enter>, <tab>, or <0> drops the block immediately\n");
	printf(" <P> pauses and unpauses the game\n");
	printf(" <backspace>, or <delete> starts a new game\n");
	printf(" <Q> or hitting escape twice, quits immediately\n");
#ifdef USE_JOYSTICK
	printf(" This build also accepts joystick input.\n\n");
#endif
}

long get_msec(void)
{
	struct timeval tv;

	gettimeofday(&tv, 0);

	return (tv.tv_sec - tv0.tv_sec) * 1000 + (tv.tv_usec - tv0.tv_usec) / 1000;
}

#ifdef USE_JOYSTICK
static void read_joystick(void)
{
	struct js_event ev;
	unsigned int dir;

	while(read(jsdev, &ev, sizeof ev) > 0) {
		if(ev.type & JS_EVENT_AXIS) {
			int axis = ev.number & 1;
			int val = abs(ev.value) < 32587 ? 0 : ev.value;

			if(axis == 0) {
				if(val) {
					dir = val > 0 ? BN_RIGHT : BN_LEFT;
				} else {
					dir = BN_LEFT | BN_RIGHT;
				}
			} else {
				if(val) {
					dir = val > 0 ? BN_DOWN : BN_UP;
					if(dir == BN_UP) {
						game_input('\n');
					}
				} else {
					dir = BN_DOWN | BN_UP;
				}
			}

			if(val) {
				jstate |= dir;
			} else {
				jstate &= ~dir;
			}
		}
		if(ev.type & JS_EVENT_BUTTON) {
			if(ev.value) {
				if(ev.number >= 4) {
					game_input('p');
				} else {
					game_input('w');
				}
			}
		}
	}

	autorepeat = jstate ? 1 : 0;
}

static void update_joystick(void)
{
	if(jstate) autorepeat++;

	if(jstate & BN_LEFT) {
		game_input('a');
	}
	if(jstate & BN_RIGHT) {
		game_input('d');
	}
	if(jstate & BN_DOWN) {
		game_input('s');
	}
	if(jstate & 0x3) {
		game_input('w');
	}
}
#endif
