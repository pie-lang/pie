#ifndef __PIE_AST_ASSIGN__
#define __PIE_AST_ASSIGN__

#include "compiler/ast/node.h"

namespace pie { namespace compiler {

class AssignNode : public Node
{
public:
	Node *var;
	Node *value;

	AssignNode(Node *var, Node *value) : var(var), value(value)
	{
		push(var);
		push(value);
	}

	DEFINE_VISIT(AssignNode);
};

}}

#endif
