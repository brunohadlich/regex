//This file has function implementations based on McNaughton-Yamada-Thompson algorithm
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

void free_state(struct state *state) {
	if (state->transition_table) {
		free(state->transition_table);
	}
	free(state);
}

void __free_automata(struct automata *automata) {
	free(automata->states);
	free(automata);
}

void free_automata(struct automata *automata) {
	for (int i = 0; i < automata->n_states; i++) {
		free_state(automata->states[i]);
	}
	__free_automata(automata);
}

struct automata *new_single_symbol_automata(char symbol) {
	struct automata *a = (struct automata *) malloc(sizeof(struct automata));
	a->n_states = 2;
	a->states = (struct state **) malloc(sizeof(struct automata *) * a->n_states);
	a->initial = new_state();
	a->final = new_state();
	a->states[0] = a->initial;
	a->states[1] = a->final;
	add_transition(a->initial, a->final, symbol);
	a->final->final = 1;
	return a;
}

struct automata *new_empty_transition_automata() {
	return new_single_symbol_automata(EPSYLON);
}

struct automata *new_union_automata(struct automata *a1, struct automata *a2) {
	struct automata *a = (struct automata *) malloc(sizeof(struct automata));
	a->n_states = a1->n_states + a2->n_states + 2;
	a->states = (struct state **)malloc(sizeof(struct state *) * a->n_states);
	int k = 0;
	for (int i = 0; i < a1->n_states; i++, k++) {
		a->states[k] = a1->states[i];
	}
	for (int i = 0; i < a2->n_states; i++, k++) {
		a->states[k] = a2->states[i];
	}
	a->initial = new_state();
	a->final = new_state();
	a->states[k++] = a->initial;
	a->states[k] = a->final;
	add_transition(a->initial, a1->initial, EPSYLON);
	add_transition(a->initial, a2->initial, EPSYLON);
	add_transition(a1->final, a->final, EPSYLON);
	add_transition(a2->final, a->final, EPSYLON);
	a1->final->final = 0;
	a2->final->final = 0;
	a->final->final = 1;
	__free_automata(a1);
	__free_automata(a2);
	return a;
}

struct automata *new_concatenate_automata(struct automata *a1, struct automata *a2) {
	struct automata *a = (struct automata *) malloc(sizeof(struct automata));
	a->n_states = a1->n_states + a2->n_states;
	a->states = (struct state **)malloc(sizeof(struct state *) * a->n_states);
	int k = 0;
	for (int i = 0; i < a1->n_states; i++, k++) {
		a->states[k] = a1->states[i];
	}
	for (int i = 0; i < a2->n_states; i++, k++) {
		a->states[k] = a2->states[i];
	}
	a->initial = a1->initial;
	a->final = a2->final;
	add_transition(a1->final, a2->initial, EPSYLON);
	a1->final->final = 0;
	__free_automata(a1);
	__free_automata(a2);
	return a;
}

struct automata *new_kleene_automata(struct automata *a1) {
	struct automata *a = (struct automata *) malloc(sizeof(struct automata));
	a->n_states = a1->n_states + 2;
	a->states = (struct state **)malloc(sizeof(struct state *) * a->n_states);
	int k = 0;
	for (int i = 0; i < a1->n_states; i++, k++) {
		a->states[k] = a1->states[i];
	}
	a->initial = new_state();
	a->final = new_state();
	a->states[k++] = a->initial;
	a->states[k] = a->final;
	add_transition(a->initial, a->final, EPSYLON);
	add_transition(a->initial, a1->initial, EPSYLON);
	add_transition(a1->final, a->final, EPSYLON);
	add_transition(a1->final, a1->initial, EPSYLON);
	a1->final->final = 0;
	a->final->final = 1;
	__free_automata(a1);
	return a;
}

struct automata *new_optional_automata(struct automata *a1) {
	struct automata *a = (struct automata *) malloc(sizeof(struct automata));
	a->n_states = a1->n_states + 2;
	a->states = (struct state **)malloc(sizeof(struct state *) * a->n_states);
	int k = 0;
	for (int i = 0; i < a1->n_states; i++, k++) {
		a->states[k] = a1->states[i];
	}
	a->initial = new_state();
	a->final = new_state();
	a->states[k++] = a->initial;
	a->states[k] = a->final;
	add_transition(a->initial, a->final, EPSYLON);
	add_transition(a->initial, a1->initial, EPSYLON);
	add_transition(a1->final, a->final, EPSYLON);
	a1->final->final = 0;
	a->final->final = 1;
	__free_automata(a1);
	return a;
}

struct automata *new_repetition_automata(struct automata *a1) {
	struct automata *a = (struct automata *) malloc(sizeof(struct automata));
	a->n_states = a1->n_states + 2;
	a->states = (struct state **)malloc(sizeof(struct state *) * a->n_states);
	int k = 0;
	for (int i = 0; i < a1->n_states; i++, k++) {
		a->states[k] = a1->states[i];
	}
	a->initial = new_state();
	a->final = new_state();
	a->states[k++] = a->initial;
	a->states[k] = a->final;
	add_transition(a->initial, a1->initial, EPSYLON);
	add_transition(a1->final, a->final, EPSYLON);
	add_transition(a1->final, a1->initial, EPSYLON);
	a1->final->final = 0;
	a->final->final = 1;
	__free_automata(a1);
	return a;
}
