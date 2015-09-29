%option noyywrap
%option stack

%x ST_COMMENT
%x ST_COMMENT_LINE

%{ /* -*- mode: c++ -*- */
#include "compiler/scanner.h"
#include "compiler/parser.h"
#include "compiler/pie.tab.hpp"

#define YYSTYPE pie::compiler::ScannerToken
#define YYLTYPE int
#define YY_EXTRA_TYPE pie::compiler::Scanner*
#define _scanner yyextra

#define DEBUG_LEX 1

#if DEBUG_LEX
#define DBG_TOKEN(t) printf("Get token: %s \"%s\"\n", #t, yytext)
#else
#define DBG_TOKEN(t)
#endif

#define RETURN_TOKEN(t) do { DBG_TOKEN(t); return t; } while(0)

%}

NUMBER			[0-9]+
DOUBLE 			([0-9]*[\.][0-9]+)|([0-9]+[\.][0-9]*)
LABEL  			[a-zA-Z_\x7f-\xff][a-zA-Z0-9_\x7f-\xff]*
WHITESPACE		[ \n\r\t]+
TABS_AND_SPACES [ \t]*

ANY_CHAR 		(.|[\n])
NEWLINE 		("\r"|"\n"|"\r\n")

%%

{NUMBER}		{ RETURN_TOKEN(T_NUMBER); }

{NEWLINE}		{ _scanner->m_line++; }
{WHITESPACE}	{ /* Skip white spaces */  }

"fn"  			{ RETURN_TOKEN(T_FUNC); }
"if"			{ RETURN_TOKEN(T_IF); }
"else"			{ RETURN_TOKEN(T_ELSE); }
"return"		{ RETURN_TOKEN(T_RETURN); }

"module"		{ RETURN_TOKEN(T_MODULE); }
"import"		{ RETURN_TOKEN(T_IMPORT); }

"#"				{
	BEGIN(ST_COMMENT_LINE);
	yymore();
}

<ST_COMMENT_LINE>[^\n\r]*{NEWLINE} {
	BEGIN(INITIAL);
	RETURN_TOKEN(T_COMMENT);
}

"{#"{WHITESPACE} {
	BEGIN(ST_COMMENT);
	yymore();
}

<ST_COMMENT>"#}" {
	BEGIN(INITIAL);
	RETURN_TOKEN(T_COMMENT);
}


[a-zA-Z_][a-zA-Z0-9_]*	{ RETURN_TOKEN(T_INDENTIFIER); }

{ANY_CHAR}			{ return *yytext; }

%%

namespace pie { namespace compiler {

Scanner::Scanner()
{
	m_line = 0;
	m_filename = "Unknown";
    yylex_init_extra(this, &m_yyscanner);
}

int Scanner::scan()
{
	return yylex(m_yyscanner);
}

Scanner::~Scanner()
{
	yylex_destroy(m_yyscanner);
}

}}
