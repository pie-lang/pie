#ifndef __PIE_AST_FUNCTION__
#define __PIE_AST_FUNCTION__

#include <string>
#include <vector>

#include "compiler/ast.h"
#include "compler/ast/type.h"

namespace pie { namespace compiler {

class FunctionNode : Node 
{
public:
	std::string name;

	int access_level;

	std::map<std::string, TypeNode *> params;

	TypeNode *return_type;

	// statements are in children
};

class ClosureNode : Node {
public:
	std::map<std::string, TypeNode *> params;

	TypeNode *return_type;
};


class FunctionCallNode : Node 
{
public:
	std::string name;

	// arguments are in children
};

}}

#endif
