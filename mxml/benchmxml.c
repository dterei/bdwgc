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

int main(int argc, char *argv[])
{
   FILE *fp;
   mxml_node_t *tree, *node;
   const char *value;
   struct HashTable* hash;
   HTItem *hvalue;
	long tStart, tFinish, size;
#if defined(USM)
	int ret;
#endif

   if (argc != 3) {
      fputs("Usage: testmxml field filename.xml\n", stderr);
      return (1);
   }

#if defined(USM)
   ret = usm_init();
   if (ret) {
      printf("Failed to initialize USM...\n");
      return ret;
   }
   usm_printf("Using USM..\n");
#endif

   GC_enable_incremental();
   printf("Switched to incremental mode\n");

	printf("\nGarbage Collector Test\n\n");
	printf("Stressing a xml parser & hash map...\n");
	tStart = currentTime();

   if (argv[2][0] == '<')
      tree = mxmlLoadString(NULL, argv[1], NULL);
   else if ((fp = fopen(argv[2], "rb")) == NULL) {
      perror(argv[2]);
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

   hash = AllocateHashTable(0, 0);

   for (node = mxmlFindElement(tree, tree, argv[1], NULL, NULL, MXML_DESCEND);
        node != NULL;
        node = mxmlFindElement(node, tree, argv[1], NULL, NULL, MXML_DESCEND)) {
      value = mxmlGetText(node, 0);
      hvalue = HashFindOrInsert(hash, (ulong) value, 0);     /* initialize to 0 */
      if (hvalue != NULL) {
         hvalue->data++;
      }
   }

   for (hvalue = HashFirstBucket(hash); hvalue != NULL;
         hvalue = HashNextBucket(hash)) {
      // printf("node: %s => %ld\n", (char *) hvalue->key, hvalue->data);
   }

	tFinish = currentTime();
	size = GC_get_heap_size();

	printf("Completed in %ld msec\n", tFinish - tStart);
	printf("Completed %ld collections\n", GC_gc_no);
	printf("Heap size is %ld bytes, %ld MB\n", size, size / (1024 * 1024));

   return 0;
}

