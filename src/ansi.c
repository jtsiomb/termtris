#include <stdio.h>
#include <string.h>

enum {
	CS_ASCII = 'B',
	CS_GRAPH = '0'
};

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

void ansi_reset(void)
{
	fputs("\033c", stdout);
	fflush(stdout);
}

void ansi_clearscr(void)
{
	fputs("\033[2J", stdout);
}

void ansi_setcursor(int row, int col)
{
	printf("\033[%d;%dH", row + 1, col + 1);
}

void ansi_ibmchar(unsigned char c, unsigned char attr)
{
	char cmd[32];
	char *ptr = cmd;

	if(c >= GMAP_FIRST && c <= GMAP_LAST) {
		if(cur_cs == CS_ASCII) {
			memcpy(ptr, "\033(0", 3);
			ptr += 3;
			cur_cs = CS_GRAPH;
		}

		c = gmap[c - GMAP_FIRST];
	} else {
		if(cur_cs == CS_GRAPH) {
			memcpy(ptr, "\033(B", 3);
			ptr += 3;
			cur_cs = CS_ASCII;
		}
	}

	if(attr != cur_attr) {
		unsigned char fg = cmap[attr & 7];
		unsigned char bg = cmap[(attr >> 4) & 7];

		ptr += sprintf(ptr, "\033[;%d;%dm", fg + 30, bg + 40);
		cur_attr = attr;
	}

	*ptr++ = c;
	*ptr = 0;

	fputs(cmd, stdout);
}
