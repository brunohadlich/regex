#include <stdio.h>
#include <stdlib.h>
#include "../headers/regex.h"
#include "../headers/automata.h"

void print_test_title(char *title) {
	printf("\n=================== %s ===================\n\n", title);
}

void tests_automata_manually() {
	print_test_title("Test assembling automata manually");
	char *regex = "kf(t|se)bb*";
	char *entry = "kfseb";
	struct automata *s2 = new_single_symbol_automata('k');
	struct automata *s3 = new_single_symbol_automata('f');
	struct automata *s4 = new_single_symbol_automata('t');
	struct automata *s5 = new_single_symbol_automata('s');
	struct automata *s6 = new_single_symbol_automata('e');
	struct automata *s7 = new_single_symbol_automata('b');
	struct automata *s8 = new_single_symbol_automata('b');
	struct automata *s9 = new_concatenate_automata(s2, s3);
	struct automata *s10 = new_concatenate_automata(s5, s6);
	struct automata *s11 = new_union_automata(s4, s10);
	struct automata *s12 = new_concatenate_automata(s9, s11);
	struct automata *s13 = new_concatenate_automata(s12, s7);
	struct automata *s14 = new_kleene_automata(s8);
	struct automata *s15 = new_concatenate_automata(s13, s14);
	entry = "kfsebbb";
	int success = 1;
	if (match_automata2(s15, entry, 0) != 7) {
		printf("Entry '%s' should match automata '%s'.\n", entry, regex);
		success = 0;
	}
	entry = "kftb";
	if (match_automata2(s15, entry, 0) != 4) {
		printf("Entry '%s' should match automata '%s'.\n", entry, regex);
		success = 0;
	}
	entry = "kftbbb";
	if (match_automata2(s15, entry, 0) != 6) {
		printf("Entry '%s' should match automata '%s'.\n", entry, regex);
		success = 0;
	}
	entry = "kft";
	if (match_automata2(s15, entry, 0) != 0) {
		printf("Entry '%s' should NOT match automata '%s'.\n", entry, regex);
		success = 0;
	}
	entry = "ftseb";
	if (match_automata2(s15, entry, 0) != 0) {
		printf("Entry '%s' should NOT match automata '%s'.\n", entry, regex);
		success = 0;
	}
	if (success) {
		printf("\nTest succeeded.\n");
	} else {
		printf("\nTest failed.\n");
	}
}

struct regex_test_entry {
	char *regex;
	char *entry;
	struct regex_matches *matches;
};

char *build_regex_should_break_testing_set[] = {
	"ab[fj-noo-q|ww",
	"ab{0x10}cc",
	"ab[ceg-j\\]|ww",
	"()|xbb",
	"a|xbb)))",
	"(a|xbb)))",
	"((a|xbb)))",
	"(a|xbb)|sdw))",
	NULL
};

#define make_regex_matches(n_elements, ...) (&(struct regex_matches){.n = n_elements, .matches = (&(struct regex_match [n_elements]){__VA_ARGS__})})

