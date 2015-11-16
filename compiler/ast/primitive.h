#ifndef __PIE_AST_PRIMITIVE__
#define __PIE_AST_PRIMITIVE__

#include "compiler/ast.h"

namespace pie { namespace compiler {

/* Do I really need such a simple node? */
// Only Int Node for now.... for simplicity
class IntNode : Node 
{
public:
	int value;

	IntNode(int value) : value(value) {}

	DEFINE_NODE(IntNode);
};

}}

#endif
