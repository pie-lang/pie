#include <stdio.h>

#include "compiler/scanner.h"
#include "compiler/parser.h"

int main(int argc, char **argv)
{
	pie::compiler::Scanner scanner;
	scanner.scan();

	pie::compiler::Parser parser;
	parser.parse();
	return 0;
}
