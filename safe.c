#include <stdlib.h>
#include <string.h>
#include <err.h>

/* Safe wrappers around functions that might return NULL */

void *smalloc(size_t size)
{
	void *res;
	if (!(res = malloc(size)))
		err(EXIT_FAILURE, "malloc(%zd)", size);
	return res;
}

void *srealloc(void *ptr, size_t size)
{
	void *res;
	if (!(res = realloc(ptr, size)))
		err(EXIT_FAILURE, "realloc(%zd)", size);
	return res;
}

char *sstrdup(const char *str)
{
	char *res;
	if (!(res = strdup(str)))
		err(EXIT_FAILURE, "strdup()");
	return res;
}
