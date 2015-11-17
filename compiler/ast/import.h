#ifndef __PIE_AST_IMPORT__
#define __PIE_AST_IMPORT__

#include <string>

#include "compiler/ast/node.h"

namespace pie { namespace compiler {

class ImportNode : Node 
{
public:
	std::string module_name;

	int access_level;

	DEFINE_NODE(ImportNode);
};

}}

#endif
