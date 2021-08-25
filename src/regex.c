#include <stdlib.h>
#include <string.h>
#include "../headers/data_structures.h"
#include "../headers/automata.h"
#include "../headers/regex.h"
#include "../headers/log.h"

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
	return match_automata(regex->automata, entry);
}

struct regex_matches *matches_regex(struct regex *regex, char *entry) {
	struct regex_matches *result = (struct regex_matches *)malloc(sizeof(struct regex_matches));
	result->n = 0;
	result->matches = NULL;
	int start_pos;
	int entry_length = strlen(entry);
	for (start_pos = 0; start_pos < entry_length;) {
		int end_pos = match_automata2(regex->automata, entry, start_pos);
		if (end_pos - start_pos > 0) {
			result->n++;
			result->matches = reallocarray(result->matches, sizeof(struct regex_match), result->n);
			result->matches[result->n-1].start_pos = start_pos;
			result->matches[result->n-1].end_pos = end_pos;
			start_pos = end_pos;
		} else {
			start_pos++;
		}
	}
	if (result->n) {
		return result;
	}
	return NULL;
}

void free_regex_matches(struct regex_matches *regex_matches) {
	if (regex_matches->matches) {
		free(regex_matches->matches);
	}
	free(regex_matches);
}

struct automata *build_automata(char *regex, int *pos) {
	char c;
	struct regex_stack *stack = NULL;
	int special_char;
	for (c = regex[*pos];; c = regex[++*pos]) {
		special_char = c == '\\';
		switch (c) {
			//TODO handle not only {n} but also {n,}, {,n} and {n,k}
			case '{': {
				int open_bracket_pos = *pos;
				if (!stack) {
					log("Left hand side operand expected for operator '{' in position %d", open_bracket_pos);
					return NULL;
				} else if (stack->type == OPERATOR) {
					log("Found '{' after '%c' in position %d.\n", stack->element.operator, open_bracket_pos);
					free_regex_stack(stack);
					return NULL;
				}
				int i, comma_pos = -1, found_closing_bracket = 0;
				for (i = open_bracket_pos + 1; regex[i] != '\0'; i++) {
					if (regex[i] == '}') {
						found_closing_bracket = 1;
						break;
					} else if (regex[i] == ',' && comma_pos == -1) {
						comma_pos = i;
					} else if (regex[i] < '0' || regex[i] > '9') {
						log("Found '%c' instead of a number in position %d.\n", regex[i], i);
						free_regex_stack(stack);
						return NULL;
					}
				}
				if (!found_closing_bracket) {
					log("Closing bracket '}' not found after '{' in position %d.\n", open_bracket_pos);
					free_regex_stack(stack);
					return NULL;
				}
				int closing_bracket_pos = i;
				if (open_bracket_pos + 1 == closing_bracket_pos || (comma_pos != -1 && open_bracket_pos + 2 == closing_bracket_pos)) {
					log("At leat one number must be defined between '{' and '}' at position %d.\n", open_bracket_pos);
					free_regex_stack(stack);
					return NULL;
				}
				if (comma_pos == -1) {
					int begin_number = open_bracket_pos + 1;
					int n_numbers = closing_bracket_pos - begin_number;
					char *str_number = malloc(n_numbers + 1);
					strncpy(str_number, &regex[begin_number], n_numbers);
					str_number[n_numbers] = '\0';
					int repetitions = atoi(str_number);
					free(str_number);
					
					struct automata **copies = copy_automata_n_times(stack->element.automata, repetitions);
					struct automata *new_automata = concatenate_automatas(copies, repetitions);
					free(copies);
					free_automata(stack->element.automata);
					stack->element.automata = new_automata;
				} else if (comma_pos == open_bracket_pos + 1) {
					int begin_number = open_bracket_pos + 2;
					int n_numbers = closing_bracket_pos - begin_number;
					char *str_number = malloc(n_numbers + 1);
					strncpy(str_number, &regex[begin_number], n_numbers);
					str_number[n_numbers] = '\0';
					int repetitions = atoi(str_number);
					free(str_number);
					
					struct automata **copies = copy_automata_n_times(stack->element.automata, repetitions);
					for (i = 0; i < repetitions; i++) {
						copies[i] = new_optional_automata(copies[i]);
					}
					struct automata *new_automata = concatenate_automatas(copies, repetitions);
					free(copies);
					free_automata(stack->element.automata);
					stack->element.automata = new_automata;
				} else if (comma_pos == closing_bracket_pos - 1) {
					int begin_number = open_bracket_pos + 1;
					int n_numbers = closing_bracket_pos - begin_number - 1;
					char *str_number = malloc(n_numbers + 1);
					strncpy(str_number, &regex[begin_number], n_numbers);
					str_number[n_numbers] = '\0';
					int repetitions = atoi(str_number);
					free(str_number);
					
					struct automata **copies = copy_automata_n_times(stack->element.automata, repetitions + 1);
					struct automata *new_automata = concatenate_automatas(copies, repetitions);
					new_automata = new_concatenate_automata(new_automata, new_kleene_automata(copies[repetitions]));
					free(copies);
					free_automata(stack->element.automata);
					stack->element.automata = new_automata;
				} else {
					int begin_number = open_bracket_pos + 1;
					int n_numbers = comma_pos - begin_number;
					char *str_number = malloc(n_numbers + 1);
					strncpy(str_number, &regex[begin_number], n_numbers);
					str_number[n_numbers] = '\0';
					int from_repetitions = atoi(str_number);
					free(str_number);
					
					begin_number = comma_pos + 1;
					n_numbers = closing_bracket_pos - begin_number;
					str_number = malloc(n_numbers + 1);
					strncpy(str_number, &regex[begin_number], n_numbers);
					str_number[n_numbers] = '\0';
					int to_repetitions = atoi(str_number);
					free(str_number);

					int repetitions = from_repetitions;

					struct automata **copies = copy_automata_n_times(stack->element.automata, repetitions);
					struct automata *new_automata = concatenate_automatas(copies, repetitions);
					free(copies);

					repetitions = to_repetitions - from_repetitions;

					copies = copy_automata_n_times(stack->element.automata, repetitions);
					for (i = 0; i < repetitions; i++) {
						copies[i] = new_optional_automata(copies[i]);
					}
					new_automata = new_concatenate_automata(new_automata, concatenate_automatas(copies, repetitions));
					free(copies);
					free_automata(stack->element.automata);
					stack->element.automata = new_automata;
				}
				
				*pos = closing_bracket_pos;
				break;
			}
			case '[': {
				int i = 0, found_closing_bracket = 0;
				for (i = *pos + 1; regex[i] != '\0'; i++) {
					if (regex[i] == ']' && regex[i-1] != '\\') {
						found_closing_bracket = 1;
						break;
					}
				}
				if (!found_closing_bracket) {
					log("%s", "Closing bracket ']' not found.\n");
					free_regex_stack(stack);
					return NULL;
				}
				if (regex[*pos + 1] == ']') {
					++*pos;
					break;
				}
				int first_idx = *pos + 1;
				int last_idx;
				for (last_idx = first_idx; regex[last_idx + 1] != ']' || regex[last_idx] == '\\'; last_idx++);
				int idx;
				struct array_list *automatas = alloc_array_list(sizeof(struct automata *));
				for (idx = first_idx; idx <= last_idx;) {
					if (regex[idx] == '\\') {
						idx++;
					}
					struct automata *a;
					if (regex[idx + 1] == '-' && regex[idx + 2] != ']') {
						char c = regex[idx];
						while (c <= regex[idx + 2]) {
							a = new_single_symbol_automata(c);
							add_to_array_list(automatas, &a);
							c++;
						}
						idx += 3;
					} else {
						a = new_single_symbol_automata(regex[idx]);
						add_to_array_list(automatas, &a);
						idx++;
					}
				}
				struct automata *a = *(struct automata **)array_list_get(automatas, 0);
				for (i = 1; i < length_array_list(automatas); i++) {
					a = new_union_automata(a, *(struct automata **)array_list_get(automatas, i));
				}
				free_array_list(automatas);
				struct regex_stack *new_element = (struct regex_stack *) malloc(sizeof(struct regex_stack));
				new_element->type = AUTOMATA;
				new_element->element.automata = a;
				new_element->previous = stack;
				stack = new_element;
				*pos = last_idx + 1;
				break;
			}
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
				if (special_char) {
					c = regex[++*pos];
				}
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
	char *new_regex = (char *)malloc(regex_length + 3 * sizeof(char));
	new_regex[0] = '(';
	strncpy(new_regex + 1, regex, regex_length);
	new_regex[regex_length + 1] = ')';
	new_regex[regex_length + 2] = '\0';
	int pos = 1;
	log("Compiling regex '%s'.", regex);
	struct automata *automata = build_automata(new_regex, &pos);
	free(new_regex);
	if (automata) {
		int new_regex_length = regex_length + 2;
		if (pos + 1 < new_regex_length) {
			log("Unmatched parenthesis in position %d.", pos);
			free_automata(automata);
			return NULL;
		}
		struct regex *p_regex = (struct regex *)malloc(sizeof(struct regex));
		p_regex->regex = (char *)malloc(regex_length + 1);
		strcpy(p_regex->regex, regex);
		p_regex->automata = automata;
		return p_regex;
	}
	return NULL;
}

void free_regex(struct regex *p_regex) {
	if (p_regex) {
		free_automata(p_regex->automata);
		free(p_regex->regex);
		free(p_regex);
	}
}
