//This header file has structs and function declarations based on McNaughton-Yamada algorithm
#ifndef AUTOMATA_H
#define AUTOMATA_H

#define EPSYLON '\0'

struct state;

struct symbol_state {
	char symbol;//symbol that leads to the next state
	struct state *state;//state related to the symbol
};

struct state {
	struct symbol_state *transition_table;//array of transitions
	int n_transitions;//number of elements at transition_table array
	int final;//boolean, is an automata final state?
	int id;
};

//The alphabet with all symbols related to an automata
struct alphabet {
	char *characters;
	int size;
};

struct automata {
	//there will always be a single entry point
	struct state *initial;
	struct state **states;
	int n_states;
	struct alphabet alphabet;
};

struct automata *new_single_symbol_automata(char symbol);
struct automata *new_union_automata(struct automata *a1, struct automata *a2);// Operator: |
struct automata *new_concatenate_automata(struct automata *a1, struct automata *a2);// Operator: concatenation
struct automata *new_kleene_automata(struct automata *a1);// Operator: *
struct automata *new_optional_automata(struct automata *a1);// Operator: ?
struct automata *new_repetition_automata(struct automata *a1);// Operator: +
struct automata *concatenate_automatas(struct automata **automatas, const unsigned int n);
struct automata *copy_automata(struct automata *a);
struct automata **copy_automata_n_times(struct automata *a, const unsigned int n);
struct automata *nfa_to_dfa(struct automata *nfa);
int match_automata(struct automata *automata, char *entry);
int match_automata2(struct automata *automata, char *entry, int start_pos);

void free_automata(struct automata *automata);
#endif
