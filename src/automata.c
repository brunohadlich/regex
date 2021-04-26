#include <stdlib.h>
#include "automata.h"

struct state *new_state() {
	struct state *s = (struct state *) malloc(sizeof(struct state));
	s->transition_table = NULL;
	s->n_transitions = 0;
	s->final = 0;
	return s;
}

void add_transition(struct state *from, struct state *to, char through) {
	size_t n_plus_one_transitions = (from->n_transitions + 1) * sizeof(struct symbol_state);
	from->transition_table = (struct symbol_state *)realloc(from->transition_table, n_plus_one_transitions);
	from->transition_table[from->n_transitions].symbol = through;
	from->transition_table[from->n_transitions].state = to;
	from->n_transitions++;
}

struct automata *new_single_symbol_automata(char symbol) {
	struct automata *a = (struct automata *) malloc(sizeof(struct automata));
	a->initial = new_state();
	a->final = new_state();
	add_transition(a->initial, a->final, symbol);
	a->final->final = 1;
	return a;
}

struct automata *new_empty_transition_automata() {
	return new_single_symbol_automata(EPSYLON);
}

struct automata *new_union_automata(struct automata *a1, struct automata *a2) {
	struct automata *a = (struct automata *) malloc(sizeof(struct automata));
	a->initial = new_state();
	a->final = new_state();
	add_transition(a->initial, a1->initial, EPSYLON);
	add_transition(a->initial, a2->initial, EPSYLON);
	add_transition(a1->final, a->final, EPSYLON);
	add_transition(a2->final, a->final, EPSYLON);
	a1->final->final = 0;
	a2->final->final = 0;
	a->final->final = 1;
	return a;
}

struct automata *new_concatenate_automata(struct automata *a1, struct automata *a2) {
	struct automata *a = (struct automata *) malloc(sizeof(struct automata));
	add_transition(a1->final, a2->initial, EPSYLON);
	a->initial = a1->initial;
	a->final = a2->final;
	a1->final->final = 0;
	return a;
}

struct automata *new_kleene_automata(struct automata *a1) {
	struct automata *a = (struct automata *) malloc(sizeof(struct automata));
	a->initial = new_state();
	a->final = new_state();
	add_transition(a->initial, a->final, EPSYLON);
	add_transition(a->initial, a1->initial, EPSYLON);
	add_transition(a1->final, a->final, EPSYLON);
	add_transition(a1->final, a1->initial, EPSYLON);
	a1->final->final = 0;
	a->final->final = 1;
	return a;
}
