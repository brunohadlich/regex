//This file has functions implemented based on McNaughton-Yamada algorithm
#include <stdlib.h>
#include <stdio.h>
#include "automata.h"
#include "data_structures.h"

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
	if (automata->alphabet.characters) {
		free(automata->alphabet.characters);
	}
	free(automata->states);
	free(automata);
}

void free_automata(struct automata *automata) {
	for (int i = 0; i < automata->n_states; i++) {
		free_state(automata->states[i]);
	}
	__free_automata(automata);
}

struct pointer_to_pointer {
	void *p1;
	void *p2;
};

void *get_mapped_pointer(struct pointer_to_pointer *map, void *p1) {
	int i;
	for (i = 0; map[i].p1 != NULL; i++) {
		if (map[i].p1 == p1) {
			return map[i].p2;
		}
	}
	return NULL;
}

struct automata *copy_automata(struct automata *a) {
	struct automata *copy = (struct automata *)malloc(sizeof(struct automata));
	int i, j;
	copy->n_states = a->n_states;
	copy->alphabet.size = a->alphabet.size;
	copy->alphabet.characters = (char *)malloc(copy->alphabet.size);
	for (i = 0; i < copy->alphabet.size; i++) {
		copy->alphabet.characters[i] = a->alphabet.characters[i];
	}
	copy->states = (struct state **)malloc(sizeof(struct state *) * copy->n_states);
	struct pointer_to_pointer *mapped_states = (struct pointer_to_pointer *)malloc(sizeof(struct pointer_to_pointer) * (copy->n_states + 1));

	for (i = 0; i < copy->n_states; i++) {
		copy->states[i] = new_state();
		copy->states[i]->n_transitions = a->states[i]->n_transitions;
		copy->states[i]->final = a->states[i]->final;
		copy->states[i]->id = a->states[i]->id;
		copy->states[i]->transition_table = (struct symbol_state *)malloc(sizeof(struct symbol_state) * copy->states[i]->n_transitions);
		mapped_states[i].p1 = a->states[i];
		mapped_states[i].p2 = copy->states[i];
	}
	mapped_states[copy->n_states].p1 = NULL;
	for (i = 0; i < copy->n_states; i++) {
		for (j = 0; j < copy->states[i]->n_transitions; j++) {
			copy->states[i]->transition_table[j].symbol = a->states[i]->transition_table[j].symbol;
			copy->states[i]->transition_table[j].state = (struct state *)get_mapped_pointer(mapped_states, a->states[i]->transition_table[j].state);
		}
	}
	copy->initial = (struct state *)get_mapped_pointer(mapped_states, a->initial);
	free(mapped_states);
	return copy;
}

void define_states_ids(struct automata *a) {
	int i;
	for (i = 0; i < a->n_states; i++) {
		a->states[i]->id = i;
	}
}

struct automata *new_single_symbol_automata(char symbol) {
	struct automata *a = (struct automata *) malloc(sizeof(struct automata));
	a->alphabet.size = 1;
	a->alphabet.characters = (char *) malloc(1);
	a->alphabet.characters[0] = symbol;
	a->n_states = 2;
	a->states = (struct state **) malloc(sizeof(struct automata *) * a->n_states);
	a->states[0] = a->initial = new_state();
	a->states[1] = new_state();
	a->states[1]->final = 1;
	add_transition(a->initial, a->states[1], symbol);
	define_states_ids(a);
	return a;
}

void union_alphabet(struct automata *a1, struct automata *a2, struct automata *dest) {
	dest->alphabet.characters = (char *)malloc(a1->alphabet.size + a2->alphabet.size);
	int i, i2, i3, character_already_added;
	for (i = 0; i < a1->alphabet.size; i++) {
		dest->alphabet.characters[i] = a1->alphabet.characters[i];
	}
	for (i2 = 0; i2 < a2->alphabet.size; i2++) {
		character_already_added = 0;
		for (i3 = 0; i3 < a1->alphabet.size; i3++) {
			if (a1->alphabet.characters[i3] == a2->alphabet.characters[i2]) {
				character_already_added = 1;
			}
		}
		if (!character_already_added) {
			dest->alphabet.characters[i] = a2->alphabet.characters[i2];
			i++;
		}
	}
	dest->alphabet.size = i;
	dest->alphabet.characters = (char *)realloc(dest->alphabet.characters, i);
}

/*
 * Function used at the McNaughton-Yamada step where a NFA is built
 * at this step every automata has a single final state. This function
 * is not supposed to be used at the subset construction step as the
 * resulting DFA may have more than one final step.
 */
