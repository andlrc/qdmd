%{
#include <stdio.h>
#include <string.h>
#include "safe.h"
#include "qdmd.h"

extern int yylex();
extern FILE *yyin;
extern int yylineno;
void yyerror(const char *err);

static Q_entity_t *genentity(void);
static void addentity(Q_dmd_t *dmd, Q_entity_t *entity);
static void addindex(Q_entity_t *ent, char *index);

static Q_relation_t *genrelation(char *atab, char *acol,
				  char *btab, char *bcol,
				  enum rel_type_e type);
static void addrelation(Q_dmd_t *dmd, Q_relation_t *relation);

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

%token TITLE LIB ENTITY COLUMN TYPE RELATION INDEX
%token <str> TEXT IDENT
%token NL

%type <str> title lib
%type <entity> entity entity_props
%type <columns> columns
%type <column> column column_props
%type <relation> relation

%%

root
	: nl root	/* Ignore */
	| title		{ root_dmd->title = $1; }
	| lib		{ root_dmd->lib = $1; }
	| entity	{ addentity(root_dmd, $1); }
	| relation	{ addrelation(root_dmd, $1); }
	| root title	{ root_dmd->title = $2; }
	| root lib	{ root_dmd->lib = $2; }
	| root entity	{ addentity(root_dmd, $2); }
	| root relation	{ addrelation(root_dmd, $2); }

title
	: TITLE ':' TEXT nl	{ $$ = $3; }

lib
	: LIB ':' TEXT nl	{ $$ = $3; }

entity
	: ENTITY ':' TEXT nl columns {
		$$ = genentity();
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
		$$ = genentity();
		$$->title = $3;
	}
	| INDEX ':' TEXT nl {
		$$ = genentity();
		addindex($$, $3);
	}
	| entity_props TITLE ':' TEXT nl {
		$$ = $1;
		$$->title = $4;
	}
	| entity_props INDEX ':' TEXT nl {
		$$ = $1;
		addindex($$, $4);
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
	: TYPE ':' TEXT nl TITLE ':' TEXT nl {
		$$ = smalloc(sizeof(Q_column_t));

		$$->type = $3;
		$$->title = $7;
	}
	| TITLE ':' TEXT nl TYPE ':' TEXT nl {
		$$ = smalloc(sizeof(Q_column_t));

		$$->title = $3;
		$$->type = $7;
	}

relation
	: RELATION ':' IDENT '.' IDENT '=' IDENT '.' IDENT NL {
		$$ = genrelation($3, $5, $7, $9, REL_TYPE_MANY_MANY);
	}
	| RELATION ':' IDENT '.' IDENT '-' IDENT '.' IDENT NL {
		$$ = genrelation($3, $5, $7, $9, REL_TYPE_ONE_ONE);
	}
	| RELATION ':' IDENT '.' IDENT '<' '=' IDENT '.' IDENT NL {
		$$ = genrelation($3, $5, $8, $10, REL_TYPE_ONE_MANY);
	}
	| RELATION ':' IDENT '.' IDENT '=' '>' IDENT '.' IDENT NL {
		$$ = genrelation($3, $5, $8, $10, REL_TYPE_MANY_ONE);
	}

nl
	: NL
	| ';'
	| NL nl
	| ';' nl
	

%%

static Q_entity_t *genentity(void)
{
	Q_entity_t *ent;
	ent = smalloc(sizeof(Q_entity_t));

	ent->name = 0;
	ent->title = 0;
	ent->columns = 0;
	ent->collen = 0;
	ent->colsize = 0;
	ent->indices = 0;
	ent->idxlen = 0;
	ent->idxsize = 0;

	return ent;
}

static void addentity(Q_dmd_t *dmd, Q_entity_t *entity)
{
	if (!dmd->entities) {
		dmd->entsize = 16;
		dmd->entities = smalloc(sizeof(Q_entity_t *) * dmd->entsize);
		dmd->entlen = 0;
	}

	dmd->entities[dmd->entlen++] = entity;

	if (dmd->entsize == dmd->entlen) {
		dmd->entsize *= 2;
		dmd->entities = srealloc(dmd->entities,
					 sizeof(Q_entity_t *) * dmd->entsize);
	}
}

static Q_relation_t *genrelation(char *atab, char *acol,
				  char *btab, char *bcol,
				  enum rel_type_e type)
{
	Q_relation_t *rel;

	rel = smalloc(sizeof(Q_relation_t));
	rel->atab = atab;
	rel->acol = acol;
	rel->btab = btab;
	rel->bcol = bcol;
	rel->type = type;

	return rel;
}
static void addrelation(Q_dmd_t *dmd, Q_relation_t *relation)
{
	if (!dmd->relations) {
		dmd->relsize = 16;
		dmd->relations = smalloc(sizeof(Q_relation_t *) * dmd->relsize);
		dmd->rellen = 0;
	}

	dmd->relations[dmd->rellen++] = relation;

	if (dmd->relsize == dmd->rellen) {
		dmd->relsize *= 2;
		dmd->relations = srealloc(dmd->relations,
					  sizeof(Q_relation_t *) * dmd->relsize);
	}
}

static void addindex(Q_entity_t *ent, char *index)
{
	if (!ent->indices) {
		ent->idxsize = 16;
		ent->indices = smalloc(sizeof(char *) * ent->idxsize);
		ent->idxlen = 0;
	}

	ent->indices[ent->idxlen++] = index;

	if (ent->idxsize == ent->idxlen) {
		ent->idxsize *= 2;
		ent->indices = srealloc(ent->indices,
					sizeof(char *) * ent->idxsize);
	}
}

static Q_dmd_t *parse(FILE *fp)
{
	root_dmd = smalloc(sizeof(Q_dmd_t));
	root_dmd->title = 0;
	root_dmd->lib = 0;
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
