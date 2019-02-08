#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/stat.h>
#include "game.h"

int init(void);
void cleanup(void);

const char *termfile = "/dev/tty";
struct termios saved_term;

int main(int argc, char **argv)
{
	if(argc > 1) {
		termfile = argv[1];
	}

	if(init() == -1) {
		return 1;
	}

	for(;;) {
		proc_input();
		if(quit) break;
	}

	cleanup();
	return 0;
}

int init(void)
{
	int fd;
	struct termios term;

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
	open("ansitris.log", O_WRONLY | O_CREAT, 0664);


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
