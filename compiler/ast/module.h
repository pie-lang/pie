#ifndef __PIE_AST_MODULE__
#define __PIE_AST_MODULE__

#include "compiler/ast/node.h"

#include <string>
#include <map>

namespace pie { namespace compiler {

class ModuleNode : public Node
{
public:
	std::string name;
	std::map<std::string, Node *> symtab;
	std::vector<ImportNode *> imports;
	std::vector<FunctionNode *> functions;

	DEFINE_VISIT(ModuleNode);

	// statements are in children
};

}}

#endif
