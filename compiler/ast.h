#ifndef __PIE_AST__
#define __PIE_AST__

namespace pie { namespace compiler {


class Node {
	Node();
};

class Visitor {
public:
	virtual void visit(Node *node)
	{
	
	}
};

}}

#endif
