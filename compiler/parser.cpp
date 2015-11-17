#include <stdio.h>

#include "compiler/scanner.h"
#include "compiler/parser.h"

#include "compiler/ast/module.h"

namespace pie { namespace compiler {

Parser::Parser(Scanner &s) : scanner(s)
{
	module = new ModuleNode();
}

void Parser::parseFatal(std::string msg)
{
  printf("%s\n", msg.c_str());
}

int Parser::scan(Token *token)
{
  return scanner.scan();
}

void Parser::onModule(std::string name)
{
	module->name = name;
}

void Parser::onImport(std::string name, int visibility)
{

}

}}
