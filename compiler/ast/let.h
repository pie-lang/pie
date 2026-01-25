#ifndef __PIE_AST_LET__
#define __PIE_AST_LET__

#include <string>

#include "compiler/ast/node.h"

namespace pie { namespace compiler {

class TypeNode;

class LetNode : public Node
{
public:
	std::string name;
	TypeNode *type;
	Node *value;

	LetNode(const std::string &name, TypeNode *type, Node *value)
		: name(name), type(type), value(value)
	{
		if (value) push(value);
	}

	DEFINE_VISIT(LetNode);
};

}}

#endif
