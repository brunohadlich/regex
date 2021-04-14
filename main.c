#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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

enum stack_element_type {
	AUTOMATA = 1,
	OPERATOR = 2
};

struct regex_stack {
	enum stack_element_type type;
	union {
		struct automata *automata;
		char operator;
	} element;
	struct regex_stack *previous;
};

struct automata *build_automata(char *regex) {
	int regex_length = strlen(regex);
	if (regex_length == 0) {
		return NULL;
	}
	int i;
	struct regex_stack *stack = (struct regex_stack *) malloc(sizeof(struct regex_stack));
	stack->type = OPERATOR;
	stack->element.operator = '(';
	stack->previous = NULL;
	for (i = 0; i <= regex_length; i++) {
		char c = regex[i];
		switch (c) {
			case '|': {
				if (stack->type == OPERATOR && (stack->element.operator == '|' || stack->element.operator == '(')) {
					printf("Found '|' after '|' or '(' in position %d.\n", i);
					return NULL;//ERROR
				}
				struct regex_stack *new_element = (struct regex_stack *) malloc(sizeof(struct regex_stack));
				new_element->type = OPERATOR;
				new_element->element.operator = '|';
				new_element->previous = stack;
				stack = new_element;
				printf("Added operator |\n");
				break;
			}
			case '*': {
				if (stack->type == OPERATOR && (stack->element.operator == '|' || stack->element.operator == '(')) {
					printf("Found '*' after '|' or '(' in position %d.\n", i);
					return NULL;//ERROR
				}
				stack->element.automata = new_kleene_automata(stack->element.automata);
				printf("Replaced automata by automata with kleene operator\n");
				break;
			}
			case '(': {
				struct regex_stack *new_element = (struct regex_stack *) malloc(sizeof(struct regex_stack));
				new_element->type = OPERATOR;
				new_element->element.operator = '(';
				new_element->previous = stack;
				stack = new_element;
				printf("Added operator (\n");
				break;
			}
			case ')':
			case '\0': {
				if (stack->type == OPERATOR && stack->element.operator == '|' || stack->element.operator == '(') {
					printf("Found ')' after '|' or '(' in position %d.\n", i);
					return NULL;//ERROR
				}
				struct automata *automata1 = NULL, *automata2 = NULL;
				char last_operator = '\0';
				int found_closing_parenthesis = 0;
				while (stack) {
					if (stack->type == OPERATOR) {
						if (stack->element.operator == '(') {
							found_closing_parenthesis = 1;
							printf("found_closing_parenthesis = 1\n");
							if (last_operator == '|') {
								automata2 = new_union_automata(automata1, automata2);
								printf("automata2 = new_union_automata\n");
							}
							stack = stack->previous;//REMOVE '('
							break;
						} else if (stack->element.operator == '|') {
							last_operator = '|';
							printf("last_operator = '|'\n");
							if (automata1) {
								automata2 = new_union_automata(automata1, automata2);
								printf("automata2 = new_union_automata\n");
								automata1 = NULL;
							}
						}
					} else if (stack->type == AUTOMATA) {
						if (!automata2) {
							automata2 = stack->element.automata;
							printf("automata2 = stack->element.automata\n");
						} else {
							if (last_operator == '\0') {
								automata2 = new_concatenate_automata(stack->element.automata, automata2);
								printf("automata2 = new_concatenate_automata\n");
							} else if (last_operator == '|') {
								if (automata1) {
									automata1 = new_concatenate_automata(stack->element.automata, automata1);
									printf("automata1 = new_concatenate_automata\n");
								} else {
									automata1 = stack->element.automata;
									printf("automata1 = stack->element.automata\n");
								}
							}
						}
					}
					stack = stack->previous;
				}
				if (!found_closing_parenthesis) {
					printf("Closing parenthesis not found in position %d.\n", i);
					return NULL;
				}
				struct regex_stack *new_element = (struct regex_stack *) malloc(sizeof(struct regex_stack));
				new_element->type = AUTOMATA;
				new_element->element.automata = automata2;
				new_element->previous = stack;
				stack = new_element;
				break;
			}
			default: {
				struct regex_stack *new_element = (struct regex_stack *) malloc(sizeof(struct regex_stack));
				new_element->type = AUTOMATA;
				new_element->element.automata = new_single_symbol_automata(c);
				new_element->previous = stack;
				stack = new_element;
				printf("Adding automata for character %c\n", c);
				break;
			}
		}
	}
	if (stack->type != AUTOMATA) {
		printf("Top of the stack should be an automata.\n");
	}
	if (stack->previous != NULL) {
		printf("Top of the stack should have only one element.\n");
	}
	return stack->element.automata;
}

void tests() {
	char *entry;
	int match;
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
	entry = "abbbbabaab";
	match = match_automata(automata_kleene_a_a_b, entry);
	printf("Did match for entry '%s'? %s\n", entry, match ? "Yes" : "No");//Yes because ends in aab
	entry = "aaba";
	match = match_automata(automata_kleene_a_a_b, entry);
	printf("Did match for entry '%s'? %s\n", entry, match ? "Yes" : "No");//Yes because the automata reach the final state
	entry = "aaaab";
	match = match_automata(automata_kleene_a_a_b, entry);
	printf("Did match for entry '%s'? %s\n", entry, match ? "Yes" : "No");//Yes because ends in aab
	entry = "abbbbabab";
	match = match_automata(automata_kleene_a_a_b, entry);
	printf("Did match for entry '%s'? %s\n", entry, match ? "Yes" : "No");//No because ends in bab and the automata does not reach the final state
	entry = "abbbbabaabaaa";
	match = match_automata(automata_kleene_a_a_b, entry);
	printf("Did match for entry '%s'? %s\n", entry, match ? "Yes" : "No");//Yes because the automata reach the final state
}

int main(int argc, char *argv[]) {
	char *regex = argv[1];
	char *entry = argv[2];
	printf("%s, %s\n", regex, entry);
	struct automata *test_automata = build_automata(regex);
	int match = match_automata(test_automata, entry);
	printf("Did match for entry '%s'? %s\n", entry, match ? "Yes" : "No");//Yes because ends in aab
	tests();
	
	return EXIT_SUCCESS;
}
