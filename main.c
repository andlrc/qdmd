#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <limits.h>
#include "qdmd.h"
#include "y.tab.h"

static void print_version(void)
{
	printf("%s: %s\n", PROGRAM_NAME, PROGRAM_VERSION);
}

static void print_usage(void)
{
	fprintf(stderr, "Usage: %s [OPTION] <file>\n", PROGRAM_NAME);
}

static void print_help(void)
{
	printf("Usage: %s [OPTION] <file>\n"
	       "  -i, --inplace      Substitute extension with dmd and write to that file\n"
	       "  -h, --help         Show this help and exit\n"
	       "  -V, --version      Output version information\n",
	       PROGRAM_NAME);
}

static struct option const long_options[] = {
	{"version", no_argument, NULL, 'V'},
	{"help", no_argument, NULL, 'h'},
	{"inplace", no_argument, NULL, 'i'},
	{NULL, 0, NULL, 0}
};

static char short_options[] = "ihV";

static int getoutfile(char *outfile, size_t size, char *infile);
static int gettyplen(char *type, int *len, char *enttype);
static void print_dmd(FILE * fp, Q_dmd_t * dmd);

int main(int argc, char **argv)
{
	int c, argind, iflag = 0;
	Q_dmd_t *dmd;
	FILE *outfp;
	char *infile, outfile[PATH_MAX];

	while ((c = getopt_long(argc, argv, short_options, long_options,
				NULL)) != -1) {
		switch (c) {
		case 'V':
			print_version();
			return 0;
			break;
		case 'h':
			print_help();
			return 0;
			break;
		case 'i':
			iflag++;
			break;
		default:
			print_usage();
			return 1;
			break;
		}
	}

	argind = optind;
	infile = "-";

	do {
		if (argind < argc)
			infile = argv[argind];

		if ((dmd = Q_parsefile(infile))) {
			if (strcmp(infile, "-") != 0 && iflag) {
				if (getoutfile(outfile,
					       sizeof(outfile), infile)) {
					fprintf(stderr,
						"Failed to convert '%s' using stdout instead\n",
						infile);
					goto print_stdout;
				}
				if (!(outfp = fopen(outfile, "w"))) {
					perror(outfile);
					continue;
				}
				print_dmd(outfp, dmd);
				fclose(outfp);
			} else {
			      print_stdout:
				print_dmd(stdout, dmd);
			}
			Q_free(dmd);
		}

	} while (++argind < argc);

	return 0;
}

static int getoutfile(char *outfile, size_t size, char *infile)
{
	char *lastdot;
	size_t reqsize;

	lastdot = strrchr(infile, '.');

	if (!lastdot)
		return 1;

	reqsize = lastdot - infile + 5;	/* .dmd + \0 */

	if (size < reqsize)
		return 1;

	memcpy(outfile, infile, lastdot - infile);
	memcpy(outfile + (lastdot - infile), ".dmd", 5);
	return 0;
}

static int gettyplen(char *type, int *len, char *enttype)
{
	char *p, *ptype;

	for (ptype = type, p = enttype; *p != '\0' && *p != '('; p++) {
		*ptype++ = *p;
	}


	*ptype = '\0';
	if (*p++ == '(') {
		*len = atoi(p);

		if (*len >= 1024) {
			strcpy(type, "textarea");
		}
	} else {
		*len = 0;
	}

	return 0;
}

