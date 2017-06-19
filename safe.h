#ifndef _H_SAFE_
#define _H_SAFE_ 1
void *smalloc(size_t size);
void *srealloc(void *ptr, size_t size);
char *sstrdup(const char *str);
#endif

