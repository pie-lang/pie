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
%left '.'

%left '='

%token ';'

%token '[' ']'
%token '(' ')'
%token '{' '}'

%nonassoc '>' '<'

%token T_COMMENT

/* Keywords */
%token T_MODULE
%token T_IMPORT
%token T_AS
%token T_ACC_PUBLIC

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

%right T_INC T_DEC

%left T_PLUS_EUQAL T_MINUS_EUQAL

%token T_WHITESPACE


%token T_STRING
%token T_NUMBER

%token T_INDENTIFIER

%token T_ERROR

%token END 0 "end of file"

%%

start:
	/* Empty */
	| top_decl_stmts statements
;

visibility:
	/* Empty */ { /* private */}
	| T_ACC_PUBLIC {}
;

top_decl_stmts:
	  module_decl_stmt import_stmts
;

module_decl_stmt:
	  T_MODULE symbol_name {}
;

import_stmts:
	/* Empty */
	| import_stmts import_stmt
;

import_stmt:
	T_IMPORT import_pattern { }
;

import_pattern:
	  symbol_name
	| symbol_name '.' '*' /* import all */
;


statements:
	/* Empty */
	  statement
	| statements statement
;

statement:
	  func_decl_stmt
	| T_LET var_name '=' expr
	| T_RETURN expr
	| if_stmt
	| expr
;

func_call:
	symbol_name '(' arguments ')'
;

arguments:
	/* Empty */
	| argument
	| arguments ',' argument
;

argument:
	expr
;

if_stmt:
	/* TODO */
	T_IF '(' expr ')' '{' statements '}'
;

func_decl_stmt:
	visibility T_FUNC T_INDENTIFIER '(' parameter_list ')' return_type func_body
;

type_name:
	/* Empty */
	| T_INDENTIFIER
	| T_INDENTIFIER '[' ']'
;

return_type:
	/* Empty */
	| ':' type_name
;

func_body:
	'{' statements '}'
;

parameter_list:
	/* Empty for now */
	| parameter
	| parameter_list ',' parameter
; 

parameter:
	  T_INDENTIFIER
	| T_INDENTIFIER ':' type_name
;

var_name:
	T_INDENTIFIER
;

symbol_name:
	  T_INDENTIFIER
	| symbol_name '.' T_INDENTIFIER
;

expr:
	  T_NUMBER
	| var_name
	| var_name '=' expr
	| expr '+' expr { printf("WTF\n"); }
	| expr '-' expr { /* Empty for now */ }
	| expr '*' expr { /* Empty for now */ }
	| expr '/' expr { /* Empty for now */ }
	| var_name T_PLUS_EUQAL expr
	| var_name T_MINUS_EUQAL expr
	| func_call
	| expr '<' expr
	| expr '>' expr
;

%%

namespace pie { namespace compiler {

int Parser::parse()
{
  return yyparse(this);
}

}}
