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
#define yyerror(p, msg) p->parseFatal(msg)

using namespace pie::compiler;

static int yylex(YYSTYPE *token, Parser *_p) {
  return _p->scan(token);
}
%}

%define api.pure
%lex-param {pie::compiler::Parser *_p}
%parse-param {pie::compiler::Parser *_p}


%left '+' '-' '*' '/'

%token T_COMMENT

/* Keywords */
%token T_MODULE
%token T_IMPORT
%token T_AS

%token T_FUNC
%token T_RETURN
%token T_LET


/* statement related */
%token T_IF
%token T_ELSE
%token T_ELSEIF

%token T_WHILE
%token T_FOR

%token T_LT
%token T_GT
%token T_LT_EQ
%token T_GT_EQ
%token T_EQ

%token T_WHITESPACE


%token T_STRING
%token T_NUMBER

%token T_INDENTIFIER

%token T_ERROR

%token END 0 "end of file"

%%

top_statement:
	/* Empty */
	| statements
	| expr
;

statements:
	  func_decl_stmt
	| module_decl_stmt
	| import_stmt
;

module_decl_stmt:
	T_MODULE T_INDENTIFIER {}
;

import_stmt:
	T_IMPORT T_INDENTIFIER {}
;

func_decl_stmt:
	T_FUNC T_INDENTIFIER '(' parameter_list ')' func_body
;

func_body:
	'{' '}'
;

parameter_list:
		/* Empty for now */
; 

expr:
	  T_NUMBER '+' T_NUMBER { printf("WTF\n"); }
	| T_NUMBER '-' T_NUMBER { /* Empty for now */ }
	| T_NUMBER '*' T_NUMBER { /* Empty for now */ }
	| T_NUMBER '/' T_NUMBER { /* Empty for now */ }
;

%%

namespace pie { namespace compiler {

int Parser::parse()
{
  yyparse(this);

  return 0;
}

}}
