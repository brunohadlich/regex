#ifndef AUTOMATA_H
#define AUTOMATA_H
const char EPSYLON = '\0';

struct symbol_state {
    char symbol;
    struct state *state;
};

struct state {
	struct symbol_state *transition_table;
	int n_transitions;
    int final;
};

struct automata {
    struct state *initial;
    struct state *final;
};

struct automata *new_empty_transition_automata();
struct automata *new_single_symbol_automata(char symbol);
struct automata *new_union_automata(struct automata *a1, struct automata *a2);
struct automata *new_concatenate_automata(struct automata *a1, struct automata *a2);
struct automata *new_kleene_automata(struct automata *a1);
#endif
