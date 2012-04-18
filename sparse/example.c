#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "libchash.h"

#if defined(USM)
#  include "usm.h"
#endif
#ifdef GC
#  include "gc.h"
#endif
#ifdef HAVE_CONFIG_H
#  include "private/config.h"
#  include "private/gc_priv.h"
#endif

static void TestInsert() {
  struct HashTable* ht;
  HTItem* bck;

  ht = AllocateHashTable(1, 0);    /* value is 1 byte, 0: don't copy keys */

  HashInsert(ht, PTR_KEY(ht, "January"), 31);  /* 0: don't overwrite old val */
  bck = HashInsert(ht, PTR_KEY(ht, "February"), 28);
  bck = HashInsert(ht, PTR_KEY(ht, "March"), 31);

  bck = HashFind(ht, PTR_KEY(ht, "February"));
  assert(bck);
  assert(bck->data == 28);

  FreeHashTable(ht);
}

static void TestFindOrInsert() {
  struct HashTable* ht;
  int i;
  int iterations = 3000000;
  int range = 30000;         /* random number between 1 and 30 */

  ht = AllocateHashTable(4, 0);    /* value is 4 bytes, 0: don't copy keys */

  /* We'll test how good rand() is as a random number generator */
  for (i = 0; i < iterations; ++i) {
    int key = rand() % range;
    HTItem* bck = HashFindOrInsert(ht, key, 0);     /* initialize to 0 */
	 if (i % 10) {
		bck->data = GC_MALLOC(sizeof(int) * 1000);
	 } else if (i % 8) {
		bck->data = 0;
	 } else {
	    int* p = bck->data;
       if (p != 0) {
		   (*((int*) bck->data))++;                   /* found one more of them */
		 }
	 }
  }

  for (i = 0; i < range; ++i) {
    HTItem* bck = HashFind(ht, i);
    /* if (bck) { */
    /*   printf("%3d: %d\n", bck->key, bck->data); */
    /* } else { */
    /*   printf("%3d: 0\n", i); */
    /* } */
  }

  FreeHashTable(ht);
}

int main(int argc, char** argv) {
#if defined(USM)
   int   ret;

   ret = usm_init();
   if (ret) {
      printf("Failed to initialize USM...\n");
      return ret;
   }
   usm_printf("Using USM..\n");
#else
   printf("Not using USM...\n");
#endif
   GC_enable_incremental();
   GC_printf("Switched to incremental mode\n");
#  if defined(MPROTECT_VDB)
      GC_printf("Emulating dirty bits with mprotect/signals\n");
#  else
#     ifdef PROC_VDB
         GC_printf("Reading dirty bits from /proc\n");
#     elif defined(GWW_VDB)
         GC_printf("Using GetWriteWatch-based implementation\n");
#     else
         GC_printf("Using DEFAULT_VDB dirty bit implementation\n");
#     endif
#  endif

  TestInsert();
  TestFindOrInsert();
  return 0;
}
