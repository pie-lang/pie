#ifndef __PIE_AST_MODULE__
#define __PIE_AST_MODULE__

#include <string>
#include <vector>
#include <map>

#include "compiler/ast.h"

namespace pie { namespace compiler {

class ModuleNode : Node 
{
public:
	std::string name;
	std::map<std::string, Node *> symtab;

	DEFINE_NODE(ModuleNode);

	// statements are in children
};

}}

#endif
