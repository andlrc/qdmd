#ifndef _H_QDMD_
#define _H_QDMD_ 1

#define PROGRAM_NAME "qdmd"
#define PROGRAM_VERSION "0.8.2"

struct Q_kv {
	char *key;
	char *value;
};

typedef struct {
	char *name;
	char *type;
	char *title;
	struct Q_kv **uiform;
	int uiformlen;
	int uiformsize;
	struct Q_kv **uigrid;
	int uigridlen;
	int uigridsize;
	struct Q_kv **values;
	int valueslen;
	int valuessize;
} Q_column_t;

typedef struct {
	Q_column_t **columns;
	int length;
	int size;
} Q_columns_t;

typedef struct {
	char *name;
	char *title;
	Q_column_t **columns;
	int collen;
	int colsize;
	char **indices;
	int idxlen;
	int idxsize;
} Q_entity_t;

#define REL_TYPE_A_ONE 1
#define REL_TYPE_A_MANY 2
#define REL_TYPE_B_ONE 4
#define REL_TYPE_B_MANY 8
enum rel_type_e {
	REL_TYPE_MANY_MANY = REL_TYPE_A_MANY + REL_TYPE_B_MANY,
	REL_TYPE_ONE_ONE = REL_TYPE_A_ONE + REL_TYPE_B_ONE,
	REL_TYPE_ONE_MANY = REL_TYPE_A_ONE + REL_TYPE_B_MANY,
	REL_TYPE_MANY_ONE = REL_TYPE_A_MANY + REL_TYPE_B_ONE
};

typedef struct {
	enum rel_type_e type;
	char *atab;
	char *acol;
	char *btab;
	char *bcol;
} Q_relation_t;

typedef struct {
	char *title;
	Q_entity_t **entities;
	int entlen;
	int entsize;
	Q_relation_t **relations;
	int rellen;
	int relsize;
} Q_dmd_t;

Q_dmd_t *Q_parsefile(char *fname);

Q_entity_t *Q_genentity(void);
void Q_addentity(Q_dmd_t * dmd, Q_entity_t * entity);

Q_relation_t *Q_genrelation(char *atab, char *acol,
			    char *btab, char *bcol, enum rel_type_e type);
void Q_addrelation(Q_dmd_t * dmd, Q_relation_t * relation);

Q_column_t *Q_gencolumn(void);

void Q_addkv(struct Q_kv ***kv, char *key, char *value, int *size, int *len);

void Q_addvalues(Q_column_t *col, char *values);

void Q_addindex(Q_entity_t * ent, char *index);

void Q_free(Q_dmd_t * dmd);

#endif
