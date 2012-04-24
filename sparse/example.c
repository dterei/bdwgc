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
  GC_enable_usm();

  long tStart, tFinish, size;

  printf("\nGarbage Collector Test\n\n");
  printf("Stressing a hash map...\n");
  tStart = currentTime();

  TestInsert();
  TestFindOrInsert();

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

