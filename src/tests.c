#include "regex.h"

void tests() {
	char *regex = "kf(t|se)bb*";
	char *entry = "kfse";
	struct automata *automata = build_automata(regex);
	if (automata) {
		int match = match_automata(automata, entry);
		printf("Regex '%s'; Entry '%s'; %s\n", regex, entry, match ? "MATCH" : "NOT MATCH");
	}
}

int main(int argc, char *argv[]) {
	tests();
	return EXIT_SUCCESS;
}
