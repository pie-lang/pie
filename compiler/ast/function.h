#ifndef __PIE_AST_FUNCTION__
#define __PIE_AST_FUNCTION__

#include <string>
#include <vector>

#include "compiler/ast/node.h"

namespace pie { namespace compiler {

class TypeNode;

class FunctionNode : Node 
{
public:
	std::string name;

	int access_level;

	std::map<std::string, TypeNode *> params;

	TypeNode *return_type;

	// statements are in children

	DEFINE_NODE(FunctionNode);
};

class ClosureNode : Node {
public:
	std::map<std::string, TypeNode *> params;

	TypeNode *return_type;

	DEFINE_NODE(ClosureNode);
};


class FunctionCallNode : Node 
{
public:
	std::string name;

	// arguments are in children
	DEFINE_NODE(FunctionCallNode);
};

}}

#endif
