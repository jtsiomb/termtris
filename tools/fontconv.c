#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

/* max rows must be a multiple of 6 */
#define MAX_ROWS	24
#define MAX_COLS	10

int proc_font(const char *fname);
void glyph_to_sixel(int glyph[MAX_ROWS][MAX_COLS], int width, int height, int cnum);
char *clean_line(char *s);

int main(int argc, char **argv)
{
	int i;

	for(i=1; i<argc; i++) {
		if(proc_font(argv[i]) == -1) {
			return 1;
		}
	}

	return 0;
}

enum {
	FONT_7X10	= 0x70a,
	FONT_10X16	= 0xa10,
};

int proc_font(const char *fname)
{
	FILE *fp;
	char buf[256];
	char *line;
	int i, state, fontsz, cury, res = -1;
	int glyph[MAX_ROWS][10] = {0};
	int width, height, cnum;

	if(!(fp = fopen(fname, "rb"))) {
		fprintf(stderr, "failed to open font desc: %s: %s\n", fname, strerror(errno));
		return -1;
	}

	state = 0;
	while(fgets(buf, sizeof buf, fp)) {
		if(!(line = clean_line(buf)) || !*line) {
			continue;
		}
		switch(state) {
		case 0:
			if(sscanf(line, "glyph %dx%d %i", &width, &height, &cnum) != 3) {
				fprintf(stderr, "invalid syntax: %s\n", line);
				goto end;
			}
			if(cnum <= 0x20 || cnum >= 0xff) {
				fprintf(stderr, "invalid char num: %d\n", cnum);
				goto end;
			}
			fontsz = (width << 8) | height;
			if(fontsz != FONT_7X10 && fontsz != FONT_10X16) {
				fprintf(stderr, "unknown font size: %dx%d\n", width, height);
				goto end;
			}
			cury = 0;
			state = 1;
			break;

		case 1:
			for(i=0; i<width; i++) {
				if(line[i] == '#') {
					glyph[cury][i] = 1;
				} else if(line[i] == '.') {
					glyph[cury][i] = 0;
				} else {
					fprintf(stderr, "invalid font pattern: %s\n", line);
					goto end;
				}
			}
			if(++cury >= height) {
				glyph_to_sixel(glyph, width, height, cnum);
				state = 0;
			}
			break;

		default:
			break;
		}
	}

	res = 0;
end:
	fclose(fp);
	return res;
}

void glyph_to_sixel(int glyph[MAX_ROWS][MAX_COLS], int width, int height, int cnum)
{
	int i, j, fontsz, cury = 0;
	unsigned int sixel;

	fontsz = (width << 8) | height;
	switch(fontsz) {
	case FONT_7X10:
		printf("\\033P1;%d;1;4{ @", cnum - 32);
		break;

	case FONT_10X16:
		printf("\\033P1;%d;1;10;0;2;20{ @", cnum - 32);
		break;

	default:
		break;
	}

	for(;;) {
		for(i=0; i<width; i++) {
			sixel = 0;
			for(j=0; j<6; j++) {
				sixel = (sixel >> 1) | (glyph[cury + j][i] << 5);
			}
			sixel += 0x3f;	/* offset into valid range of symbols */
			putchar((char)sixel);
		}
		cury += 6;
		if(cury >= height) break;
		putchar('/');
	}

	printf("\\033\\\\\n");
}

char *clean_line(char *s)
{
	char *endp;

	while(*s && isspace(*s)) s++;
	if(!*s) return 0;

	if((endp = strrchr(s, ';'))) {
		*endp = 0;
	} else {
		endp = s + strlen(s) - 1;
	}
	while(--endp >= s && isspace(*endp)) {
		*endp = 0;
	}
	return *s ? s : 0;
}
