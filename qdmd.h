#ifndef _H_QDMD_
#define _H_QDMD_ 1

#define PROGRAM_NAME "qdmd"
#define PROGRAM_VERSION "0.7.2"

typedef struct {
	char *name;
	char *type;
	char *title;
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
	char *lib;
	Q_entity_t **entities;
	int entlen;
	int entsize;
	Q_relation_t **relations;
	int rellen;
	int relsize;
} Q_dmd_t;

void Q_setfilename(char *fname);
void Q_free(Q_dmd_t *dmd);
Q_dmd_t *Q_parsefile(char *fname);

#endif
