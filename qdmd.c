#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "qdmd.h"
#include "y.tab.h"

static void print_dmd(Q_dmd_t *dmd);
static int gettyplen(char *type, int *len, char *enttype)
{
	char *p;

	for (p = enttype; *p != '\0' && *p != '('; p++) {
		*type++ = *p;
	}


	if (*p++ == '(') {
		*type = '\0';
		*len = atoi(p);
	}
	else {
		*len = 0;
	}

	return 0;
}

int main(int argc, char **argv)
{
	Q_dmd_t *dmd;
	char *infile;
	int argind;

	argind = 1;
	infile = "-";

	do {
		if (argind < argc)
			infile = argv[argind];

		if ((dmd = Q_parsefile(infile))) {
			print_dmd(dmd);
			Q_free(dmd);
		}

	} while (++argind < argc);

	return 0;
}

static void print_dmd(Q_dmd_t *dmd)
{
	Q_entity_t *ent;
	Q_column_t *col;
	int i, y, len;
	char type[32];

	printf("{\n"
	       "\t\"model\": {\n"
	       "\t\t\"proxy\": {\n"
	       "\t\t\t\"url\": \"/ip2-services/ip2dmdProxy.aspx\"\n"
	       "\t\t},\n"
	       "\t\t\"entities\": {\n");
	for (i = 0; i < dmd->entlen; i++) {
		ent = dmd->entities[i];
		printf("\t\t\t\"%s\": {\n", ent->name);
		if (ent->title)
			printf("\t\t\t\t\"title\": \"%s\",\n", ent->title);
		printf("\t\t\t\t\"columns\": [\n");
		for (y = 0; y < ent->collen; y++) {
			col = ent->columns[y];
			gettyplen(type, &len, col->type);
			printf("\t\t\t\t\t{\n"
			       "\t\t\t\t\t\t\"type\": \"%s\",\n"
			       "\t\t\t\t\t\t\"name\": \"%s\",\n"
			       "\t\t\t\t\t\t\"title\": \"%s\"",
			       type, col->name, col->title);

			if (len) {
				printf(",\n\t\t\t\t\t\t\"len\": %d\n", len);
			} else {
				printf("\n");
			}

			printf("\t\t\t\t\t}%s\n",
			       y + 1 < ent->collen ? "," : "");
		}
		printf("\t\t\t\t],\n"
		       "\t\t\t\t\"designer\": {\n"
		       "\t\t\t\t},\n"
		       "\t\t\t\t\"ui\": {\n"
		       "\t\t\t\t\t\"grids\": {\n"
		       "\t\t\t\t\t\t\"portfolio\": {\n"
		       "\t\t\t\t\t\t\t\"columns\": [\n");
		for (y = 0; y < ent->collen; y++) {
			col = ent->columns[y];
			printf("\t\t\t\t\t\t\t\t{\n");
			printf("\t\t\t\t\t\t\t\t\t\"name\": \"%s\",\n"
			       "\t\t\t\t\t\t\t\t\t\"title\": \"%s\",\n"
			       "\t\t\t\t\t\t\t\t\t\"showned\": true,\n"
			       "\t\t\t\t\t\t\t\t\t\"hidden\": false,\n"
			       "\t\t\t\t\t\t\t\t\t\"width\": 100,\n"
			       "\t\t\t\t\t\t\t\t\t\"isTitle\": false\n"
			       "\t\t\t\t\t\t\t\t}%s\n",
			       col->name, col->title,
			       y + 1 < ent->collen ? "," : "");
		}
		printf("\t\t\t\t\t\t\t]\n"
		       "\t\t\t\t\t\t}\n"
		       "\t\t\t\t\t},\n"
		       "\t\t\t\t\t\"forms\": {\n"
		       "\t\t\t\t\t\t\"portfolio\": {\n"
		       "\t\t\t\t\t\t\t\"columns\": [\n");
		for (y = 0; y < ent->collen; y++) {
			col = ent->columns[y];
			printf("\t\t\t\t\t\t\t\t{\n");
			printf("\t\t\t\t\t\t\t\t\t\"name\": \"%s\"\n"
			       "\t\t\t\t\t\t\t\t}%s\n",
			       col->name,
			       y + 1 < ent->collen ? "," : "");
		}
		printf("\t\t\t\t\t\t\t],\n"
		       "\t\t\t\t\t\t\t\"items\": {\n"
		       "\t\t\t\t\t\t\t\t\"columns\": [\n");
		for (y = 0; y < ent->collen; y++) {
			col = ent->columns[y];
			gettyplen(type, &len, col->type);
			if (strcmp(type, "integer") == 0)
				strcpy(type, "number");

			printf("\t\t\t\t\t\t\t\t\t{\n"
			       "\t\t\t\t\t\t\t\t\t\t\"name\": \"%s\",\n"
			       "\t\t\t\t\t\t\t\t\t\t\"type\": \"%s\",\n"
			       "\t\t\t\t\t\t\t\t\t\t\"title\": \"%s\",\n"
			       "\t\t\t\t\t\t\t\t\t\t\"fieldLabel\": \"%s\",\n"
			       "\t\t\t\t\t\t\t\t\t\t\"syncEvent\": \"change\"",
			       col->name, type, col->title, col->title);

			if (len) {
				printf(",\n\t\t\t\t\t\t\t\t\t\t\"len\": %d\n",
				       len);
			} else {
				printf("\n");
			}


			printf("\t\t\t\t\t\t\t\t\t}%s\n",
			       y + 1 < ent->collen ? "," : "");
		}
		printf("\t\t\t\t\t\t\t\t]\n"
		       "\t\t\t\t\t\t\t},\n"
		       "\t\t\t\t\t\t\t\"groups\": {\n"
		       "\t\t\t\t\t\t\t\t\"system\": {\n"
		       "\t\t\t\t\t\t\t\t\t\"name\": \"system\",\n"
		       "\t\t\t\t\t\t\t\t\t\"title\": {\n"
		       "\t\t\t\t\t\t\t\t\t\t\"da\": \"System\",\n"
		       "\t\t\t\t\t\t\t\t\t\t\"en\": \"System\"\n"
		       "\t\t\t\t\t\t\t\t\t}\n"
		       "\t\t\t\t\t\t\t\t}\n"
		       "\t\t\t\t\t\t\t}\n"
		       "\t\t\t\t\t\t}\n"
		       "\t\t\t\t\t}\n"
		       "\t\t\t\t},\n");
		printf("\t\t\t\t\"idColumn\": \"%s\",\n",
		       ent->columns[0]->name);
		printf("\t\t\t\t\"widgets\": {\n"
		       "\t\t\t\t},\n"
		       "\t\t\t\t\"permission\": {\n"
		       "\t\t\t\t\t\"add\": {\n"
		       "\t\t\t\t\t\t\"form\": true,\n"
		       "\t\t\t\t\t\t\"grid\": true\n"
		       "\t\t\t\t\t},\n"
		       "\t\t\t\t\t\"edit\": {\n"
		       "\t\t\t\t\t\t\"form\": true,\n"
		       "\t\t\t\t\t\t\"grid\": true\n"
		       "\t\t\t\t\t},\n"
		       "\t\t\t\t\t\"remove\": {\n"
		       "\t\t\t\t\t\t\"form\": true,\n"
		       "\t\t\t\t\t\t\"grid\": true\n"
		       "\t\t\t\t\t}\n"
		       "\t\t\t\t}\n"
		       "\t\t\t}%s\n", i + 1 < dmd->entlen ? "," : "");
	}
	printf("\t\t},\n"
	       "\t\t\"types\": {\n"
	       "\t\t},\n"
	       "\t\t\"relations\": {\n"
	       "\t\t},\n"
	       "\t\t\"languages\": {\n"
	       "\t\t},\n"
	       "\t\t\"navigator\": {\n"
	       "\t\t},\n"
	       "\t\t\"menu\": {\n"
	       "\t\t},\n"
	       "\t\t\"designer\": {\n"
	       "\t\t},\n"
	       "\t\t\"name\": \"%s\",\n"
	       "\t\t\"title\": {\n"
	       "\t\t\t\"da\": \"%s\"\n"
	       "\t\t},\n"
	       "\t\t\"require\": [\n"
	       "\t\t]\n"
	       "\t}\n"
	       "}\n",
	       dmd->lib, dmd->title);
}