struct regex_test_entry match_testing_set[] = {
	{"kf(t|se)bb*", "kfseb", make_regex_matches(1, {.start_pos = 0, .end_pos = 5})},
	{"k*f(t|se)bb", "kfsebb", make_regex_matches(1, {.start_pos = 0, .end_pos = 6})},
	{"k*f(t|se)bb", "kftbb", make_regex_matches(1, {.start_pos = 0, .end_pos = 5})},
	{"k*f(t|se)bb", "fsebb", make_regex_matches(1, {.start_pos = 0, .end_pos = 5})},
	{"k*f(t|se)bb", "sebb", NULL},
	{"k*f()|xbb", "fbb", make_regex_matches(1, {.start_pos = 0, .end_pos = 1})},
	{"k*f()|xbb", "fbbfnf", make_regex_matches(3, {.start_pos = 0, .end_pos = 1}, {.start_pos = 3, .end_pos = 4}, {.start_pos = 5, .end_pos = 6})},
	{"k*f()|xbb", "bb", NULL},
	{"aba", "abaaba", make_regex_matches(2, {.start_pos = 0, .end_pos = 3}, {.start_pos = 3, .end_pos = 6})},
	{"\\(\\)|xbb", "bb", NULL},
	{"a|xbb\\)\\)\\)", "bb", NULL},
	{"(a|xbb)\\)\\)", "bb", NULL},
	{"((a|xbb))\\)", "bb", NULL},
	{"(a|xbb)|sdw\\)\\)", "bb", NULL},
	{"abc?d", "abd", make_regex_matches(1, {.start_pos = 0, .end_pos = 3})},
	{"abc?d", "abcd", make_regex_matches(1, {.start_pos = 0, .end_pos = 4})},
	{"abc?d", "abccd", NULL},
	{"abc+d", "abd", NULL},
	{"abc+d", "abcd", make_regex_matches(1, {.start_pos = 0, .end_pos = 4})},
	{"abc+d", "abccd", make_regex_matches(1, {.start_pos = 0, .end_pos = 5})},
	{"a*", "abccd", make_regex_matches(1, {.start_pos = 0, .end_pos = 1})},
	{"asd", "qwe", NULL},
	{"l|e*tqs(3|or*)", "l", make_regex_matches(1, {.start_pos = 0, .end_pos = 1})},
	{"l|e*tqs(3|or*)", "e", NULL},
	{"l|e*tqs(3|or*)", "tqso", make_regex_matches(1, {.start_pos = 0, .end_pos = 4})},
	{"ab[fj-n]", "abf", make_regex_matches(1, {.start_pos = 0, .end_pos = 3})},
	{"ab[fj-n]", "abi", NULL},
	{"ab[fj-n]", "abj", make_regex_matches(1, {.start_pos = 0, .end_pos = 3})},
	{"ab[fj-n]", "abl", make_regex_matches(1, {.start_pos = 0, .end_pos = 3})},
	{"ab[fj-n]", "abn", make_regex_matches(1, {.start_pos = 0, .end_pos = 3})},
	{"ab[fj-n]", "abo", NULL},
	{"ab[fj-no]", "abo", make_regex_matches(1, {.start_pos = 0, .end_pos = 3})},
	{"ab[fj-noo-q]", "abp", make_regex_matches(1, {.start_pos = 0, .end_pos = 3})},
	{"ab[fj-noo-q]|ww", "ww", make_regex_matches(1, {.start_pos = 0, .end_pos = 2})},
	{"ab[f-]|ww", "abf", make_regex_matches(1, {.start_pos = 0, .end_pos = 3})},
	{"ab[f-]|ww", "ab-", make_regex_matches(1, {.start_pos = 0, .end_pos = 3})},
	{"ab[f-]|ww", "abp", NULL},
	{"xka{3}b", "xkaaabp", make_regex_matches(1, {.start_pos = 0, .end_pos = 6})},
	{"xka{3}b", "xkaabp", NULL},
	{"x(ka){4}b", "xkakakakabp", make_regex_matches(1, {.start_pos = 0, .end_pos = 10})},
	{"x(ka){3}b", "xkakabp", NULL},
	{"ab[f\\fg-j]|ww", "abf", make_regex_matches(1, {.start_pos = 0, .end_pos = 3})},
	{"ab[f\\fg-j]|ww", "abg", make_regex_matches(1, {.start_pos = 0, .end_pos = 3})},
	{"ab[f\\fg-j]|ww", "abi", make_regex_matches(1, {.start_pos = 0, .end_pos = 3})},
	{"ab[f\\fg-j]|ww", "abj", make_regex_matches(1, {.start_pos = 0, .end_pos = 3})},
	{"ab[f\\fg-j]|ww", "abk", NULL},
	{"ab\\[ceg-j]|ww", "ab[ceg-j]", make_regex_matches(1, {.start_pos = 0, .end_pos = 9})},
	{"ab[ceg-j]|ww", "abi", make_regex_matches(1, {.start_pos = 0, .end_pos = 3})},
	{"ab\\{\\(|ww", "ab{(", make_regex_matches(1, {.start_pos = 0, .end_pos = 4})},
	{"ab\\{\\(|ww", "ab{", NULL},
	{"ab\\{\\(\\[\\*\\+\\?\\||ww", "ab{([*+?|", make_regex_matches(1, {.start_pos = 0, .end_pos = 9})},
	{"ab{10}cc", "abbbbbbbbbbcc", make_regex_matches(1, {.start_pos = 0, .end_pos = 13})},
	{"x(ka){3,}b", "xb", NULL},
	{"x(ka){3,}b", "xkab", NULL},
	{"x(ka){3,}b", "xkakab", NULL},
	{"x(ka){3,}b", "xkakakab", make_regex_matches(1, {.start_pos = 0, .end_pos = 8})},
	{"x(ka){3,}b", "xkakakakab", make_regex_matches(1, {.start_pos = 0, .end_pos = 10})},
	{"x(ka){3,}b", "xkakakakakab", make_regex_matches(1, {.start_pos = 0, .end_pos = 12})},
	{"x(ka){,3}b", "xb", make_regex_matches(1, {.start_pos = 0, .end_pos = 2})},
	{"x(ka){,3}b", "xkab", make_regex_matches(1, {.start_pos = 0, .end_pos = 4})},
	{"x(ka){,3}b", "xkakab", make_regex_matches(1, {.start_pos = 0, .end_pos = 6})},
	{"x(ka){,3}b", "xkakakab", make_regex_matches(1, {.start_pos = 0, .end_pos = 8})},
	{"x(ka){,3}b", "xkakakakab", NULL},
	{"x(ka){,3}b", "xkakakakakab", NULL},
	{"x(ka){2,5}b", "xb", NULL},
	{"x(ka){2,5}b", "xkab", NULL},
	{"x(ka){2,5}b", "xkakab", make_regex_matches(1, {.start_pos = 0, .end_pos = 6})},
	{"x(ka){2,5}b", "xkakakab", make_regex_matches(1, {.start_pos = 0, .end_pos = 8})},
	{"x(ka){2,5}b", "xkakakakab", make_regex_matches(1, {.start_pos = 0, .end_pos = 10})},
	{"x(ka){2,5}b", "xkakakakakab", make_regex_matches(1, {.start_pos = 0, .end_pos = 12})},
	{"x(ka){2,5}b", "xkakakakakakab", NULL},
	{"x(ka){2,5}b", "xkakakakakakakab", NULL},
	{NULL, NULL, NULL}
};

