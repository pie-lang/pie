%code requires {
#include <string>
#include <vector>
#include <utility>
#include "compiler/ast.h"

namespace pie { namespace compiler {
    class Parser;
}}
}

%code {
#include "compiler/parser.h"
#include "compiler/scanner.h"

#include <cstdlib>

#ifdef yyerror
#undef yyerror
#endif
#define yyerror(p, msg) p->parseFatal(msg)

using namespace pie::compiler;

static int yylex(YYSTYPE *token, pie::compiler::Parser *_p);
}

%define api.pure
%lex-param {pie::compiler::Parser *_p}
%parse-param {pie::compiler::Parser *_p}

%union {
    pie::compiler::Node *node;
    pie::compiler::BlockNode *block;
    pie::compiler::TypeNode *type;
    std::string *str;
    int64_t num;
    double dbl;
    int visibility;
    std::vector<pie::compiler::Node*> *node_list;
    std::vector<std::pair<std::string, pie::compiler::TypeNode*>> *param_list;
}

%destructor { delete $$; } <str>
%destructor { delete $$; } <node_list>
%destructor { delete $$; } <param_list>

/* Precedence from lowest to highest */
%right '='
%left T_OR
%left T_AND
%nonassoc '<' '>' T_LE T_GE T_EQ T_NE
%left '+' '-'
%left '*' '/'
%left '.'

%token ';'

%token '[' ']'
%token '(' ')'
%token '{' '}'

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

%right T_INC T_DEC

%left T_PLUS_EQUAL T_MINUS_EQUAL

%token T_WHITESPACE

%token <str> T_STRING
%token <num> T_NUMBER
%token <dbl> T_DOUBLE
%token <str> T_IDENTIFIER

%token T_ERROR

%token END 0 "end of file"

%type <node> start
%type <node> module_decl_stmt
%type <node> import_stmt
%type <node> statement
%type <node> func_decl_stmt
%type <node> expr
%type <node> if_stmt
%type <block> func_body
%type <block> block
%type <type> type_name
%type <type> return_type
%type <str> symbol_name
%type <visibility> visibility
%type <node_list> arguments
%type <node_list> argument_list
%type <param_list> parameter_list
%type <param_list> parameter_list_inner

%%

start:
    module_decl_stmt import_stmts statements { $$ = _p->module; }
;

import_stmts:
    import_stmts import_stmt
    | /* Empty */
;

visibility:
    /* Empty */ { $$ = 0; /* private */ }
    | T_ACC_PUBLIC { $$ = 1; /* public */ }
;

module_decl_stmt:
    T_MODULE symbol_name {
        _p->module->name = *$2;
        delete $2;
        $$ = _p->module;
    }
;

import_stmt:
    visibility T_IMPORT symbol_name {
        ImportNode *imp = new ImportNode(*$3, $1, false);
        _p->module->imports.push_back(imp);
        _p->module->push(imp);
        delete $3;
        $$ = imp;
    }
    | visibility T_IMPORT symbol_name '.' '*' {
        ImportNode *imp = new ImportNode(*$3, $1, true);
        _p->module->imports.push_back(imp);
        _p->module->push(imp);
        delete $3;
        $$ = imp;
    }
;

statements:
    /* Empty */
    | statements statement {
        if ($2 && _p->function) {
            _p->function->push($2);
        }
    }
;

statement:
    func_decl_stmt { $$ = $1; }
    | T_LET T_IDENTIFIER '=' expr {
        $$ = _p->makeLet(*$2, nullptr, $4);
        delete $2;
    }
    | T_LET T_IDENTIFIER ':' type_name '=' expr {
        $$ = _p->makeLet(*$2, $4, $6);
        delete $2;
    }
    | T_RETURN expr {
        $$ = _p->makeReturn($2);
    }
    | T_RETURN {
        $$ = _p->makeReturn(nullptr);
    }
    | if_stmt { $$ = $1; }
    | expr { $$ = $1; }
;


arguments:
    /* Empty */ { $$ = new std::vector<Node*>(); }
    | argument_list { $$ = $1; }
;

argument_list:
    expr {
        $$ = new std::vector<Node*>();
        $$->push_back($1);
    }
    | argument_list ',' expr {
        $$ = $1;
        $$->push_back($3);
    }
;

if_stmt:
    T_IF '(' expr ')' block {
        $$ = _p->makeIf($3, $5, nullptr);
    }
    | T_IF '(' expr ')' block T_ELSE block {
        $$ = _p->makeIf($3, $5, $7);
    }
    | T_IF '(' expr ')' block T_ELSE if_stmt {
        $$ = _p->makeIf($3, $5, $7);
    }
;

block:
    '{' {
        _p->blocks.push(_p->makeBlock());
    }
    block_statements '}' {
        $$ = _p->blocks.top();
        _p->blocks.pop();
    }
;

block_statements:
    /* Empty */
    | block_statements block_statement
;

block_statement:
    statement {
        if ($1 && !_p->blocks.empty()) {
            _p->blocks.top()->addStatement($1);
        }
    }
;

func_decl_stmt:
    visibility T_FUNC T_IDENTIFIER '(' parameter_list ')' return_type {
        FunctionNode *fn = new FunctionNode(*$3, $1);
        if ($5) {
            fn->params = *$5;
            delete $5;
        }
        fn->return_type = $7;
        _p->function = fn;
        _p->module->functions.push_back(fn);
        _p->module->symtab[*$3] = fn;
        delete $3;
    }
    func_body {
        // Function body statements are already added to _p->function
        $$ = _p->function;
        _p->function = nullptr;
    }
