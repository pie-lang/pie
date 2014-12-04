%option noyywrap

%{ /* -*- mode: c++ -*- */
#include "compiler/scanner.h"
#include "compiler/parser.h"
#include "compiler/pie.tab.hpp"

#define YYSTYPE pie::compiler::ScannerToken
#define YYLTYPE int
#define YY_EXTRA_TYPE pie::compiler::Scanner*
#define _scanner yyextra
%}


NUMBER [0-9]+
DOUBLE ([0-9]*[\.][0-9]+)|([0-9]+[\.][0-9]*)
LABEL  [a-zA-Z_\x7f-\xff][a-zA-Z0-9_\x7f-\xff]*
WHITESPACE [ \n\r\t]+

%%

"+"		{ return T_PLUS; }
"-"		{ return T_MINUS; }
"*"		{ return T_MUL; }
"/"		{ return T_DIV; }


%%

namespace pie { namespace compiler {

void Scanner::scan()
{
    yylex_init_extra(this, &m_yyscanner);
	yylex(m_yyscanner);
}

}}