struct state *get_final_state(struct automata *a) {
	int i;
	for (i = 0; i < a->n_states; i++) {
		if (a->states[i]->final) {
			return a->states[i];
		}
	}
	return NULL;
}

struct automata *new_union_automata(struct automata *a1, struct automata *a2) {
	struct automata *a = (struct automata *) malloc(sizeof(struct automata));
	union_alphabet(a1, a2, a);
	a->n_states = a1->n_states + a2->n_states + 2;
	a->states = (struct state **)malloc(sizeof(struct state *) * a->n_states);
	int k = 0;
	for (int i = 0; i < a1->n_states; i++, k++) {
		a->states[k] = a1->states[i];
	}
	for (int i = 0; i < a2->n_states; i++, k++) {
		a->states[k] = a2->states[i];
	}
	a->states[k++] = a->initial = new_state();
	struct state *a_final = a->states[k] = new_state();
	add_transition(a->initial, a1->initial, EPSYLON);
	add_transition(a->initial, a2->initial, EPSYLON);
	struct state *a1_final = get_final_state(a1),
				 *a2_final = get_final_state(a2);
	add_transition(a1_final, a_final, EPSYLON);
	add_transition(a2_final, a_final, EPSYLON);
	a1_final->final = 0;
	a2_final->final = 0;
	a_final->final = 1;
	define_states_ids(a);
	__free_automata(a1);
	__free_automata(a2);
	return a;
}

struct automata *new_concatenate_automata(struct automata *a1, struct automata *a2) {
	struct automata *a = (struct automata *) malloc(sizeof(struct automata));
	union_alphabet(a1, a2, a);
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
	struct state *a1_final = get_final_state(a1);
	add_transition(a1_final, a2->initial, EPSYLON);
	a1_final->final = 0;
	define_states_ids(a);
	__free_automata(a1);
	__free_automata(a2);
	return a;
}

struct automata *new_kleene_automata(struct automata *a1) {
	struct automata *a = (struct automata *) malloc(sizeof(struct automata));
	a->alphabet.characters = a1->alphabet.characters;
	a->alphabet.size = a1->alphabet.size;
	a1->alphabet.characters = NULL;
	a->n_states = a1->n_states + 2;
	a->states = (struct state **)malloc(sizeof(struct state *) * a->n_states);
	int k = 0;
	for (int i = 0; i < a1->n_states; i++, k++) {
		a->states[k] = a1->states[i];
	}
	a->initial = new_state();
	a->states[k++] = a->initial;
	struct state *a_final = a->states[k] = new_state();
	struct state *a1_final = get_final_state(a1);
	add_transition(a->initial, a_final, EPSYLON);
	add_transition(a->initial, a1->initial, EPSYLON);
	add_transition(a1_final, a_final, EPSYLON);
	add_transition(a1_final, a1->initial, EPSYLON);
	a1_final->final = 0;
	a_final->final = 1;
	define_states_ids(a);
	__free_automata(a1);
	return a;
}

struct automata *new_optional_automata(struct automata *a1) {
	struct automata *a = (struct automata *) malloc(sizeof(struct automata));
	a->alphabet.characters = a1->alphabet.characters;
	a->alphabet.size = a1->alphabet.size;
	a1->alphabet.characters = NULL;
	a->n_states = a1->n_states + 2;
	a->states = (struct state **)malloc(sizeof(struct state *) * a->n_states);
	int k = 0;
	for (int i = 0; i < a1->n_states; i++, k++) {
		a->states[k] = a1->states[i];
	}
	a->initial = new_state();
	a->states[k++] = a->initial;
	struct state *a_final = a->states[k] = new_state();
	struct state *a1_final = get_final_state(a1);
	add_transition(a->initial, a_final, EPSYLON);
	add_transition(a->initial, a1->initial, EPSYLON);
	add_transition(a1_final, a_final, EPSYLON);
	a1_final->final = 0;
	a_final->final = 1;
	define_states_ids(a);
	__free_automata(a1);
	return a;
}

struct automata *new_repetition_automata(struct automata *a1) {
	struct automata *a = (struct automata *) malloc(sizeof(struct automata));
	a->alphabet.characters = a1->alphabet.characters;
	a->alphabet.size = a1->alphabet.size;
	a1->alphabet.characters = NULL;
	a->n_states = a1->n_states + 2;
	a->states = (struct state **)malloc(sizeof(struct state *) * a->n_states);
	int k = 0;
	for (int i = 0; i < a1->n_states; i++, k++) {
		a->states[k] = a1->states[i];
	}
	a->initial = new_state();
	a->states[k++] = a->initial;
	struct state *a_final = a->states[k] = new_state();
	struct state *a1_final = get_final_state(a1);
	add_transition(a->initial, a1->initial, EPSYLON);
	add_transition(a1_final, a_final, EPSYLON);
	add_transition(a1_final, a1->initial, EPSYLON);
	a1_final->final = 0;
	a_final->final = 1;
	define_states_ids(a);
	__free_automata(a1);
	return a;
}

