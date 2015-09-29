#include <stdio.h>

#include "compiler/scanner.h"
#include "compiler/parser.h"

int main(int argc, char **argv)
{
	// TODO read file from file
	pie::compiler::Scanner scanner;
	pie::compiler::Parser parser(scanner);

	parser.parse();
	return 0;
}