;

type_name:
    /* Empty */ { $$ = nullptr; }
    | T_IDENTIFIER {
        $$ = _p->makeType(*$1, false);
        delete $1;
    }
    | T_IDENTIFIER '[' ']' {
        $$ = _p->makeType(*$1, true);
        delete $1;
    }
;

return_type:
    /* Empty */ { $$ = nullptr; }
    | ':' type_name { $$ = $2; }
;

func_body:
    '{' func_body_statements '}' {
        $$ = nullptr;
    }
;

func_body_statements:
    /* Empty */
    | func_body_statements func_body_statement
;

func_body_statement:
    statement {
        if ($1 && _p->function) {
            _p->function->push($1);
        }
    }
;

parameter_list:
    /* Empty */ { $$ = new std::vector<std::pair<std::string, TypeNode*>>(); }
    | parameter_list_inner { $$ = $1; }
;

parameter_list_inner:
    T_IDENTIFIER {
        $$ = new std::vector<std::pair<std::string, TypeNode*>>();
        $$->push_back(std::make_pair(*$1, (TypeNode*)nullptr));
        delete $1;
    }
    | T_IDENTIFIER ':' type_name {
        $$ = new std::vector<std::pair<std::string, TypeNode*>>();
        $$->push_back(std::make_pair(*$1, $3));
        delete $1;
    }
    | parameter_list_inner ',' T_IDENTIFIER {
        $$ = $1;
        $$->push_back(std::make_pair(*$3, (TypeNode*)nullptr));
        delete $3;
    }
    | parameter_list_inner ',' T_IDENTIFIER ':' type_name {
        $$ = $1;
        $$->push_back(std::make_pair(*$3, $5));
        delete $3;
    }
;

symbol_name:
    T_IDENTIFIER { $$ = $1; }
    | symbol_name '.' T_IDENTIFIER {
        *$1 += ".";
        *$1 += *$3;
        delete $3;
        $$ = $1;
    }
;

expr:
    T_NUMBER {
        $$ = _p->makeInt($1);
    }
    | T_DOUBLE {
        $$ = _p->makeDouble($1);
    }
    | T_STRING {
        $$ = _p->makeString(*$1);
        delete $1;
    }
    | T_IDENTIFIER {
        $$ = _p->makeIdentifier(*$1);
        delete $1;
    }
    | T_IDENTIFIER '=' expr {
        Node *var = _p->makeIdentifier(*$1);
        delete $1;
        $$ = _p->makeAssign(var, $3);
    }
    | T_IDENTIFIER '(' arguments ')' {
        std::vector<Node*> args;
        if ($3) {
            args = *$3;
            delete $3;
        }
        $$ = _p->makeFunctionCall(*$1, args);
        delete $1;
    }
    | symbol_name '.' T_IDENTIFIER '(' arguments ')' {
        std::string fullName = *$1 + "." + *$3;
        std::vector<Node*> args;
        if ($5) {
            args = *$5;
            delete $5;
        }
        $$ = _p->makeFunctionCall(fullName, args);
        delete $1;
        delete $3;
    }
    | expr '+' expr {
        $$ = _p->makeBinaryOp(BinaryOp::Add, $1, $3);
    }
    | expr '-' expr {
        $$ = _p->makeBinaryOp(BinaryOp::Sub, $1, $3);
    }
    | expr '*' expr {
        $$ = _p->makeBinaryOp(BinaryOp::Mul, $1, $3);
    }
    | expr '/' expr {
        $$ = _p->makeBinaryOp(BinaryOp::Div, $1, $3);
    }
    | T_IDENTIFIER T_PLUS_EQUAL expr {
        Node *var = _p->makeIdentifier(*$1);
        delete $1;
        $$ = _p->makeBinaryOp(BinaryOp::AddAssign, var, $3);
    }
    | T_IDENTIFIER T_MINUS_EQUAL expr {
        Node *var = _p->makeIdentifier(*$1);
        delete $1;
        $$ = _p->makeBinaryOp(BinaryOp::SubAssign, var, $3);
    }
    | expr '<' expr {
        $$ = _p->makeBinaryOp(BinaryOp::Lt, $1, $3);
    }
    | expr '>' expr {
        $$ = _p->makeBinaryOp(BinaryOp::Gt, $1, $3);
    }
    | expr T_LE expr {
        $$ = _p->makeBinaryOp(BinaryOp::Le, $1, $3);
    }
    | expr T_GE expr {
        $$ = _p->makeBinaryOp(BinaryOp::Ge, $1, $3);
    }
    | expr T_EQ expr {
        $$ = _p->makeBinaryOp(BinaryOp::Eq, $1, $3);
    }
    | expr T_NE expr {
        $$ = _p->makeBinaryOp(BinaryOp::Ne, $1, $3);
    }
    | expr T_AND expr {
        $$ = _p->makeBinaryOp(BinaryOp::And, $1, $3);
    }
    | expr T_OR expr {
        $$ = _p->makeBinaryOp(BinaryOp::Or, $1, $3);
    }
    | '(' expr ')' {
        $$ = $2;
    }
    | '-' expr %prec T_INC {
        $$ = _p->makeUnaryOp(UnaryOp::Neg, $2);
    }
    | '!' expr %prec T_INC {
        $$ = _p->makeUnaryOp(UnaryOp::Not, $2);
    }
;

%%

static int yylex(YYSTYPE *token, pie::compiler::Parser *_p)
{
    int tok = _p->scan(token);
    return tok;
}

namespace pie { namespace compiler {

int Parser::parse()
{
    return yyparse(this);
}

}}
