%{
#include "qdmd.h"
#include "y.tab.h"

static void count();
static int prev_state = 0;
%}

%x XENTITY XTEXT

text	[^\t\n :]+[^\n]+
ws	[\t ]
nl	\r?\n

%%

<INITIAL,XENTITY>"TITLE"	{ count(); BEGIN(XTEXT); return TITLE; }
<INITIAL>"LIB"			{ count(); BEGIN(XTEXT); return LIB; }
<INITIAL,XENTITY>"ENTITY"	{ count(); prev_state = XENTITY; BEGIN(XTEXT); return ENTITY; }
<XENTITY>"COLUMN"		{ count(); BEGIN(XTEXT); return COLUMN; }
<XENTITY>"TYPE"			{ count(); BEGIN(XTEXT); return TYPE; }
<XTEXT>{text}			{ count(); yylval.str = strdup(yytext); BEGIN(prev_state); return TEXT; }
<*>{ws}				{ count(); }
<*>{nl}				{ count(); return NL; }
<*>.				{ count(); return yytext[0]; }

%%

int column = 0,
    line = 1;
static void count()
{
	int i;
#ifdef QDMD_DEBUG
	printf(">>%s<<\n", yytext);
#endif
	for (i = 0; yytext[i] != '\0'; i++) {
		if (yytext[i] == '\n') {
			column = 0;
			line++;
		}
		else if (yytext[i] == '\t') {
			column += 8 - (column % 8);
		}
		else {
			column++;
		}
	}
}

void yyerror(const char *err)
{
	fprintf(stderr,
		"%s:%d:%d %s\n",
		"INPUT_FILE",
		line, column, err);
}
