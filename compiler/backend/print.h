#ifndef __PIE_BACKEND_PRINT__
#define __PIE_BACKEND_PRINT__

#include <string>
#include <vector>
#include <map>

#include "compiler/ast.h"

namespace pie { namespace compiler {

class PrintVistor : Visitor
{
public:
	std::string out; // output str TODO use stream instead

	#define AST_NODE DECLARE_VISITOR
	AST_NODES
	#undef AST_NODE
};

}}

#endif
