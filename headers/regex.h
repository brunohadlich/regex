#ifndef REGEX_H
#define REGEX_H
#include "automata.h"

struct automata *build(char *regex);
int match(struct automata *a, char *entry);
#endif
