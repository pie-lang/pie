%option noyywrap

%{ /* -*- mode: c++ -*- */
#include "scanner.h"
#include "parser.h"
#include "pie.tab.hpp"

// macros for flex
#define YYSTYPE pie::compiler::ScannerToken
#define YYLTYPE pie::compiler::Location
#define YY_EXTRA_TYPE pie::compiler::Scanner*
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