void test_build_regex() {
	print_test_title("Test build_regex() function");
	int i, success = 1;
	for (i = 0; build_regex_should_break_testing_set[i] != NULL; i++) {
		char *regex = build_regex_should_break_testing_set[i];
		struct regex *p_regex = build_regex(regex);
		if (p_regex) {
			printf("Regex '%s' should have failed to compile.\n", regex);
			free_regex(p_regex);
			success = 0;
		}
	}
	for (i = 0; match_testing_set[i].regex != NULL; i++) {
		char *regex = match_testing_set[i].regex;
		struct regex *p_regex = build_regex(regex);
		if (!p_regex) {
			printf("Regex '%s' should have compiled successfully.\n", regex);
			success = 0;
		} else {
			free_regex(p_regex);
		}
	}
	if (success) {
		printf("\nTest succeeded.\n");
	} else {
		printf("\nTest failed.\n");
	}
}

int test_regex(struct regex *p_regex, char *entry, struct regex_matches *expected_matches) {
	int i;
	struct regex_matches *matches = matches_regex(p_regex, entry);
	if (matches != NULL && expected_matches == NULL) {
		printf("Entry '%s' should NOT match regex '%s'.\n", entry, p_regex->regex);
		goto error;
	}
	if (matches == NULL && expected_matches != NULL) {
		printf("Entry '%s' SHOULD match regex '%s'.\n", entry, p_regex->regex);
		goto error;
	}
	if (matches) {
		if (matches->n != expected_matches->n) {
			printf("Entry '%s' SHOULD match regex '%s' %d times but it did %d times.\n", entry, p_regex->regex, expected_matches->n, matches->n);
			goto error;
		}
		for (i = 0; i < matches->n; i++) {
			if (matches->matches[i].start_pos != expected_matches->matches[i].start_pos) {
				printf("Start match position differ from expected. Expected %d, found%d.\n", expected_matches->matches[i].start_pos, matches->matches[i].start_pos);
				goto error;
			}
			if (matches->matches[i].end_pos != expected_matches->matches[i].end_pos) {
				printf("End match position differ from expected. Expected %d, found%d.\n", expected_matches->matches[i].end_pos, matches->matches[i].end_pos);
				goto error;
			}
		}
	}
	return 0;
	error:
		if (matches) {
			free_regex_matches(matches);
		}
		return 1;
}

void test_regex_nfa() {
	print_test_title("Test regex to NFA construction");
	int i, success = 1;
	for (i = 0; match_testing_set[i].regex != NULL; i++) {
		char *regex = match_testing_set[i].regex;
		char *entry = match_testing_set[i].entry;
		struct regex_matches *matches = match_testing_set[i].matches;
		struct regex *p_regex = build_regex(regex);
		if (p_regex) {
			if (test_regex(p_regex, entry, matches)) {
				success = 0;
			}
			free_regex(p_regex);
		} else {
			printf("Failed to build regex '%s'.\n", regex);
		}
	}
	if (success) {
		printf("\nTest succeeded.\n");
	} else {
		printf("\nTest failed.\n");
	}
}

void test_regex_dfa() {
	print_test_title("Test regex to NFA to DFA construction");
	int i, success = 1;
	for (i = 0; match_testing_set[i].regex != NULL; i++) {
		char *regex = match_testing_set[i].regex;
		char *entry = match_testing_set[i].entry;
		struct regex_matches *matches = match_testing_set[i].matches;
		struct regex *p_regex = build_regex(regex);
		if (p_regex) {
			struct automata *dfa = nfa_to_dfa(p_regex->automata);
			free_automata(p_regex->automata);
			p_regex->automata = dfa;
			if (test_regex(p_regex, entry, matches)) {
				success = 0;
			}
			free_regex(p_regex);
		} else {
			printf("Failed to build regex '%s'.\n", regex);
		}
	}
	if (success) {
		printf("\nTest succeeded.\n");
	} else {
		printf("\nTest failed.\n");
	}
}

int main(int argc, char *argv[]) {
	tests_automata_manually();
	test_build_regex();
	test_regex_nfa();
	test_regex_dfa();

	return EXIT_SUCCESS;
}
