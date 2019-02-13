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
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <pwd.h>
#include "scoredb.h"

struct score_entry {
	char *user;
	int score, lines, level;
	struct score_entry *next;
};

#define SCOREDB_PATH	"/var/games/termtris/scores"

static void write_score(FILE *fp, struct score_entry *s);
static struct score_entry *read_scores(FILE *fp);
static void free_list(struct score_entry *s);

int save_score(int score, int lines, int level)
{
	int fd, count;
	FILE *fp;
	struct passwd *pw;
	struct score_entry *slist, *sptr;
	struct score_entry newscore;
	struct flock flk;

	if(!(pw = getpwuid(getuid()))) {
		perror("save_score: failed to retreive user information");
		return -1;
	}
	newscore.user = pw->pw_name;
	newscore.score = score;
	newscore.lines = lines;
	newscore.level = level;

	if((fd = open(SCOREDB_PATH, O_RDWR | O_CREAT, 0666)) == -1 || !(fp = fdopen(fd, "r+"))) {
		close(fd);
		fprintf(stderr, "failed to save scores to %s: %s\n", SCOREDB_PATH, strerror(errno));
		return -1;
	}

	/* lock the file */
	flk.l_type = F_WRLCK;
	flk.l_start = flk.l_len = 0;
	flk.l_whence = SEEK_SET;
	while(fcntl(fd, F_SETLKW, &flk) == -1);

	slist = read_scores(fp);

	rewind(fp);

	count = 0;
	sptr = slist;
	while(sptr && sptr->score >= score && count++ < 100) {
		write_score(fp, sptr);
		sptr = sptr->next;
	}
	if(count++ < 100) {
		write_score(fp, &newscore);
	}
	while(sptr && count++ < 100) {
		write_score(fp, sptr);
		sptr = sptr->next;
	}
	fflush(fp);

	/* unlock the file */
	flk.l_type = F_UNLCK;
	flk.l_start = flk.l_len = 0;
	flk.l_whence = SEEK_SET;
	fcntl(fd, F_SETLK, &flk);

	free_list(slist);
	fclose(fp);
	return 0;
}

static void write_score(FILE *fp, struct score_entry *s)
{
	fprintf(fp, "%s %d/%d/%d\n", s->user, s->score, s->lines, s->level);
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
	int scores[3];

	if(!(userptr = skip_space(buf))) return -1;
	scoreptr = userptr;
	while(*scoreptr && !isspace(*scoreptr)) scoreptr++;
	if(!*scoreptr) return -1;

	*scoreptr = 0;
	if(!(scoreptr = skip_space(scoreptr + 1))) return -1;

	if(sscanf(scoreptr, "%d/%d/%d", scores, scores + 1, scores + 2) != 3) {
		return -1;
	}

	ent->user = userptr;
	ent->score = scores[0];
	ent->lines = scores[1];
	ent->level = scores[2];
	return 0;
}

static struct score_entry *read_scores(FILE *fp)
{
	char buf[128];
	struct score_entry *node, *head = 0, *tail = 0;

	while(fgets(buf, sizeof buf, fp)) {
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

	return head;
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
	int i;
	FILE *fp;
	struct flock flk;
	char buf[128];
	struct score_entry sc;

	if(!(fp = fopen(SCOREDB_PATH, "r"))) {
		fprintf(stderr, "no high-scores found\n");
		return -1;
	}

	flk.l_type = F_RDLCK;
	flk.l_start = flk.l_len = 0;
	flk.l_whence = SEEK_SET;
	while(fcntl(fileno(fp), F_SETLKW, &flk) == -1);

	for(i=0; i<num; i++) {
		if(!fgets(buf, sizeof buf, fp)) break;

		if(parse_score(buf, &sc) == -1) {
			continue;
		}
		printf("%2d. %s - %d pts  (%d lines)\n", i + 1, sc.user, sc.score, sc.lines);
	}

	flk.l_type = F_UNLCK;
	flk.l_start = flk.l_len = 0;
	flk.l_whence = SEEK_SET;
	fcntl(fileno(fp), F_SETLK, &flk);

	fclose(fp);
	return 0;
}
