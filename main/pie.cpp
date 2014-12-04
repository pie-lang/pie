#include <stdio.h>

#include "compiler/scanner.h"

int main(int argc, char **argv)
{
	pie::compiler::Scanner scanner;
	scanner.scan();
	return 0;
}
