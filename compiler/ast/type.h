#ifndef __PIE_AST_TYPE__
#define __PIE_AST_TYPE__

#include <string>

#include "compiler/ast/node.h"

namespace pie { namespace compiler {

class TypeNode : Node 
{
public:
	std::string name;  // function type ? or object ? TODO

	bool is_array; // Maybe there is a better way

	DEFINE_NODE(TypeNode);
};

}}

#endif
