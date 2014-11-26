#ifndef __PIE_PARSER__
#define __PIE_PARSER__

#include <string>

namespace pie { namespace parser {

struct Token {
};

struct Location {
};


class Parser {
public:
	int scan(Token *token, Location *loc);
	void parseFatal(Location *loc, std::string msg);

};

}}

#endif
