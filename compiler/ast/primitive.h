#ifndef __PIE_AST_PRIMITIVE__
#define __PIE_AST_PRIMITIVE__

#include <string>

#include "compiler/ast/node.h"

namespace pie { namespace compiler {

class IntNode : public Node
{
public:
	int64_t value;

	IntNode(int64_t value) : value(value) {}

	DEFINE_VISIT(IntNode);
};

class DoubleNode : public Node
{
public:
	double value;

	DoubleNode(double value) : value(value) {}

	DEFINE_VISIT(DoubleNode);
};

class StringNode : public Node
{
public:
	std::string str;

	StringNode(const std::string &str) : str(str) {}

	DEFINE_VISIT(StringNode);
};

class IdentifierNode : public Node
{
public:
	std::string name;

	IdentifierNode(const std::string &name) : name(name) {}

	DEFINE_VISIT(IdentifierNode);
};

}}

#endif
