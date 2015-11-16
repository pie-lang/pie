#ifndef __PIE_AST_RETURN__
#define __PIE_AST_RETURN__

#include <string>

#include "compiler/ast.h"

namespace pie { namespace compiler {

class ReturnNode : Node 
{
public:
	Node *expr;

	ReturnNode(Node *node) : expr(node)
	{
		push(node);
	}
};

}}

#endif
