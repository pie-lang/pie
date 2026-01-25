#ifndef __PIE_AST_RETURN__
#define __PIE_AST_RETURN__

#include "compiler/ast/node.h"

namespace pie { namespace compiler {

class ReturnNode : public Node
{
public:
	Node *expr;

	ReturnNode(Node *node) : expr(node)
	{
		if (node) push(node);
	}

	DEFINE_VISIT(ReturnNode);
};

}}

#endif
