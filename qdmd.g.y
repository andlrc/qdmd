%{
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "qdmd.h"

extern int yylex();
extern int yyerror(const char *);
extern FILE *yyin;
static Q_entity_t *gen_entity(char *name, char *title, Q_columns_t *columns);
static void add_entity(Q_dmd_t *dmd, Q_entity_t *entity);

static Q_dmd_t *root_dmd;
%}

%start root

%define parse.error verbose

%union {
	char *str;
	Q_entity_t *entity;
	Q_columns_t *columns;
	Q_column_t *column;
}

%token TITLE LIB ENTITY COLUMN TYPE
%token <str> TEXT
%token NL

%type <str> title lib
%type <entity> entity
%type <columns> columns
%type <column> column column_props

%%

root
	: title		{ root_dmd->title = $1; }
	| lib		{ root_dmd->lib = $1; }
	| entity	{ add_entity(root_dmd, $1); }
	| root title	{ root_dmd->title = $2; }
	| root lib	{ root_dmd->lib = $2; }
	| root entity	{ add_entity(root_dmd, $2); }

title
	: TITLE ':' TEXT nl	{ $$ = $3; }

lib
	: LIB ':' TEXT nl	{ $$ = $3; }

entity
	: ENTITY ':' TEXT nl columns {
		$$ = gen_entity($3, NULL, $5);
	}
	| ENTITY ':' TEXT nl TITLE ':' TEXT nl columns {
		$$ = gen_entity($3, $7, $9);
	}

columns
	: column {
		if (!($$ = malloc(sizeof(Q_columns_t)))) {
			perror("malloc");
			exit(EXIT_FAILURE);
		}
		if (!($$->columns = malloc(sizeof(Q_column_t *) * 64))) {
			perror("malloc");
			free($$);
			exit(EXIT_FAILURE);
		}
		$$->length = 0;
		$$->columns[$$->length++] = $1;
	}
	| columns column {
		$$ = $1;
		$$->columns[$$->length++] = $2;
	}

column
	: COLUMN ':' TEXT nl column_props {
		$$ = $5;
		$$->name = $3;
	}

column_props
	: TYPE ':' TEXT nl TITLE ':' TEXT nl {
		if (!($$ = malloc(sizeof(Q_column_t)))) {
			perror("malloc");
			exit(EXIT_FAILURE);
		}

		$$->type = $3;
		$$->title = $7;
	}
	| TITLE ':' TEXT nl TYPE ':' TEXT nl {
		if (!($$ = malloc(sizeof(Q_column_t)))) {
			perror("malloc");
			exit(EXIT_FAILURE);
		}

		$$->title = $3;
		$$->type = $7;
	}

nl
	: NL
	| ';'

%%

static Q_entity_t *gen_entity(char *name, char *title, Q_columns_t *columns)
{
	Q_entity_t *ent;
	if (!(ent = malloc(sizeof(Q_entity_t)))) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}

	ent->name = name;
	ent->title = title;
	ent->columns = columns->columns;
	ent->collen = columns->length;
	free(columns);

	return ent;
}

static void add_entity(Q_dmd_t *dmd, Q_entity_t *entity)
{
	if (dmd->entities == NULL) {
		if (!(dmd->entities = malloc(sizeof(Q_entity_t *) * 64))) {
			perror("malloc");
			exit(EXIT_FAILURE);
		}
		dmd->entlen = 0;
	}

	dmd->entities[dmd->entlen++] = entity;
}


static Q_dmd_t *parse(FILE *fp)
{
	if (!(root_dmd = malloc(sizeof(Q_dmd_t)))) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}

	yyin = fp;
	yyparse();

	return root_dmd;
}

Q_dmd_t *Q_parsefile(char *fname)
{
	Q_dmd_t *dmd = 0;
	FILE *fp;

	Q_setfilename(fname);

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

void Q_free(Q_dmd_t *dmd)
{
	Q_entity_t *ent;
	Q_column_t *col;
	int i, y;
	for (i = 0; i < dmd->entlen; i++) {
		ent = dmd->entities[i];
		for (y = 0; y < ent->collen; y++) {
			col = ent->columns[y];
			free(col->name);
			free(col->type);
			free(col->title);
			free(col);
		}
		free(ent->name);
		free(ent->title);
		free(ent->columns);
		free(ent);
	}
	free(dmd->title);
	free(dmd->lib);
	free(dmd->entities);
	free(dmd);
}
