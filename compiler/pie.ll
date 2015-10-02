%option noyywrap
%option stack

%x ST_BLOCK_COMMENT
%x ST_LINE_COMMENT

%{ /* -*- mode: c++ -*- */
#define YYSTYPE pie::compiler::ScannerToken
#define YYLTYPE int
#define YY_EXTRA_TYPE pie::compiler::Scanner*
#define _scanner yyextra

#define DEBUG_LEX 1

#if DEBUG_LEX
# define DBG_TOKEN(t) do { 								\
	if (!getenv("NDEBUG_FLEX")) {						\
		printf("T: %s \"%s\"\n", #t, yytext);	\
	} 													\
} while(0)
#else
# define DBG_TOKEN(t)
#endif

#define RETURN_TOKEN(t) do { 	\
	DBG_TOKEN(t); 				\
	return t; 					\
} while(0)

#include "compiler/scanner.h"
#include "compiler/parser.h"
#include "compiler/pie.tab.hpp"

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
"let"			{ RETURN_TOKEN(T_LET); }

"+="			{ RETURN_TOKEN(T_PLUS_EUQAL); }
"-="			{ RETURN_TOKEN(T_MINUS_EUQAL); }

"++"			{ RETURN_TOKEN(T_INC); }
"--"			{ RETURN_TOKEN(T_DEC); }

"{#" {
	BEGIN(ST_BLOCK_COMMENT);
	yymore();
}

<ST_BLOCK_COMMENT>"#}" {
	BEGIN(INITIAL);
	RETURN_TOKEN(T_COMMENT);
}

"#"				{
	BEGIN(ST_LINE_COMMENT);
	yymore();
}

<ST_LINE_COMMENT>[^\n\r]* {
	BEGIN(INITIAL);
	RETURN_TOKEN(T_COMMENT);
}


[a-zA-Z_][a-zA-Z0-9_]*	{ RETURN_TOKEN(T_INDENTIFIER); }

{ANY_CHAR}		{ DBG_TOKEN(T_RAW_TEXT); return *yytext; }

%%

namespace pie { namespace compiler {

Scanner::Scanner(FILE *file)
{
	m_line = 0;
	m_filename = "Unknown";
	m_file = file ? file : stdin;

    yylex_init_extra(this, &m_yyscanner);

	if (file) {
		yy_switch_to_buffer(yy_create_buffer(file, YY_BUF_SIZE, m_yyscanner), m_yyscanner);
	}
}

int Scanner::scan()
{
	int tok;

get_next:
	tok = yylex(m_yyscanner);

	/* Ignore T_COMMENT maybe we need it in reflection? */
	if (tok == T_COMMENT) {
		goto get_next;
	}

	return tok;
}

Scanner::~Scanner()
{
	yylex_destroy(m_yyscanner);
}

}}