static void print_dmd(FILE * fp, Q_dmd_t * dmd)
{
	Q_entity_t *ent;
	Q_column_t *col;
	Q_relation_t *rel;
	struct Q_kv *kv;
	int i, j, k, len;
	char type[32];

	fprintf(fp, "{\n"
		"\t\"model\": {\n"
		"\t\t\"proxy\": {\n"
		"\t\t\t\"url\": \"/ip2-services/ip2dmdProxy.aspx\"\n"
		"\t\t},\n" "\t\t\"entities\": {\n");
	for (i = 0; i < dmd->entlen; i++) {
		ent = dmd->entities[i];
		fprintf(fp, "\t\t\t\"%s\": {\n", ent->name);
		if (ent->title)
			fprintf(fp, "\t\t\t\t\"title\": \"%s\",\n",
				ent->title);
		fprintf(fp, "\t\t\t\t\"keys\": [\n");
		for (j = 0; j < ent->idxlen; j++) {
			fprintf(fp, "\t\t\t\t\t{\n"
				"\t\t\t\t\t\t\"columns\": [\n"
				"\t\t\t\t\t\t\t\"%s\"\n"
				"\t\t\t\t\t\t]\n"
				"\t\t\t\t\t}%s\n",
				ent->indices[j],
				j + 1 < ent->idxlen ? "," : "");
		}
		fprintf(fp, "\t\t\t\t],\n");
		fprintf(fp, "\t\t\t\t\"columns\": [\n");
		/* Print grids */
		for (j = 0; j < ent->collen; j++) {
			col = ent->columns[j];
			gettyplen(type, &len, col->type);
			strcpy(type, col->type);
			fprintf(fp, "\t\t\t\t\t{\n");
			for (k = 0; k < col->uigridlen; k++) {
				kv = col->uigrid[k];
				if (strcmp(kv->key, "type") == 0)
					strcpy(type, kv->value);
				else
					fprintf(fp,
						"\t\t\t\t\t\t\"%s\": \"%s\",\n",
						kv->key, kv->value);
			}
			fprintf(fp, "\t\t\t\t\t\t\"type\": \"%s\",\n"
				"\t\t\t\t\t\t\"name\": \"%s\",\n"
				"\t\t\t\t\t\t\"title\": \"%s\"",
				type, col->name, col->title);

			if (len) {
				fprintf(fp, ",\n\t\t\t\t\t\t\"len\": %d\n",
					len);
			} else {
				fprintf(fp, "\n");
			}

			fprintf(fp, "\t\t\t\t\t}%s\n",
				j + 1 < ent->collen ? "," : "");
		}
		fprintf(fp, "\t\t\t\t],\n"
			"\t\t\t\t\"designer\": {\n"
			"\t\t\t\t},\n"
			"\t\t\t\t\"ui\": {\n"
			"\t\t\t\t\t\"grids\": {\n"
			"\t\t\t\t\t\t\"portfolio\": {\n"
			"\t\t\t\t\t\t\t\"columns\": [\n");
		for (j = 0; j < ent->collen; j++) {
			col = ent->columns[j];
			fprintf(fp, "\t\t\t\t\t\t\t\t{\n");
			fprintf(fp, "\t\t\t\t\t\t\t\t\t\"name\": \"%s\",\n"
				"\t\t\t\t\t\t\t\t\t\"title\": \"%s\",\n"
				"\t\t\t\t\t\t\t\t\t\"showned\": true,\n"
				"\t\t\t\t\t\t\t\t\t\"hidden\": false,\n"
				"\t\t\t\t\t\t\t\t\t\"flex\": 1,\n"
				"\t\t\t\t\t\t\t\t\t\"isTitle\": false\n"
				"\t\t\t\t\t\t\t\t}%s\n",
				col->name, col->title,
				j + 1 < ent->collen ? "," : "");
		}
		fprintf(fp, "\t\t\t\t\t\t\t]\n"
			"\t\t\t\t\t\t}\n"
			"\t\t\t\t\t},\n"
			"\t\t\t\t\t\"forms\": {\n"
			"\t\t\t\t\t\t\"portfolio\": {\n"
			"\t\t\t\t\t\t\t\"columns\": [\n");
		for (j = 0; j < ent->collen; j++) {
			col = ent->columns[j];
			fprintf(fp, "\t\t\t\t\t\t\t\t{\n");
			fprintf(fp, "\t\t\t\t\t\t\t\t\t\"name\": \"%s\"\n"
				"\t\t\t\t\t\t\t\t}%s\n",
				col->name, j + 1 < ent->collen ? "," : "");
		}
		fprintf(fp, "\t\t\t\t\t\t\t],\n"
			"\t\t\t\t\t\t\t\"items\": {\n"
			"\t\t\t\t\t\t\t\t\"columns\": [\n");
		/* Print forms */
		for (j = 0; j < ent->collen; j++) {
			col = ent->columns[j];
			gettyplen(type, &len, col->type);
			if (strcmp(type, "integer") == 0)
				strcpy(type, "number");

			fprintf(fp, "\t\t\t\t\t\t\t\t\t{\n"
				"\t\t\t\t\t\t\t\t\t\t\"name\": \"%s\",\n",
				col->name);
			for (k = 0; k < col->uiformlen; k++) {
				kv = col->uiform[k];
				if (strcmp(kv->key, "type") == 0)
					strcpy(type, kv->value);
				else
					fprintf(fp,
						"\t\t\t\t\t\t\t\t\t\t\"%s\": \"%s\",\n",
						kv->key, kv->value);
			}

			fprintf(fp,
				"\t\t\t\t\t\t\t\t\t\t\"type\": \"%s\",\n"
				"\t\t\t\t\t\t\t\t\t\t\"title\": \"%s\",\n"
				"\t\t\t\t\t\t\t\t\t\t\"anchor\": \"100%%\",\n"
				"\t\t\t\t\t\t\t\t\t\t\"fieldLabel\": \"%s\"",
				type, col->title, col->title);

			if (len) {
				fprintf(fp,
					",\n\t\t\t\t\t\t\t\t\t\t\"len\": %d\n",
					len);
			} else {
				fprintf(fp, "\n");
			}


			fprintf(fp, "\t\t\t\t\t\t\t\t\t}%s\n",
				j + 1 < ent->collen ? "," : "");
		}
		fprintf(fp, "\t\t\t\t\t\t\t\t]\n"
			"\t\t\t\t\t\t\t}\n"
			"\t\t\t\t\t\t}\n" "\t\t\t\t\t}\n" "\t\t\t\t},\n");
		fprintf(fp, "\t\t\t\t\"idColumn\": \"%s\",\n",
			ent->columns[0]->name);
		fprintf(fp, "\t\t\t\t\"widgets\": {\n"
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
	fprintf(fp, "\t\t},\n"
		"\t\t\"types\": {\n"
		"\t\t},\n"
		"\t\t\"relations\": [\n");
	for (i = 0; i < dmd->rellen; i++) {
		rel = dmd->relations[i];
		fprintf(fp, "\t\t\t{\n"
			"\t\t\t\t\"between\": {\n"
			"\t\t\t\t\t\"entity\": \"%s\",\n"
			"\t\t\t\t\t\"on\": [\n"
			"\t\t\t\t\t\t{\n"
			"\t\t\t\t\t\t\t\"name\": \"%s\"\n"
			"\t\t\t\t\t\t}\n"
			"\t\t\t\t\t],\n"
			"\t\t\t\t\t\"occurrence\": \"%s\"\n"
			"\t\t\t\t},\n"
			"\t\t\t\t\"and\": {\n"
			"\t\t\t\t\t\"entity\": \"%s\",\n"
			"\t\t\t\t\t\"on\": [\n"
			"\t\t\t\t\t\t{\n"
			"\t\t\t\t\t\t\t\"name\": \"%s\"\n"
			"\t\t\t\t\t\t}\n"
			"\t\t\t\t\t],\n"
			"\t\t\t\t\t\"occurrence\": \"%s\"\n"
			"\t\t\t\t},\n"
			"\t\t\t\t\"properties\": {\n"
			"\t\t\t\t}\n"
			"\t\t\t}%s\n",
			rel->atab, rel->acol,
			rel->type & REL_TYPE_A_ONE ? "one" : "many",
			rel->btab, rel->bcol,
			rel->type & REL_TYPE_B_ONE ? "one" : "many",
			i + 1 < dmd->rellen ? "," : "");
	}
	fprintf(fp, "\t\t],\n"
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
		"\t\t]\n" "\t}\n" "}\n", dmd->lib, dmd->title);
}