int traverse_state(struct state *s, char *entry, int entry_index) {
	if (s->final) {
		return 1;
	}
	char current_symbol = entry[entry_index];
	int i;
	for (i = 0; i < s->n_transitions; i++) {
		if (s->transition_table[i].symbol == EPSYLON) {
			if (traverse_state(s->transition_table[i].state, entry, entry_index)) {
				return 1;
			}
		} else if (s->transition_table[i].symbol == current_symbol) {
			if (traverse_state(s->transition_table[i].state, entry, ++entry_index)) {
				return 1;
			}
		}
	}
	return 0;
}

int match_automata(struct automata *automata, char *entry) {
	return traverse_state(automata->initial, entry, 0);
}

int traverse_state2(struct state *s, char *entry, int entry_index) {
	int longest_path = 0;
	if (s->final) {
		longest_path = entry_index;
	}
	char current_symbol = entry[entry_index];
	int i;
	for (i = 0; i < s->n_transitions; i++) {
		if (s->transition_table[i].symbol == EPSYLON) {
			int result = traverse_state2(s->transition_table[i].state, entry, entry_index);
			if (result) {
				if (result > longest_path) {
					longest_path = result;
				}
			}
		} else if (s->transition_table[i].symbol == current_symbol) {
			int result = traverse_state2(s->transition_table[i].state, entry, ++entry_index);
			if (result) {
				if (result > longest_path) {
					longest_path = result;
				}
			}
		}
	}
	return longest_path;
}

int match_automata2(struct automata *automata, char *entry, int start_pos) {
	return traverse_state2(automata->initial, entry, start_pos);
}

struct automata **copy_automata_n_times(struct automata *a, const unsigned int n) {
	struct automata **automatas = (struct automata **)malloc(sizeof(struct automata *) * n);
	int i;
	for (i = 0; i < n; i++) {
		automatas[i] = copy_automata(a);
	}
	return automatas;
}

struct automata *concatenate_automatas(struct automata **automatas, const unsigned int n) {
	if (n == 0) {
		return NULL;
	}
	struct automata *result = automatas[0];
	int i;
	for (i = 1; i < n; i++) {
		result = new_concatenate_automata(result, automatas[i]);
	}
	return result;
}

//*****************CALCULATE EPSYLON CLOSURE****************
int is_state_in_array_list(struct array_list *list, struct state *state) {
	int i;
	for (i = 0; i < length_array_list(list); i++) {
		if ((*(struct state **)array_list_get(list, i))->id == state->id) {
			return 1;
		}
	}
	return 0;
}

void add_next_epsylon_states_recursively(struct array_list *list, struct state *state) {
	int i;
	add_to_array_list(list, &state);
	for (i = 0; i < state->n_transitions; i++) {
		if (state->transition_table[i].symbol == EPSYLON && !is_state_in_array_list(list, state->transition_table[i].state)) {
			add_next_epsylon_states_recursively(list, state->transition_table[i].state);
		}
	}
}

struct array_list *epsylon_closure(struct array_list *states) {
	int i;
	struct array_list *epsylon_closure_list = alloc_array_list(sizeof(struct state *));
	for (i = 0; i < length_array_list(states); i++) {
		add_next_epsylon_states_recursively(epsylon_closure_list, *(struct state **)array_list_get(states, i));
	}
	return epsylon_closure_list;
}
//**********************************************************

struct nfa_to_dfa_states;

struct symbol_to_dfa_state {
	char symbol;
	//nfa_states are the states of the NFA that will compose
	//a symbol in the new DFA that is being built
	struct nfa_to_dfa_states *nfa_states;
};

struct nfa_to_dfa_states {
	struct array_list *nfa_states;
	int dfa_state_id;
	struct array_list *transitions;
	int visited;
};

struct nfa_to_dfa_states *get_unvisited_dfa_state(struct array_list *dfa_states) {
	int i;
	struct nfa_to_dfa_states *state;
	for (i = 0; i < length_array_list(dfa_states); i++) {
		state = *(struct nfa_to_dfa_states **)array_list_get(dfa_states, i);
		if (state->visited == 0) {
			return state;
		}
	}
	return NULL;
}

