#ifndef __PIE_AST_ASSIGN__
#define __PIE_AST_ASSIGN__

#include "compiler/ast.h"

namespace pie { namespace compiler {

class AssignNode : Node 
{
public:
	Node *var;
	Node *value;

	AssignNode(Node *var, Node *value) : var(var), value(value) {}

	void visit(Visitor *visitor)
	{
		visitor->visit(var);
		visitor->visit(value);

		visitor->visit(this);
	}

};


}}
#endif
