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
void glyph_to_asm(int glyph[MAX_ROWS][MAX_COLS], int width, int height, int cnum);
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
	FONT_7X10_VT2	= 0x70a,
	FONT_15X12_VT3	= 0xf0c,
	FONT_10X16_VT4	= 0xa10,
	FONT_8X14_EGA	= 0x80e,
	FONT_8X16_VGA	= 0x810
};

#define FONT_VALID(x)	((x) == FONT_7X10_VT2 || (x) == FONT_10X16_VT4 || \
		(x) == FONT_15X12_VT3 || (x) == FONT_8X14_EGA || (x) == FONT_8X16_VGA)
#define FONT_IS_VT(x)	(!FONT_IS_PC(x))
#define FONT_IS_PC(x)	(((x) >> 8) == 8)

int proc_font(const char *fname)
{
	FILE *fp;
	char buf[256];
	char *line;
	int i, state, fontsz, cury, count, tmp, res = -1;
	int glyph[MAX_ROWS][10] = {0};
	int width, height, cnum;

	if(!(fp = fopen(fname, "rb"))) {
		fprintf(stderr, "failed to open font desc: %s: %s\n", fname, strerror(errno));
		return -1;
	}

	cnum = 0x20;
	state = 0;
	while(fgets(buf, sizeof buf, fp)) {
		if(!(line = clean_line(buf)) || !*line) {
			continue;
		}
		switch(state) {
		case 0:
			if((count = sscanf(line, "glyph %dx%d %i", &width, &height, &tmp)) < 2) {
				fprintf(stderr, "invalid syntax: %s\n", line);
				goto end;
			}
			if(count >= 3) {
				cnum = tmp;
			} else {
				cnum++;
			}
			fontsz = (width << 8) | height;
			if(!FONT_VALID(fontsz)) {
				fprintf(stderr, "unknown font size: %dx%d\n", width, height);
				goto end;
			}
			if(FONT_IS_VT(fontsz) && (cnum <= 0x20 || cnum >= 0xff)) {
				fprintf(stderr, "invalid char num: %d\n", cnum);
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
				if(FONT_IS_VT(fontsz)) {
					glyph_to_sixel(glyph, width, height, cnum);
				} else {
					glyph_to_asm(glyph, width, height, cnum);
				}
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
	case FONT_7X10_VT2:
		printf("\\033P1;%d;1;4{ @", cnum - 32);
		break;

	case FONT_15X12_VT3:
		printf("\\033P1;%d;1;15;0;2;12{ @", cnum - 32);
		break;

	case FONT_10X16_VT4:
		printf("\\033P1;%d;1;10;0;2;16{ @", cnum - 32);
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

void glyph_to_asm(int glyph[MAX_ROWS][MAX_COLS], int width, int height, int cnum)
{
	int i, j;
	unsigned int bits;

	printf("glyph_%02xh:\n", (unsigned int)cnum);

	for(i=0; i<height; i++) {
		bits = 0;
		for(j=0; j<width; j++) {
			bits = (bits << 1) | glyph[i][j];
		}
		printf("\tdb 0x%02x\n", bits);
	}
	putchar('\n');
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