void traverse_nfa_through_dfa_state(struct array_list *dfa_transition_table, struct nfa_to_dfa_states *unvisited_dfa_state, char symbol) {
	unvisited_dfa_state->visited = 1;
	struct array_list *transitions_in_symbol = alloc_array_list(sizeof(struct state *));
	int i, k, j;
	for (i = 0; i < length_array_list(unvisited_dfa_state->nfa_states); i++) {
		struct state *nfa_state = *(struct state **)array_list_get(unvisited_dfa_state->nfa_states, i);
		for (k = 0; k < nfa_state->n_transitions; k++) {
			if (nfa_state->transition_table[k].symbol == symbol) {
				add_to_array_list(transitions_in_symbol, &(nfa_state->transition_table[k].state));
			}
		}
	}
	struct array_list *next_dfa_state = epsylon_closure(transitions_in_symbol);
	free_array_list(transitions_in_symbol);
	if (length_array_list(next_dfa_state) == 0) {
		free_array_list(next_dfa_state);
		return;
	}
	struct nfa_to_dfa_states *current_state;
	int add_next_dfa_states_to_transition_table = 1;
	struct nfa_to_dfa_states *already_existing_dfa_state = NULL;
	for (i = 0; i < length_array_list(dfa_transition_table); i++) {
		current_state = *(struct nfa_to_dfa_states **)array_list_get(dfa_transition_table, i);
		int size_next_dfa_states = length_array_list(next_dfa_state);
		int size_current_state_nfa_states = length_array_list(current_state->nfa_states);
		if (size_next_dfa_states == size_current_state_nfa_states) {
			int has_all_states = 1;
			for (k = 0; k < size_next_dfa_states; k++) {
				int has_state = 0;
				struct state *s1 = *(struct state **)array_list_get(next_dfa_state, k);
				for (j = 0; j < size_current_state_nfa_states; j++) {
					struct state *s2 = *(struct state **)array_list_get(current_state->nfa_states, j);
					if (s1->id == s2->id) {
						has_state = 1;
						break;
					}
				}
				if (!has_state) {
					has_all_states = 0;
					break;
				}
			}
			if (has_all_states) {
				add_next_dfa_states_to_transition_table = 0;
				already_existing_dfa_state = current_state;
				break;
			}
		}
	}
	struct nfa_to_dfa_states *nfa_states = already_existing_dfa_state;
	if (add_next_dfa_states_to_transition_table) {
		struct nfa_to_dfa_states *new_dfa_state = (struct nfa_to_dfa_states *)malloc(sizeof(struct nfa_to_dfa_states));
		new_dfa_state->visited = 0;
		new_dfa_state->transitions = alloc_array_list(sizeof(struct symbol_to_dfa_state *));
		new_dfa_state->nfa_states = next_dfa_state;
		add_to_array_list(dfa_transition_table, &new_dfa_state);
		nfa_states = new_dfa_state;
	} else {
		free_array_list(next_dfa_state);
	}
	struct symbol_to_dfa_state *dfa_state = (struct symbol_to_dfa_state *)malloc(sizeof(struct symbol_to_dfa_state));
	dfa_state->symbol = symbol;
	dfa_state->nfa_states = nfa_states;
	add_to_array_list(unvisited_dfa_state->transitions, &dfa_state);
}

//*********************PRINT DEBUG INFO*********************
void print_automata_info(struct automata *a) {
	printf("Alphabet: ");
	int i, j;
	for (i = 0; i < a->alphabet.size; i++) {
		printf("%c, ", a->alphabet.characters[i]);
	}
	printf("\n");
	printf("States ids: ");
	for (i = 0; i < a->n_states; i++) {
		printf("%d, ", a->states[i]->id);
	}
	for (i = 0; i < a->n_states; i++) {
		printf("\n%d -> ", a->states[i]->id);
		for (j = 0; j < a->states[i]->n_transitions; j++) {
			struct symbol_state transition = a->states[i]->transition_table[j];
			char symbol = transition.symbol == EPSYLON ? '-' : transition.symbol;
			printf("[%c:%d], ", symbol, transition.state->id);
		}
	}
	printf("\nInitial state: %d", a->initial->id);
	printf("\n");
}

