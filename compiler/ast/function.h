#ifndef __PIE_AST_FUNCTION__
#define __PIE_AST_FUNCTION__

#include <string>
#include <vector>

#include "compiler/ast/node.h"

namespace pie { namespace compiler {

class TypeNode;

class FunctionNode : public Node
{
public:
	std::string name;
	int access_level;
	std::vector<std::pair<std::string, TypeNode *>> params;
	TypeNode *return_type;

	FunctionNode() : access_level(0), return_type(nullptr) {}
	FunctionNode(const std::string &name, int access)
		: name(name), access_level(access), return_type(nullptr) {}

	// statements are in children
	DEFINE_VISIT(FunctionNode);
};

class ClosureNode : public Node
{
public:
	std::vector<std::pair<std::string, TypeNode *>> params;
	TypeNode *return_type;

	ClosureNode() : return_type(nullptr) {}

	DEFINE_VISIT(ClosureNode);
};

class FunctionCallNode : public Node
{
public:
	std::string name;

	FunctionCallNode() {}
	FunctionCallNode(const std::string &name) : name(name) {}

	// arguments are in children
	DEFINE_VISIT(FunctionCallNode);
};

}}

#endif
