#ifndef REGEX_H
#define REGEX_H
#include "automata.h"

struct regex {
	struct automata *automata;
};

struct regex *build_regex(char *regex);
int match_regex(struct regex *regex, char *entry);//return boolean
void free_regex(struct regex *regex);

#endif
