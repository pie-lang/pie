#ifndef __PIE_PARSER__
#define __PIE_PARSER__

#include <string>

namespace pie { namespace compiler {

struct Token {
};

struct Location {
};


class Parser {
public:
	void parse();
	void parseFatal(std::string msg);

};

}}

#endif
