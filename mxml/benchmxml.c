#include "libchash.h"
#if defined(USM)
#include "usm.h"
#endif
#include "gc.h"
#include "mxml.h"
#include <fcntl.h>
#ifndef O_BINARY
#  define O_BINARY 0
#endif

#if !defined(USM)
static inline unsigned long usm_get_ticks(void)
{
	unsigned int a, d;
	asm volatile("rdtsc" : "=a" (a), "=d" (d));
	return ((unsigned long) a) | (((unsigned long) d) << 32);
}
#endif

/* Get the current time in milliseconds */
unsigned long currentTime(void)
{
   #define MILISECONDS 1000
   #define CYCLES 2401000000ul

   unsigned long t;

   t = usm_get_ticks();
   t *= MILISECONDS;
   t /= CYCLES;

   return t;
}

int runonfile(char *term, HashTable *hash, char *file)
{
   FILE *fp;
   mxml_node_t *tree, *node;
   const char *value;
   HTItem *hvalue;

   if ((fp = fopen(file, "rb")) == NULL) {
      perror(file);
      return (1);
   } else {
      // read the file
      tree = mxmlLoadFile(NULL, fp, NULL);
      fclose(fp);
   }

   if (!tree) {
      fputs("Unable to read XML file!\n", stderr);
      return (1);
   }

   for (node = mxmlFindElement(tree, tree, term, NULL, NULL, MXML_DESCEND);
        node != NULL;
        node = mxmlFindElement(node, tree, term, NULL, NULL, MXML_DESCEND)) {
      value = mxmlGetText(node, 0);
      hvalue = HashFindOrInsert(hash, (ulong) value, 0);     /* initialize to 0 */
      if (hvalue != NULL) {
         hvalue->data++;
      }
   }

   /* for (hvalue = HashFirstBucket(hash); hvalue != NULL; */
   /*       hvalue = HashNextBucket(hash)) { */
   /*    printf("node: %s => %ld\n", (char *) hvalue->key, hvalue->data); */
   /* } */

	mxmlDelete(tree);
	/* GC_free(tree); */
	tree = NULL;
	GC_gcollect();
	return 0;
}

int main(int argc, char *argv[])
{
   struct HashTable* hash;
	long tStart, tFinish, size;
	int i, ret;

   if (argc < 3) {
      fputs("Usage: testmxml field [filename.xml ...]\n", stderr);
      return (1);
   }

#if defined(USM)
   ret = usm_init();
   if (ret) {
      printf("Failed to initialize USM...\n");
      return ret;
   }
   usm_printf("Using USM..\n");
#else
   printf("Not using USM..\n");
#endif

   GC_enable_incremental();
   printf("Switched to incremental mode\n");

	printf("\nGarbage Collector Test\n\n");
	printf("Stressing a xml parser & hash map...\n");
	tStart = currentTime();

   hash = AllocateHashTable(0, 1);
	for (i = 2; i < argc; i++) {
		ret = runonfile(argv[1], hash, argv[i]);
		if (ret) {
			return ret;
		}
	}

	tFinish = currentTime();

	printf("Completed in %ld msec\n", tFinish - tStart);
	printf("Completed %ld collections\n", GC_gc_no);

	size = GC_get_total_bytes();
	printf("Total allocated memory is %ld bytes, %ld MB\n", size, size / (1024 * 1024));
	size = GC_get_heap_size();
	printf("Current heap size is %ld bytes, %ld MB\n", size, size / (1024 * 1024));
	size = GC_get_free_bytes();
	printf("Free heap is %ld bytes, %ld MB\n", size, size / (1024 * 1024));

   return 0;
}

