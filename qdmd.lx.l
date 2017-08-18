%{
#include "qdmd.h"
#include "safe.h"
#include "y.tab.h"

%}

%option stack
%x XENTITY XREL XIDENT XTEXT

comm	"#"[^\n]*[\n]
ident	[a-zA-Z_][a-zA-Z0-9_]*
text	[^\n;]+
ws	[\t ]
nl	(\r?\n)+

%%

<INITIAL>{
	"TITLE"		{ return TITLE; }
	"LIB"		{ return LIB; }
	"ENTITY"	{ yy_push_state(XENTITY); return ENTITY; }
}

<XENTITY>{
	"ENTITY"	{ return ENTITY; }
	"TITLE"		{ return TITLE; }
	"INDEX"		{ return INDEX; }
	"UIFORM"	{ return UIFORM; }
	"UIGRID"	{ return UIGRID; }
	"COLUMN"	{ return COLUMN; }
	"TYPE"		{ return TYPE; }
	"VALUES"	{ return VALUES; }
	"RELATION"	{ yy_push_state(XREL); return RELATION; }
	"."		{ yy_push_state(XIDENT); return yytext[0]; }
}

<XREL>{
	":"		{ return yytext[0]; }
	{ident}		{ yylval.str = sstrdup(yytext); return IDENT; }
	{nl}		{ yy_pop_state(); return NL; }
}

<XIDENT>{
	{ident}		{
		yylval.str = sstrdup(yytext);
		yy_pop_state();
		return IDENT;
	}
}

<XTEXT>{
	{text}		{
		for (;*yytext == '\t' || *yytext == ' '; yytext++)
			/* Remove leading white spaces */;
		yylval.str = sstrdup(yytext);
		yy_pop_state();
		return TEXT;
	}
}

<*>{
	{ws}		/* Nothing */
	{nl}		{ return NL; }
	{comm}		/* Comment */
	":"		{ yy_push_state(XTEXT); return yytext[0]; }
	.		{ return yytext[0]; }
}

%%
