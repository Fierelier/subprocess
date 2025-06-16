#include <stdlib.h>
#include <stdio.h>

// To use this, use -Wl,--wrap=malloc,--wrap=free

size_t allocated = 0;
int inited = 0;

void mallocprint(void) {
	printf("Objects malloc'd = %zu\n", allocated);
}

void* __real_malloc(size_t size);

void __real_free(void * ptr);

void __wrap_free(void * ptr) {
	allocated -= 1;
	__real_free(ptr);
}

void* __wrap_malloc(size_t size) {
	if (!inited) {
		inited = 1;
		atexit(mallocprint);
	}
    allocated += 1;
    return __real_malloc(size);
}

