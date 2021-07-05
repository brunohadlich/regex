//This header file has structs and function declarations based on McNaughton-Yamada-Thompson algorithm
#ifndef AUTOMATA_H
#define AUTOMATA_H

#define EPSYLON '\0'

struct symbol_state {
    char symbol;//symbol that leads to the next state
    struct state *state;//state related to the symbol
};

struct state {
	struct symbol_state *transition_table;//array
	int n_transitions;//number of elements at transition_table array
    int final;//boolean, is final state?
};

struct automata {
	//thre will always be a single entry point and a single final point
    struct state *initial;
    struct state *final;
	struct state **states;
	int n_states;
};

struct automata *new_empty_transition_automata();
struct automata *new_single_symbol_automata(char symbol);
struct automata *new_union_automata(struct automata *a1, struct automata *a2);
struct automata *new_concatenate_automata(struct automata *a1, struct automata *a2);
struct automata *new_kleene_automata(struct automata *a1);
struct automata *new_optional_automata(struct automata *a1);
struct automata *new_repetition_automata(struct automata *a1);

void free_automata(struct automata *automata);
#endif
