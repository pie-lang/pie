#include "compiler/scanner.h"
#include "compiler/parser.h"

namespace pie { namespace compiler {

void Parser::parseFatal(std::string msg)
{
  printf("%s\n", msg.c_str());
}

int Parser::scan(Token *token)
{
  return scanner.scan();
}

}}
