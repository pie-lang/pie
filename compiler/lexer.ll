%option noyywrap
%option stack
%option yylineno
%option reentrant

%x ST_BLOCK_COMMENT
%x ST_LINE_COMMENT
%x ST_STRING

%{ /* -*- mode: c++ -*- */
#define YYLTYPE int
#define YY_EXTRA_TYPE pie::compiler::Scanner*
#define _scanner yyextra

#define DEBUG_LEX 1

#if DEBUG_LEX
# define DBG_TOKEN(t) do { 								\
	if (getenv("DEBUG_LEX")) {						\
		printf("T:%d\t%s \"%s\"\n", yylineno, #t, yytext);	\
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
#include "compiler/parser.tab.hpp"

static std::string string_buffer;

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
{DOUBLE}		{ RETURN_TOKEN(T_DOUBLE); }

{NEWLINE}		{ _scanner->m_line++; }
{WHITESPACE}	{ /* Skip white spaces */  }

"fn"  			{ RETURN_TOKEN(T_FUNC); }
"if"			{ RETURN_TOKEN(T_IF); }
"else"			{ RETURN_TOKEN(T_ELSE); }
"return"		{ RETURN_TOKEN(T_RETURN); }

"module"		{ RETURN_TOKEN(T_MODULE); }
"import"		{ RETURN_TOKEN(T_IMPORT); }
"let"			{ RETURN_TOKEN(T_LET); }

"public"		{ RETURN_TOKEN(T_ACC_PUBLIC); }

"+="			{ RETURN_TOKEN(T_PLUS_EQUAL); }
"-="			{ RETURN_TOKEN(T_MINUS_EQUAL); }

"++"			{ RETURN_TOKEN(T_INC); }
"--"			{ RETURN_TOKEN(T_DEC); }

"<="			{ RETURN_TOKEN(T_LE); }
">="			{ RETURN_TOKEN(T_GE); }
"=="			{ RETURN_TOKEN(T_EQ); }
"!="			{ RETURN_TOKEN(T_NE); }

"&&"			{ RETURN_TOKEN(T_AND); }
"||"			{ RETURN_TOKEN(T_OR); }

"{#" {
	BEGIN(ST_BLOCK_COMMENT);
	yymore();
}

<ST_BLOCK_COMMENT>[^#]+ { /* FIXME it should allow contain inline comment inside of block */
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

\" {
	BEGIN(ST_STRING);
	string_buffer.clear();
}

<ST_STRING>\" {
	BEGIN(INITIAL);
	RETURN_TOKEN(T_STRING);
}

<ST_STRING>\\n  { string_buffer += '\n'; }
<ST_STRING>\\t  { string_buffer += '\t'; }
<ST_STRING>\\r  { string_buffer += '\r'; }
<ST_STRING>\\\\ { string_buffer += '\\'; }
<ST_STRING>\\\" { string_buffer += '"'; }
<ST_STRING>[^\\\"]+ { string_buffer += yytext; }


[a-zA-Z_][a-zA-Z0-9_]*	{ RETURN_TOKEN(T_IDENTIFIER); }

{ANY_CHAR}		{ DBG_TOKEN(T_RAW_CHAR); return *yytext; }

%%

namespace pie { namespace compiler {

Scanner::Scanner(FILE *file)
{
	m_line = 0;
	m_filename = "Unknown";
	m_file = file ? file : stdin;

    yylex_init_extra(this, (yyscan_t*)&m_yyscanner);

	if (file) {
		yy_switch_to_buffer(yy_create_buffer(file, YY_BUF_SIZE, (yyscan_t)m_yyscanner), (yyscan_t)m_yyscanner);
	}
}

int Scanner::scan()
{
	int tok;

get_next:
	tok = yylex((yyscan_t)m_yyscanner);

	/* Ignore T_COMMENT maybe we need it in reflection? */
	if (tok == T_COMMENT) {
		goto get_next;
	}

	return tok;
}

const char *Scanner::tokenText() const
{
	return yyget_text((yyscan_t)m_yyscanner);
}

int Scanner::tokenLength() const
{
	return yyget_leng((yyscan_t)m_yyscanner);
}

const std::string &Scanner::stringValue() const
{
	return string_buffer;
}

Scanner::~Scanner()
{
	yylex_destroy((yyscan_t)m_yyscanner);
}

}}
