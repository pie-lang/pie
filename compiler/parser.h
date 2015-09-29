#ifndef __PIE_PARSER__
#define __PIE_PARSER__

#include <string>

namespace pie { namespace compiler {

class Scanner;

struct Token {
};

struct Location {
};


class Parser {
public:
	Parser(Scanner &s): scanner(s) {}
	int scan(Token *token);
	void parse();
	void parseFatal(std::string msg);

public:
	Scanner &scanner;
};

}}

#endif
