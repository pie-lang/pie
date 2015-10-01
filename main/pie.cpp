#include <stdio.h>
#include <stdlib.h>

#include "compiler/scanner.h"
#include "compiler/parser.h"

int main(int argc, char **argv)
{
	FILE *file = NULL;

	if (argc >= 2) {
		file = fopen(argv[1], "r");

		if (!file) {
			fprintf(stderr, "filed to open file: %s\n", argv[1]);
			exit(1);
		}
	}

	// pass FILE directly for now
	pie::compiler::Scanner scanner(file);
	pie::compiler::Parser parser(scanner);

	parser.parse();

	fclose(file);

	return 0;
}
