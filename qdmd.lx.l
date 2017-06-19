%{
#include "qdmd.h"
#include "y.tab.h"

%}

%option stack
%x XENTITY XREL XTEXT

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
	"COLUMN"	{ return COLUMN; }
	"TYPE"		{ return TYPE; }
	"RELATION"	{ yy_push_state(XREL); return RELATION; }
}

<XREL>{
	":"		{ return yytext[0]; }
	{ident}		{ yylval.str = strdup(yytext); return IDENT; }
	{nl}		{ yy_push_state(INITIAL); return NL; }
}

<XTEXT>{
	{text}		{
		for (;*yytext == '\t' || *yytext == ' '; yytext++)
			/* Remove leading white spaces */;
		yylval.str = strdup(yytext);
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
