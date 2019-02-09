#ifndef ANSI_H_
#define ANSI_H_

void ansi_reset(void);
void ansi_clearscr(void);

void ansi_setcursor(int row, int col);
void ansi_cursor(int show);

/* convert a PC cga/ega/vga char+attr to an ANSI sequence and write it to stdout */
void ansi_ibmchar(unsigned char c, unsigned char attr);

#endif	/* ANSI_H_ */
