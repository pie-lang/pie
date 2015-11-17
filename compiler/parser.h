#ifndef __PIE_PARSER__
#define __PIE_PARSER__

#include <string>

#include "compiler/ast.h"

namespace pie { namespace compiler {

class Scanner;

struct Token {
};

struct Location {
};


class Parser {
public:
	Parser(Scanner &s);

	int scan(Token *token);
	int parse();
	void parseFatal(std::string msg);

	void onModule(std::string name);
	void onImport(std::string name, int visibility);
	void onFunctionStart(std::string name, int visibility);
	void onFunctionEnd();

public:
	Scanner &scanner;

	ModuleNode *module;			// current parsed module
	FunctionNode *function;		// current parsed function
};

}}

#endif
