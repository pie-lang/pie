%{
#define YYERROR_VERBOSE
#define YYSTYPE pie::compiler::Token
#define YYLEX_PARAM _p
#define YYLTYPE_IS_TRIVIAL true
#define YYLTYPE pie::compiler::Location

#include "compiler/parser.h"

#ifdef yyerror
#undef yyerror
#endif
#define yyerror(p,msg) p->parseFatal(msg)

using namespace pie::compiler;

static int yylex(YYSTYPE *token, Parser *_p) {
  //return _p->scan(token);
}
%}

%define api.pure
%lex-param {pie::compiler::Parser *_p}
%parse-param {pie::compiler::Parser *_p}


%left T_PLUS T_MINUS T_MUL T_DIV


%token T_PLUS
%token T_MINUS
%token T_MUL
%token T_DIV

%token T_NUMBER

%%


top_statement:
	/* Empty */
	| expr
;

expr:
	T_NUMBER T_PLUS T_NUMBER { printf("WTF\n"); }
	T_NUMBER T_MINUS T_NUMBER { /* Empty for now */ }
	T_NUMBER T_MUL T_NUMBER { /* Empty for now */ }
	T_NUMBER T_DIV T_NUMBER { /* Empty for now */ }
;

%%

namespace pie { namespace compiler {

void Parser::parse()
{
  yyparse(this);
}

}}
