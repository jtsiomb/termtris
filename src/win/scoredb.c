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
#include <ctype.h>
#include <limits.h>
#include "scoredb.h"

#ifdef SCOREDIR
#define SCOREDB_PATH	SCOREDIR "/scores"
#else
#define SCOREDB_PATH	"scores"
#endif

static void write_score(FILE *fp, struct score_entry *s);
static int parse_score(char *buf, struct score_entry *ent);
static void free_list(struct score_entry *s);

char *username;


struct score_entry *read_scores(FILE *fp, int max_scores)
{
	char buf[128];
	struct score_entry *node, *head = 0, *tail = 0;
	int close_file = 0;

	/*
	if(!fp) {
		if(!(fp = fopen(SCOREDB_PATH, "rb"))) {
			return 0;
		}
		close_file = 1;

		flk.l_type = F_RDLCK;
		flk.l_start = flk.l_len = 0;
		flk.l_whence = SEEK_SET;
		while(fcntl(fileno(fp), F_SETLKW, &flk) == -1);
	}
	*/
	if(max_scores <= 0) max_scores = INT_MAX;

	while(max_scores-- > 0 && fgets(buf, sizeof buf, fp)) {
		struct score_entry ent;
		if(parse_score(buf, &ent) == -1) {
			continue;
		}

		if(!(node = malloc(sizeof *node)) || !(node->user = malloc(strlen(ent.user) + 1))) {
			perror("failed to allocate scorelist");
			free(node);
			free_list(head);
			return 0;
		}
		strcpy(node->user, ent.user);
		node->score = ent.score;
		node->lines = ent.lines;
		node->level = ent.level;
		node->next = 0;

		if(!head) {
			head = tail = node;
		} else {
			tail->next = node;
			tail = node;
		}
	}

	if(close_file) {
		/*
		flk.l_type = F_UNLCK;
		flk.l_start = flk.l_len = 0;
		flk.l_whence = SEEK_SET;
		fcntl(fileno(fp), F_SETLK, &flk);
		*/
		fclose(fp);
	}

	return head;
}

int save_score(struct score_entry *sc)
{
	int count;
	FILE *fp;
	struct score_entry *slist, *sptr;
	struct score_entry newscore;

	struct passwd *pw;

	if(!username) {
		/*
		if(!(pw = getpwuid(getuid()))) {
			perror("save_score: failed to retrieve user information");
			return -1;
		}
		username = pw->pw_name;
		*/
		username = "user";
	}
	newscore.user = username;
	newscore.score = sc->score;
	newscore.lines = sc->lines;
	newscore.level = sc->level;

	if(!(fp = fopen(SCOREDB_PATH, "r+"))) {
		fclose(fp);
		fprintf(stderr, "failed to save scores to %s: %s\n", SCOREDB_PATH, strerror(errno));
		return -1;
	}

	/* lock the file */
	/*
	flk.l_type = F_WRLCK;
	flk.l_start = flk.l_len = 0;
	flk.l_whence = SEEK_SET;
	while(fcntl(fd, F_SETLKW, &flk) == -1);
	*/

	slist = read_scores(fp, 0);

	rewind(fp);

	count = 0;
	sptr = slist;
	while(sptr && sptr->score >= sc->score && count++ < 100) {
		write_score(fp, sptr);
		sptr = sptr->next;
	}
	if(count++ < 100) {
		write_score(fp, &newscore);
		sc->user = newscore.user;
	}
	while(sptr && count++ < 100) {
		write_score(fp, sptr);
		sptr = sptr->next;
	}
	fflush(fp);

	/* unlock the file */
	/*
	flk.l_type = F_UNLCK;
	flk.l_start = flk.l_len = 0;
	flk.l_whence = SEEK_SET;
	fcntl(fd, F_SETLK, &flk);
	*/

	free_list(slist);
	fclose(fp);
	return 0;
}

static void write_score(FILE *fp, struct score_entry *s)
{
	fprintf(fp, "%s %ld/%ld/%ld\n", s->user, s->score, s->lines, s->level);
}

static char *skip_space(char *s)
{
	if(!s) return 0;
	while(*s && isspace(*s)) s++;
	return *s ? s : 0;
}

static int parse_score(char *buf, struct score_entry *ent)
{
	char *userptr, *scoreptr;
	long scores[3];

	if(!(userptr = skip_space(buf))) return -1;
	scoreptr = userptr;
	while(*scoreptr && !isspace(*scoreptr)) scoreptr++;
	if(!*scoreptr) return -1;

	*scoreptr = 0;
	if(!(scoreptr = skip_space(scoreptr + 1))) return -1;

	if(sscanf(scoreptr, "%ld/%ld/%ld", scores, scores + 1, scores + 2) != 3) {
		return -1;
	}

	ent->user = userptr;
	ent->score = scores[0];
	ent->lines = scores[1];
	ent->level = scores[2];
	return 0;
}

static void free_list(struct score_entry *s)
{
	while(s) {
		struct score_entry *tmp = s;
		s = s->next;
		free(tmp->user);
		free(tmp);
	}
}

int print_scores(int num)
{
	int idx;
	struct score_entry *sc;

	if(!(sc = read_scores(0, num))) {
		fprintf(stderr, "no high-scores found\n");
		return -1;
	}

	idx = 0;
	while(sc) {
		printf("%2d. %s - %ld pts  (%ld lines)\n", ++idx, sc->user, sc->score, sc->lines);
		sc = sc->next;
	}

	return 0;
}