void print_dfa_transition_table(struct array_list *dfa_transition_table) {
	int i, j;
	printf("DFA transition table size: %d\n", length_array_list(dfa_transition_table));
	for (i = 0; i < length_array_list(dfa_transition_table); i++) {
		struct nfa_to_dfa_states *state = *(struct nfa_to_dfa_states **)array_list_get(dfa_transition_table, i);
		printf("DFA state: %d; size: %d; NFA states: ", i, length_array_list(state->nfa_states));
		for (j = 0; j < length_array_list(state->nfa_states); j++) {
			printf("%d", (*(struct state **)array_list_get(state->nfa_states, j))->id);
			if ((*(struct state **)array_list_get(state->nfa_states, j))->final) {
				printf("*");
			}
			printf(", ");
		}
		printf("; DFA state: ");
		for (j = 0; j < length_array_list(state->transitions); j++) {
			struct symbol_to_dfa_state *transition = *(struct symbol_to_dfa_state **)array_list_get(state->transitions, j);
			printf("[%c:%d], ", transition->symbol, transition->nfa_states->dfa_state_id);
		}
		printf("\n");
	}
}
//**********************************************************

int has_final_state(struct array_list *nfa_states) {
	int i;
	for (i = 0; i < length_array_list(nfa_states); i++) {
		struct state *s = *(struct state **)array_list_get(nfa_states, i);
		if (s->final) {
			return 1;
		}
	}
	return 0;
}

struct automata *nfa_to_dfa(struct automata *nfa) {
	int i, j;
	struct array_list *s0 = alloc_array_list(sizeof(struct state *));
	add_to_array_list(s0, &(nfa->initial));
	struct nfa_to_dfa_states *s0_dfa_state = (struct nfa_to_dfa_states *)malloc(sizeof(struct nfa_to_dfa_states));
	s0_dfa_state->nfa_states = epsylon_closure(s0);
	free_array_list(s0);
	s0_dfa_state->transitions = alloc_array_list(sizeof(struct symbol_to_dfa_state *));
	s0_dfa_state->visited = 0;
	struct array_list *dfa_transition_table = alloc_array_list(sizeof(struct nfa_to_dfa_states *));
	add_to_array_list(dfa_transition_table, &s0_dfa_state);

	struct nfa_to_dfa_states *unvisited_dfa_state;
	while ((unvisited_dfa_state = get_unvisited_dfa_state(dfa_transition_table)) != NULL) {
		for (i = 0; i < nfa->alphabet.size; i++) {
			traverse_nfa_through_dfa_state(dfa_transition_table, unvisited_dfa_state, nfa->alphabet.characters[i]);
		}
	}

	for (i = 0; i < length_array_list(dfa_transition_table); i++) {
		struct nfa_to_dfa_states *state = *(struct nfa_to_dfa_states **)array_list_get(dfa_transition_table, i);
		state->dfa_state_id = i;
	}

	#ifdef DEBUG
	print_dfa_transition_table(dfa_transition_table);
	#endif

	struct automata *dfa = (struct automata *)malloc(sizeof(struct automata));
	dfa->alphabet.size = nfa->alphabet.size;
	dfa->alphabet.characters = (char *)malloc(dfa->alphabet.size);
	for (i = 0; i < nfa->alphabet.size; i++) {
		dfa->alphabet.characters[i] = nfa->alphabet.characters[i];
	}
	dfa->n_states = length_array_list(dfa_transition_table);
	dfa->states = (struct state **)malloc(dfa->n_states * sizeof(struct state *));
	for (i = 0; i < dfa->n_states; i++) {
		dfa->states[i] = new_state();
		dfa->states[i]->id = i;
	}
	for (i = 0; i < dfa->n_states; i++) {
		struct nfa_to_dfa_states *state = *(struct nfa_to_dfa_states **)array_list_get(dfa_transition_table, i);
		dfa->states[i]->final = has_final_state(state->nfa_states);
		for (j = 0; j < length_array_list(state->transitions); j++) {
			struct symbol_to_dfa_state *transition = *(struct symbol_to_dfa_state **)array_list_get(state->transitions, j);
			add_transition(dfa->states[i], dfa->states[transition->nfa_states->dfa_state_id], transition->symbol);
		}
	}
	dfa->initial = dfa->states[0];

	for (i = 0; i < length_array_list(dfa_transition_table); i++) {
		struct nfa_to_dfa_states *state = *(struct nfa_to_dfa_states **)array_list_get(dfa_transition_table, i);
		struct array_list *transitions = state->transitions;
		for (j = 0; j < length_array_list(transitions); j++) {
			free(*(struct symbol_to_dfa_state **)array_list_get(transitions, j));
		}
		free_array_list(transitions);
		free_array_list(state->nfa_states);
		free(state);
	}
	free_array_list(dfa_transition_table);

	return dfa;
}
