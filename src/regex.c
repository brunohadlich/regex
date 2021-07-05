#include <stdlib.h>
#include <string.h>
#include "automata.h"
#include "regex.h"
#include "log.h"

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

void free_regex_stack(struct regex_stack *stack) {
	struct regex_stack *old_element;
	while (stack) {
		if (stack->type == AUTOMATA && stack->element.automata) {
			free_automata(stack->element.automata);
		}
		old_element = stack;
		stack = stack->previous;
		free(old_element);
	}
}

int match_regex(struct regex *regex, char *entry) {
	return traverse_state(regex->automata->initial, entry, 0);
}

struct automata *build_automata(char *regex, int *pos) {
	char c;
	struct regex_stack *stack = NULL;
	for (c = regex[*pos]; ; c = regex[++*pos]) {
		switch (c) {
			case '|': {
				if (!stack) {
					log("Left hand side operand expected for operator '|' in position %d", *pos);
					return NULL;
				} else if (stack->type == OPERATOR) {
					log("Found '|' after '%c' in position %d.\n", stack->element.operator, *pos);
					free_regex_stack(stack);
					return NULL;
				}
				struct regex_stack *new_element = (struct regex_stack *) malloc(sizeof(struct regex_stack));
				new_element->type = OPERATOR;
				new_element->element.operator = '|';
				new_element->previous = stack;
				stack = new_element;
				break;
			}
			case '?': {
				if (!stack) {
					log("Left hand side operand expected for operator '?' in position %d.\n", *pos);
					return NULL;
				} else if (stack->type == OPERATOR) {
					log("Found '?' after '%c' in position %d.\n", stack->element.operator, *pos);
					free_regex_stack(stack);
					return NULL;
				}
				stack->element.automata = new_optional_automata(stack->element.automata);
				break;
			}
			case '+': {
				if (!stack) {
					log("Left hand side operand expected for operator '+' in position %d.\n", *pos);
					return NULL;
				} else if (stack->type == OPERATOR) {
					log("Found '+' after '%c' in position %d.\n", stack->element.operator, *pos);
					free_regex_stack(stack);
					return NULL;
				}
				stack->element.automata = new_repetition_automata(stack->element.automata);
				break;
			}
			case '*': {
				if (!stack) {
					log("Left hand side operand expected for operator '*' in position %d.\n", *pos);
					return NULL;
				} else if (stack->type == OPERATOR) {
					log("Found '*' after '%c' in position %d.\n", stack->element.operator, *pos);
					free_regex_stack(stack);
					return NULL;
				}
				stack->element.automata = new_kleene_automata(stack->element.automata);
				break;
			}
			case '(': {
				*pos = *pos + 1;
				struct automata *automata = build_automata(regex, pos);
				if (automata) {
					struct regex_stack *new_element = (struct regex_stack *) malloc(sizeof(struct regex_stack));
					new_element->type = AUTOMATA;
					new_element->element.automata = automata;
					new_element->previous = stack;
					stack = new_element;
				}
				break;
			}
			case ')': {
				if (!stack) {
					return NULL;
				}
				if (stack->type == OPERATOR) {
					log("Found ')' after '%c' in position %d.\n", stack->element.operator, *pos);
					free_regex_stack(stack);
					return NULL;
				}
				struct automata *automata1 = NULL, *automata2 = NULL;
				char last_operator = ')';
				while (stack) {
					if (stack->type == OPERATOR && stack->element.operator == '|') {
						last_operator = '|';
						if (automata1) {
							automata2 = new_union_automata(automata1, automata2);
							automata1 = NULL;
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
					struct regex_stack *previous_element = stack->previous;
					free(stack);
					stack = previous_element;
				}
				if (last_operator == '|') {
					automata2 = new_union_automata(automata1, automata2);
					automata1 = NULL;
				}
				return automata2;
			}
			case '\0': {
				log("Missing closing group parenthesis ')' in position %d.\n", *pos);
				free_regex_stack(stack);
				return NULL;
			}
			default: {
				struct regex_stack *new_element = (struct regex_stack *) malloc(sizeof(struct regex_stack));
				new_element->type = AUTOMATA;
				new_element->element.automata = new_single_symbol_automata(c);
				new_element->previous = stack;
				stack = new_element;
				break;
			}
		}
	}
}

struct regex *build_regex(char *regex) {
	int regex_length;
	if (regex == NULL || (regex_length = strlen(regex)) == 0) {
		return NULL;
	}
	char *new_regex = (char *)malloc(regex_length + 3);
	new_regex[0] = '(';
	strncpy(new_regex + 1, regex, regex_length);
	new_regex[regex_length + 1] = ')';
	int pos = 1;
	log("Compiling regex '%s'.", regex);
	struct automata *automata = build_automata(new_regex, &pos);
	if (automata) {
		int new_regex_length = regex_length + 2;
		if (pos + 1 < new_regex_length) {
			log("Unmatched parenthesis in position %d.", pos);
			free_automata(automata);
			return NULL;
		}
		struct regex *p_regex = (struct regex *)malloc(sizeof(struct regex));
		p_regex->automata = automata;
		return p_regex;
	}
	return NULL;
}

void free_regex(struct regex *regex) {
	free_automata(regex->automata);
	free(regex);
}
