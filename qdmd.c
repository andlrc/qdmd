#include <stdlib.h>
#include <string.h>
#include "safe.h"
#include "qdmd.h"

/* ``Q_parsefile'' is defined in ``qdmd.g.y:181'' since it sets variables needed
 * for ``yyerror'' and the yacc parser in general */

Q_entity_t *Q_genentity(void)
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

void Q_addentity(Q_dmd_t * dmd, Q_entity_t * entity)
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

Q_relation_t *Q_genrelation(char *atab, char *acol,
			    char *btab, char *bcol, enum rel_type_e type)
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

void Q_addrelation(Q_dmd_t * dmd, Q_relation_t * relation)
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

Q_column_t *Q_gencolumn(void)
{
	Q_column_t *col;
	col = smalloc(sizeof(Q_column_t));

	col->name = 0;
	col->title = 0;
	col->type = 0;

	col->uiformsize = 8;
	col->uiform = smalloc(sizeof(struct Q_kv *) * col->uiformsize);
	col->uiformlen = 0;

	col->uigridsize = 8;
	col->uigrid = smalloc(sizeof(struct Q_kv *) * col->uigridsize);
	col->uigridlen = 0;

	col->valuessize = 8;
	col->values = smalloc(sizeof(struct Q_kv *) * col->valuessize);
	col->valueslen = 0;

	return col;
}

void Q_addkv(struct Q_kv ***kv, char *key, char *value, int *size, int *len)
{
	struct Q_kv *pair;

	pair = smalloc(sizeof(struct Q_kv));
	pair->key = sstrdup(key);
	pair->value = sstrdup(value);
	(*kv)[*len] = pair;
	*len = *len + 1;

	if (*size == *len) {
		*size = *size * 2;
		*kv = srealloc(*kv, sizeof(struct Q_kv *) * *size);
	}
}

void Q_addvalues(Q_column_t *col, char *values)
{
	/* values is a string like: key1:val1,key2:val2 */
	char *kv, *key, *val;

	while ((kv = strsep(&values, ","))) {
		key = strsep(&kv, ":");
		val = kv;

		Q_addkv(&col->values, key, val,
			&col->valuessize, &col->valueslen);
	}
}

void Q_addindex(Q_entity_t * ent, char *index)
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

void Q_free(Q_dmd_t * dmd)
{
	Q_entity_t *ent;
	Q_column_t *col;
	Q_relation_t *rel;
	struct Q_kv *kv;
	int i, j, k;
	for (i = 0; i < dmd->entlen; i++) {
		ent = dmd->entities[i];
		for (j = 0; j < ent->collen; j++) {
			col = ent->columns[j];
			free(col->name);
			free(col->type);
			free(col->title);
			for (k = 0; k < col->uiformlen; k++) {
				kv = col->uiform[k];
				free(kv->key);
				free(kv->value);
				free(kv);
			}
			free(col->uiform);
			for (k = 0; k < col->uigridlen; k++) {
				kv = col->uigrid[k];
				free(kv->key);
				free(kv->value);
				free(kv);
			}
			free(col->uigrid);
			for (k = 0; k < col->valueslen; k++) {
				kv = col->values[k];
				free(kv->key);
				free(kv->value);
				free(kv);
			}
			free(col->values);
			free(col);
		}
		for (j = 0; j < ent->idxlen; j++) {
			free(ent->indices[j]);
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
	free(dmd->entities);
	free(dmd->relations);
	free(dmd);
}
