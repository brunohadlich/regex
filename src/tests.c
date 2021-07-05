#include <stdio.h>
#include <stdlib.h>
#include "regex.h"
#include "automata.h"

void tests_automata_manually() {
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
	struct regex *p_regex = (struct regex *)malloc(sizeof(struct regex));
	p_regex->automata = s15;
	char *entry = "kfse";
	int match = match_regex(p_regex, entry);
	free_regex(p_regex);
	char *regex = "kf(t|se)bb*";
	printf("Regex '%s'; Entry '%s'; %s\n", regex, entry, match ? "MATCH" : "NOT MATCH");
}

void test_regex(char *regex, char *entry) {
	struct regex *p_regex = build_regex(regex);
	if (p_regex) {
		int match = match_regex(p_regex, entry);
		free_regex(p_regex);
		printf("Regex '%s'; Entry '%s'; %s.\n", regex, entry, match ? "MATCH" : "NOT MATCH");
	} else {
		printf("Failed to build regex '%s'.\n", regex);
	}
}

int main(int argc, char *argv[]) {
	tests_automata_manually();
	test_regex("kf(t|se)bb*", "kfseb");
	test_regex("k*f(t|se)bb", "kfsebb");
	test_regex("k*f(t|se)bb", "kftbb");
	test_regex("k*f(t|se)bb", "fsebb");
	test_regex("k*f(t|se)bb", "sebb");
	test_regex("k*f(t|se)bb", "kfseb");
	test_regex("k*f()|xbb", "fbb");
	test_regex("k*f()|xbb", "bb");
	test_regex("()|xbb", "bb");
	test_regex("a|xbb)))", "bb");
	test_regex("(a|xbb)))", "bb");
	test_regex("((a|xbb)))", "bb");
	test_regex("(a|xbb)|sdw))", "bb");
	test_regex("abc?d", "abd");
	test_regex("abc?d", "abcd");
	test_regex("abc?d", "abccd");
	test_regex("abc+d", "abd");
	test_regex("abc+d", "abcd");
	test_regex("abc+d", "abccd");
	return EXIT_SUCCESS;
}
