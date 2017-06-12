#ifndef _H_QDMD_
#define _H_QDMD_ 1

#define PROGRAM_NAME "qdmd"
#define PROGRAM_VERSION "0.2.0"

typedef struct {
	char *name;
	char *type;
	char *title;
} Q_column_t;

typedef struct {
	Q_column_t **columns;
	int length;
} Q_columns_t;

typedef struct {
	char *name;
	char *title;
	Q_column_t **columns;
	int collen;
} Q_entity_t;

typedef struct {
	char *title;
	char *lib;
	Q_entity_t **entities;
	int entlen;
} Q_dmd_t;

void Q_setfilename(char *fname);
void Q_free(Q_dmd_t *dmd);
Q_dmd_t *Q_parsefile(char *fname);

#endif
