%option noyywrap

%{ /* -*- mode: c++ -*- */
#include "pie.tab.hpp"
// macros for flex
#define YYSTYPE HPHP::ScannerToken
#define YYLTYPE pie::paser::Location
#define YY_EXTRA_TYPE pie::Scanner*
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
