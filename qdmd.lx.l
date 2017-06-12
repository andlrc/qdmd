%{
#include "qdmd.h"
#include "y.tab.h"

static void count();
static int prev_state = 0;
static char *cur_file;
%}

%x XENTITY XREL XTEXT

ident	[a-zA-Z_][a-zA-Z0-9_]*
text	[^\t\n :]+[^\n]+
ws	[\t ]
nl	(\r?\n)+

%%

	/* Root */
<INITIAL>"TITLE"		{ count(); BEGIN(XTEXT); return TITLE; }
<INITIAL>"LIB"			{ count(); BEGIN(XTEXT); return LIB; }

	/* Entity */
<INITIAL,XENTITY>"ENTITY"	{
	count();
	prev_state = XENTITY;
	BEGIN(XTEXT);
	return ENTITY;
}
<XENTITY>"TITLE"		{ count(); BEGIN(XTEXT); return TITLE; }
<XENTITY>"COLUMN"		{ count(); BEGIN(XTEXT); return COLUMN; }
<XENTITY>"TYPE"			{ count(); BEGIN(XTEXT); return TYPE; }

	/* Relation */
<INITIAL,XENTITY>"RELATION"	{
	count();
	BEGIN(XREL);
	return RELATION;
}
<XREL>{ident}			{
	count();
	yylval.str = strdup(yytext);
	return IDENT;
}
<XREL>{nl}			{ count(); BEGIN(INITIAL); return NL; }

	/* Text */
<XTEXT>{text}			{
	count();
	yylval.str = strdup(yytext);
	BEGIN(prev_state);
	return TEXT;
}

	/* Other */
<*>{ws}				{ count(); }
<*>{nl}				{ count(); return NL; }
<*>.				{ count(); return yytext[0]; }

%%

static int column = 0, line = 1;
static void count()
{
	int i;
#ifdef QDMD_DEBUG
	fprintf(stderr, ">>%s<<\n", yytext);
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

void Q_setfilename(char *fname)
{
	column = 0;
	line = 1;
	prev_state = INITIAL;
	BEGIN(INITIAL);
	cur_file = fname;
}

void yyerror(const char *err)
{
	fprintf(stderr,
		"%s:%d:%d %s\n",
		cur_file,
		line, column, err);
}
