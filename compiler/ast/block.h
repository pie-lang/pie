#ifndef __PIE_AST_BLOCK__
#define __PIE_AST_BLOCK__

#include <vector>

#include "compiler/ast/node.h"

namespace pie { namespace compiler {

class BlockNode : public Node
{
public:
	// statements are in children

	BlockNode() {}

	void addStatement(Node *stmt)
	{
		if (stmt) push(stmt);
	}

	DEFINE_VISIT(BlockNode);
};

}}

#endif
