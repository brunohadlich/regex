#ifndef REGEX_H
#define REGEX_H
#include "./automata.h"

struct regex {
	char *regex;
	struct automata *automata;
};

struct regex_match {
	int start_pos;
	int end_pos;
};

struct regex_matches {
	struct regex_match *matches;
	int n;
};

struct regex *build_regex(char *regex);
int match_regex(struct regex *regex, char *entry);//return boolean
void free_regex(struct regex *regex);
struct regex_matches *matches_regex(struct regex *regex, char *entry);
void free_regex_matches(struct regex_matches *regex_matches);

#endif
