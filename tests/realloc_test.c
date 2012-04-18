
#include <stdio.h>
#include <stdlib.h>
#include "gc.h"
#include "usm.h"

#define COUNT 10000000

int main(int argc, char *argv[]) {
  int i;
  unsigned long last_heap_size = 0;
	 int ret;

	if (argc > 1) {
		printf("Not using USM...\n");
	} else {
		ret = usm_init();
		if (ret) {
			printf("Failed to initialize USM...\n");
			return ret;
		}
		usm_printf("Using USM..\n");
	}

  GC_INIT();

  for (i = 0; i < COUNT; i++) {
    int **p = GC_MALLOC(sizeof(int *));
    int *q = GC_MALLOC_ATOMIC(sizeof(int));

    if (p == 0 || *p != 0) {
      fprintf(stderr, "GC_malloc returned garbage (or NULL)\n");
      exit(1);
    }

    *p = GC_REALLOC(q, 2 * sizeof(int));

    if (i % 10 == 0) {
      unsigned long heap_size = (unsigned long)GC_get_heap_size();
      if (heap_size != last_heap_size) {
        printf("Heap size: %lu\n", heap_size);
        last_heap_size = heap_size;
      }
    }
  }
  return 0;
}
