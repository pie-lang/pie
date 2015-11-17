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

	int ret = parser.parse();

	if (file) {
		fclose(file);
	}

	if (ret != 0) {
		fprintf(stderr, "failed to compile\n");	
		exit(2);
	}

	Node *module = parser.module;

	// Walk the AST tree and print it
	PrintVisitor visitor;
	module->visit(&visitor);

	// stdout << visitor;


	// TODO vm compiler compile to IR or bytecode directly?

	return 0;
}
