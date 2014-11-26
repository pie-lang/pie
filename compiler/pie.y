%{
#define YYSTYPE pie::parser::Token
#define YYLEX_PARAM _p
#define YYLTYPE pie::parser::Location

#include "parser.h"


#ifdef yyerror
#undef yyerror
#endif
#define yyerror(loc,p,msg) p->parseFatal(loc,msg)

using namespace pie::parser;

static int yylex(YYSTYPE *token, Location *loc, Parser *_p) {
  return _p->scan(token, loc);
}
%}

%left T_PLUS T_MINUS T_MUL T_DIV


%token T_PLUS
%token T_MINUS
%token T_MUL
%token T_DIV

%token T_NUMBER

%%


top_statement:
	expr
;

expr:
	T_NUMBER T_PLUS T_NUMBER
;
