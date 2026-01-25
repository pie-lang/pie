#ifndef __PIE_AST_IMPORT__
#define __PIE_AST_IMPORT__

#include <string>

#include "compiler/ast/node.h"

namespace pie { namespace compiler {

class ImportNode : public Node
{
public:
	std::string module_name;
	int access_level;
	bool import_all;  // true if "import foo.*"

	ImportNode() : access_level(0), import_all(false) {}
	ImportNode(const std::string &name, int access, bool all = false)
		: module_name(name), access_level(access), import_all(all) {}

	DEFINE_VISIT(ImportNode);
};

}}

#endif
