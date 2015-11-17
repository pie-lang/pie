#ifndef __PIE_AST_PRIMITIVE__
#define __PIE_AST_PRIMITIVE__

#include <string>

#include "compiler/ast/node.h"

namespace pie { namespace compiler {

/* Do I really need such a simple node? */
// Only Int Node for now.... for simplicity
class IntNode : Node 
{
public:
	int value;

	IntNode(int value) : value(value) {}

	DEFINE_VISIT(IntNode);
};


class StringNode : Node
{
public:
	std::string str;

	StringNode(std::string str) : str(str) {}

	DEFINE_VISIT(StringNode);
};

}}

#endif
