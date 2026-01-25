#ifndef __PIE_AST_IF__
#define __PIE_AST_IF__

#include "compiler/ast/node.h"

namespace pie { namespace compiler {

class BlockNode;

class IfNode : public Node
{
public:
	Node *condition;
	BlockNode *then_block;
	Node *else_block;  // Can be BlockNode or another IfNode (for else-if)

	IfNode(Node *cond, BlockNode *then_blk, Node *else_blk = nullptr)
		: condition(cond), then_block(then_blk), else_block(else_blk)
	{
		if (cond) push(cond);
		if (then_blk) push(reinterpret_cast<Node*>(then_blk));
		if (else_blk) push(else_blk);
	}

	DEFINE_VISIT(IfNode);
};

}}

#endif
