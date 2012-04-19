#include "libchash.h"
#include "usm.h"
#include "gc.h"
#include "mxml.h"
#include <fcntl.h>
#ifndef O_BINARY
#  define O_BINARY 0
#endif

int main(int argc, char *argv[])
{
   FILE *fp;
   mxml_node_t *tree, *node;
   const char *value;
   struct HashTable* hash;
   HTItem *hvalue;

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
      printf("node: %s => %ld\n", (char *) hvalue->key, hvalue->data);
   }

   return 0;
}

