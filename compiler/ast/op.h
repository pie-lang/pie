#ifndef __PIE_AST_OP__
#define __PIE_AST_OP__

#include "compiler/ast/node.h"

namespace pie { namespace compiler {

class BinaryOpNode : Node
{
public:
	Node *lhs;
	Node *rhs;

	int op;

	BinaryOpNode(int op, Node *lhs, Node *rhs) : op(op), lhs(lhs), rhs(rhs)
	{
		push(lhs);
		push(rhs);	
	}

	DEFINE_NODE(BinaryOpNode);
};

class UnaryOpNode : Node
{
public:
	Node *expr;

	int op;

	UnaryOpNode(int op, Node *expr) : op(op), expr(expr)
	{
		push(expr);
	}

	DEFINE_NODE(UnaryOpNode);
};

}}

#endif
