#ifndef MEMORY
#define MEMORY

#include <stddef.h>

void* malloc(size_t size);

void* calloc(size_t count, size_t size);

void* realloc(void* ptr, size_t size);

void free();

#endif
