#include <stdlib.h>
#include <string.h>
#include "automata.h"
#include "log.h"

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

int match(struct automata *a, char *entry) {
	return traverse_state(a->initial, entry, 0);
}

struct automata *build(char *regex) {
	int regex_length = strlen(regex);
	if (regex_length == 0) {
		return NULL;
	}
	int i, minimum_reps = -1, maximum_reps = -1;
	const int BUFFER_SIZE = 7;
	char minimum_reps_buffer[BUFFER_SIZE];
	minimum_reps_buffer[0] = '\0';
	char maximum_reps_buffer[BUFFER_SIZE];
	maximum_reps_buffer[0] = '\0';
	struct regex_stack *stack = NULL;
	for (i = 0; i <= regex_length; i++) {
		char c = regex[i];
		switch (c) {
			case '|': {
				if (!stack) {
					4;
					log("Left hand side operand expected for operator '|' in position %d", i);
					return NULL;
				} else if (stack->type == OPERATOR) {
					log("Found '|' after '%c' in position %d.\n", stack->element.operator, i);
					return NULL;
				}
				struct regex_stack *new_element = (struct regex_stack *) malloc(sizeof(struct regex_stack));
				new_element->type = OPERATOR;
				new_element->element.operator = '|';
				new_element->previous = stack;
				stack = new_element;
				break;
			}
			case '*': {
				if (!stack) {
					log("Left hand side operand expected for operator '*' in position %d.\n", i);
					return NULL;
				} else if (stack->type == OPERATOR) {
					log("Found '*' after '%c' in position %d.\n", stack->element.operator, i);
					return NULL;
				}
				stack->element.automata = new_kleene_automata(stack->element.automata);
				break;
			}
			case '(': {
				if (!stack) {
					stack = (struct regex_stack *) malloc(sizeof(struct regex_stack));
					stack->type = OPERATOR;
					stack->element.operator = '(';
					stack->previous = NULL;
				} else {
					struct regex_stack *new_element = (struct regex_stack *) malloc(sizeof(struct regex_stack));
					new_element->type = OPERATOR;
					new_element->element.operator = '(';
					new_element->previous = stack;
					stack = new_element;
				}
				break;
			}
			case ')':
				if (stack->type == OPERATOR) {
					log("Found ')' after '%c' in position %d.\n", stack->element.operator, i);
					return NULL;
				}
				struct automata *automata1 = NULL, *automata2 = NULL;
				char last_operator = ')';
				int found_open_parenthesis = 0;
				while (stack) {
					if (stack->type == OPERATOR) {
						if (stack->element.operator == '(') {
							found_open_parenthesis = 1;
							if (last_operator == '|') {
								automata2 = new_union_automata(automata1, automata2);
							}
							stack = stack->previous;//REMOVE '('
							break;
						} else if (stack->element.operator == '|') {
							last_operator = '|';
							if (automata1) {
								automata2 = new_union_automata(automata1, automata2);
								automata1 = NULL;
							}
						}
					} else if (stack->type == AUTOMATA) {
						if (!automata2) {
							automata2 = stack->element.automata;
						} else {
							if (last_operator == ')') {
								automata2 = new_concatenate_automata(stack->element.automata, automata2);
							} else if (last_operator == '|') {
								if (automata1) {
									automata1 = new_concatenate_automata(stack->element.automata, automata1);
								} else {
									automata1 = stack->element.automata;
								}
							}
						}
					}
					stack = stack->previous;
				}
				if (!found_open_parenthesis) {
					log("There is no previous '(' to match symbol ')' in position %d.\n", i);
					return NULL;
				}
				struct regex_stack *new_element = (struct regex_stack *) malloc(sizeof(struct regex_stack));
				new_element->type = AUTOMATA;
				new_element->element.automata = automata2;
				new_element->previous = stack;
				stack = new_element;
				break;
			case '\0': {
				if (stack->type == OPERATOR) {
					log("Expression finished with wrong symbol %c.\n", stack->element.operator);
					return NULL;
				}
				struct automata *automata1 = NULL, *automata2 = NULL;
				char last_operator = '\0';
				while (stack) {
					if (stack->type == OPERATOR) {
						if (stack->element.operator == '(') {
							log("Missing closing group parenthesis ')' in position %d.\n", i);
							return NULL;
						} else if (stack->element.operator == '|') {
							last_operator = '|';
							if (automata1) {
								automata2 = new_union_automata(automata1, automata2);
								automata1 = NULL;
							}
						}
					} else if (stack->type == AUTOMATA) {
						if (!automata2) {
							automata2 = stack->element.automata;
						} else {
							if (last_operator == '\0') {
								automata2 = new_concatenate_automata(stack->element.automata, automata2);
							} else if (last_operator == '|') {
								if (automata1) {
									automata1 = new_concatenate_automata(stack->element.automata, automata1);
								} else {
									automata1 = stack->element.automata;
								}
							}
						}
					}
					if (!stack->previous) {
						break;
					}
					stack = stack->previous;
				}
				stack->type = AUTOMATA;
				stack->element.automata = automata2;
				break;
			}
			default: {
				if (!stack) {
					stack = (struct regex_stack *) malloc(sizeof(struct regex_stack));
					stack->type = AUTOMATA;
					stack->element.automata = new_single_symbol_automata(c);
					stack->previous = NULL;
				} else {
					struct regex_stack *new_element = (struct regex_stack *) malloc(sizeof(struct regex_stack));
					new_element->type = AUTOMATA;
					new_element->element.automata = new_single_symbol_automata(c);
					new_element->previous = stack;
					stack = new_element;
				}
				break;
			}
		}
	}
	return stack->element.automata;
}
