#include <stdlib.h>
#include "qdmd.h"

/* ``Q_parsefile'' is defined in ``qdmd.g.y:272'' since it sets variables needed
 * for ``yyerror'' */

void Q_free(Q_dmd_t *dmd)
{
	Q_entity_t *ent;
	Q_column_t *col;
	Q_relation_t *rel;
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
		for (y = 0; y < ent->idxlen; y++) {
			free(ent->indices[y]);
		}
		free(ent->indices);
		free(ent->name);
		free(ent->title);
		free(ent->columns);
		free(ent);
	}
	for (i = 0; i < dmd->rellen; i++) {
		rel = dmd->relations[i];
		free(rel->atab);
		free(rel->acol);
		free(rel->btab);
		free(rel->bcol);
		free(rel);
	}
	free(dmd->title);
	free(dmd->lib);
	free(dmd->entities);
	free(dmd->relations);
	free(dmd);
}

