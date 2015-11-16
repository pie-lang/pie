#ifndef __PIE_AST_OP__
#define __PIE_AST_OP__

#include "compiler/ast.h"

namespace pie { namespace compiler {

class BinaryOpNode : Node 
{
public:
	Node *lhs;
	Node *rhs;

	int op;

	BinaryNode(int op, Node *lhs, Node *rhs) : op(op), lhs(lhs), rhs(rhs)
	{
		push(lhs);
		push(rhs);	
	}

	DEFINE_NODE(BinaryOpNode);
};

}}

#endif
