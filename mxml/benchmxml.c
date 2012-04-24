#include "libchash.h"
#if defined(USM)
#include "usm.h"
#endif
#include "private/config.h"
#include "gc.h"
#include "private/gc_priv.h"
#include "mxml.h"
#include <fcntl.h>
#include <stdio.h>
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

char* getFile(char *file)
{
	FILE *f;
	char *buffer;
	long size, got;
	 
	f = fopen(file, "rb");
	if (f == NULL)
		 return NULL;
	 
	fseek(f, 0L, SEEK_END);
	size = ftell(f);
	fseek(f, 0L, SEEK_SET);	
	buffer = (char*) GC_malloc(size * sizeof(char));	
	if (buffer == NULL)
		 return NULL;
	 
	got = fread(buffer, sizeof(char), size, f);

	if (got != size) {
		got = ferror(f);
		fclose(f);
		if (got)
			return NULL;
	} else
		fclose(f);

	return buffer;
}

int runonstring(char *term, HashTable *hash, char *string)
{
   mxml_node_t *tree, *node;
   const char *value;
   HTItem *hvalue;

	tree = mxmlLoadString(NULL, string, NULL);

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
	char *string;

   if (argc < 3) {
      fputs("Usage: testmxml field [filename.xml ...]\n", stderr);
      return (1);
   }

   GC_enable_usm();

   printf("\nGarbage Collector Test\n\n");
   printf("Stressing a xml parser & hash map...\n");

	// load file first, I think read is slow under USM
	if ((string = getFile(argv[2])) == NULL) {
		printf("Error loading file!\n");
		return 1;
	}

   tStart = currentTime();

   hash = AllocateHashTable(0, 1);
   for (i = 2; i < argc; i++) {
      ret = runonstring(argv[1], hash, string);
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

