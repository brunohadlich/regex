#include <stdlib.h>
#include <stdio.h>

#define EPSYLON '\0'

struct symbol_state {
	char symbol;
	struct state *state;
};

struct state {
	struct symbol_state *transition_table;
	int n_transitions;
	int final;
};

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

struct automata {
	struct state *initial;
	struct state *final;
};

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

static int entry_index;

int traverse_state(struct state *s, char *entry) {
	if (s->final) {
		return 1;
	}
	char current_symbol = entry[entry_index];
	if (current_symbol != '\0') {
		int i;
		for (i = 0; i < s->n_transitions; i++) {
			if (s->transition_table[i].symbol == EPSYLON) {
				if (traverse_state(s->transition_table[i].state, entry)) {
					return 1;
				}
			} else if (s->transition_table[i].symbol == current_symbol) {
				entry_index++;
				if (traverse_state(s->transition_table[i].state, entry)) {
					return 1;
				}
			}
		}
	}
	return 0;
}

int match_automata(struct automata *a, char *entry) {
	entry_index = 0;
	return traverse_state(a->initial, entry);
}

int main(int argc, char *argv[]) {
	//char *regex = argv[0];
	//char *entry = argv[1];
	//BEGIN REGEX (a|b)*aab
	struct automata *automata_a = new_single_symbol_automata('a');
	struct automata *automata_b = new_single_symbol_automata('b');
	struct automata *automata_a_or_b = new_union_automata(automata_a, automata_b);
	struct automata *automata_kleene = new_kleene_automata(automata_a_or_b);
	struct automata *automata_a_2 = new_single_symbol_automata('a');
	struct automata *automata_kleene_a = new_concatenate_automata(automata_kleene, automata_a_2);
	struct automata *automata_a_3 = new_single_symbol_automata('a');
	struct automata *automata_kleene_a_a = new_concatenate_automata(automata_kleene_a, automata_a_3);
	struct automata *automata_b_2 = new_single_symbol_automata('b');
	struct automata *automata_kleene_a_a_b = new_concatenate_automata(automata_kleene_a_a, automata_b_2);
	//END REGEX (a|b)*aab in automata_kleene_a_a_b
	char *entry = "abbbbabaab";
	int match = match_automata(automata_kleene_a_a_b, entry);
	printf("Did match for entry '%s'? %s\n", entry, match ? "Yes" : "No");//Yes because ends in aab
	entry = "abbbbabab";
	match = match_automata(automata_kleene_a_a_b, entry);
	printf("Did match for entry '%s'? %s\n", entry, match ? "Yes" : "No");//No because ends in aaa and the automata does not reach the final state
	entry = "abbbbabaabaaa";
	match = match_automata(automata_kleene_a_a_b, entry);
	printf("Did match for entry '%s'? %s\n", entry, match ? "Yes" : "No");//Yes because the automata reach the final state
	
	return EXIT_SUCCESS;
}
