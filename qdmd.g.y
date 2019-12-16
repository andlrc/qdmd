%{
#include <stdio.h>
#include <string.h>
#include "safe.h"
#include "qdmd.h"

extern int yylex();
extern FILE *yyin;
extern int yylineno;
void yyerror(const char *err);

/* Used by ``yyerror'' and the ``root'' rule in the parser.
 * Reset whenever ``Q_parsefile'' is called */
static char *root_file;
static Q_dmd_t *root_dmd;

%}

%start root

%define parse.error verbose

%union {
	char *str;
	Q_entity_t *entity;
	Q_columns_t *columns;
	Q_column_t *column;
	Q_relation_t *relation;
}

%token TITLE ENTITY COLUMN TYPE RELATION INDEX UIFORM UIGRID VALUES
%token <str> TEXT IDENT
%token NL

%type <str> title
%type <entity> entity entity_props
%type <columns> columns
%type <column> column column_props
%type <relation> relation

%%

root
	: nl root	/* Ignore */
	| title		{ root_dmd->title = $1; }
	| entity	{ Q_addentity(root_dmd, $1); }
	| relation	{ Q_addrelation(root_dmd, $1); }
	| root title	{ root_dmd->title = $2; }
	| root entity	{ Q_addentity(root_dmd, $2); }
	| root relation	{ Q_addrelation(root_dmd, $2); }

title
	: TITLE ':' TEXT nl	{ $$ = $3; }

entity
	: ENTITY ':' TEXT nl columns {
		$$ = Q_genentity();
		$$->name = $3;
		$$->columns = $5->columns;
		$$->collen = $5->length;
		$$->colsize = $5->size;
		free($5);
	}
	| ENTITY ':' TEXT nl entity_props columns {
		$$ = $5;
		$$->name = $3;
		$$->columns = $6->columns;
		$$->collen = $6->length;
		$$->colsize = $6->size;
		free($6);
	}

entity_props
	: TITLE ':' TEXT nl {
		$$ = Q_genentity();
		$$->title = $3;
	}
	| INDEX ':' TEXT nl {
		$$ = Q_genentity();
		Q_addindex($$, $3);
	}
	| entity_props TITLE ':' TEXT nl {
		$$ = $1;
		$$->title = $4;
	}
	| entity_props INDEX ':' TEXT nl {
		$$ = $1;
		Q_addindex($$, $4);
	}

columns
	: column {
		$$ = smalloc(sizeof(Q_columns_t));
		$$->size = 16;
		$$->columns = smalloc(sizeof(Q_column_t *) * $$->size);
		$$->length = 0;
		$$->columns[$$->length++] = $1;
	}
	| columns column {
		$$ = $1;
		$$->columns[$$->length++] = $2;
		if ($$->size == $$->length) {
			$$->size *= 2;
			$$->columns = srealloc($$->columns,
					       sizeof(Q_column_t *) * $$->size);
		}
	}

column
	: COLUMN ':' TEXT nl column_props {
		$$ = $5;
		$$->name = $3;
	}

column_props
	: TYPE ':' TEXT nl {
		$$ = Q_gencolumn();
		$$->type = $3;
	}
	| TITLE ':' TEXT nl {
		$$ = Q_gencolumn();
		$$->title = $3;
	}
	| UIFORM '.' IDENT ':' TEXT nl {
		$$ = Q_gencolumn();
		Q_addkv(&$$->uiform, $3, $5, &$$->uiformsize, &$$->uiformlen);
		free($3);
		free($5);
	}
	| UIGRID '.' IDENT ':' TEXT nl {
		$$ = Q_gencolumn();
		Q_addkv(&$$->uigrid, $3, $5, &$$->uigridsize, &$$->uigridlen);
		free($3);
		free($5);
	}
	| VALUES ':' TEXT nl {
		$$ = Q_gencolumn();
		Q_addvalues($$, $3);
	}
	| column_props TYPE ':' TEXT nl {
		$$ = $1;
		$$->type = $4;
	}
	| column_props TITLE ':' TEXT nl {
		$$ = $1;
		$$->title = $4;
	}
	| column_props UIFORM '.' IDENT ':' TEXT nl {
		$$ = $1;
		Q_addkv(&$$->uiform, $4, $6, &$$->uiformsize, &$$->uiformlen);
		free($4);
		free($6);
	}
	| column_props UIGRID '.' IDENT ':' TEXT nl {
		$$ = $1;
		Q_addkv(&$$->uigrid, $4, $6, &$$->uigridsize, &$$->uigridlen);
		free($4);
		free($6);
	}
	| column_props VALUES ':' TEXT nl {
		$$ = $1;
		Q_addvalues($$, $4);
	}

relation
	: RELATION ':' IDENT '.' IDENT '=' IDENT '.' IDENT nl {
		$$ = Q_genrelation($3, $5, $7, $9, REL_TYPE_MANY_MANY);
	}
	| RELATION ':' IDENT '.' IDENT '-' IDENT '.' IDENT nl {
		$$ = Q_genrelation($3, $5, $7, $9, REL_TYPE_ONE_ONE);
	}
	| RELATION ':' IDENT '.' IDENT '<' '=' IDENT '.' IDENT nl {
		$$ = Q_genrelation($3, $5, $8, $10, REL_TYPE_ONE_MANY);
	}
	| RELATION ':' IDENT '.' IDENT '=' '>' IDENT '.' IDENT nl {
		$$ = Q_genrelation($3, $5, $8, $10, REL_TYPE_MANY_ONE);
	}

nl
	: NL
	| ';'
	| NL nl
	| ';' nl
	

%%

static Q_dmd_t *parse(FILE *fp)
{
	root_dmd = smalloc(sizeof(Q_dmd_t));
	root_dmd->title = 0;
	root_dmd->entities = 0;
	root_dmd->entlen = 0;
	root_dmd->relations = 0;
	root_dmd->rellen = 0;

	yyin = fp;
	if (yyparse()) {
		Q_free(root_dmd);
		return NULL;
	}

	return root_dmd;
}

void yyerror(const char *err)
{
	fprintf(stderr,
		"%s:%d %s\n",
		root_file, yylineno, err);
}

Q_dmd_t *Q_parsefile(char *fname)
{
	Q_dmd_t *dmd = 0;
	FILE *fp;

	root_file = fname;

	if (strcmp(fname, "-") == 0) {
		dmd = parse(stdin);
	} else {
		if (!(fp = fopen(fname, "r"))) {
			perror(fname);
		} else {
			dmd = parse(fp);
			fclose(fp);
		}
	}

	return dmd;
}
