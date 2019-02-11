#ifndef GAME_H_
#define GAME_H_

int quit;
long tick_interval;
int use_bell;

int init_game(void);
void cleanup_game(void);

long update(long msec);
void game_input(int c);

#endif	/* GAME_H_ */
