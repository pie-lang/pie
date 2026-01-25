#ifndef __PIE_AST_TYPE__
#define __PIE_AST_TYPE__

#include <string>

#include "compiler/ast/node.h"

namespace pie { namespace compiler {

class TypeNode : public Node
{
public:
	std::string name;
	bool is_array;

	TypeNode() : is_array(false) {}
	TypeNode(const std::string &name, bool is_array = false)
		: name(name), is_array(is_array) {}

	DEFINE_VISIT(TypeNode);
};

}}

#endif
