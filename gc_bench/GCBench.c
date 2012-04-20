// This is adapted from a benchmark written by John Ellis and Pete Kovac
// of Post Communications.
// It was modified by Hans Boehm of Silicon Graphics.
// Translated to C++ 30 May 1997 by William D Clinger of Northeastern Univ.
// Translated to C 15 March 2000 by Hans Boehm, now at HP Labs.
//
//      This is no substitute for real applications.  No actual application
//      is likely to behave in exactly this way.  However, this benchmark was
//      designed to be more representative of real applications than other
//      Java GC benchmarks of which we are aware.
//      It attempts to model those properties of allocation requests that
//      are important to current GC techniques.
//      It is designed to be used either to obtain a single overall performance
//      number, or to give a more detailed estimate of how collector
//      performance varies with object lifetimes.  It prints the time
//      required to allocate and collect balanced binary trees of various
//      sizes.  Smaller trees result in shorter object lifetimes.  Each cycle
//      allocates roughly the same amount of memory.
//      Two data structures are kept around during the entire process, so
//      that the measured performance is representative of applications
//      that maintain some live in-memory data.  One of these is a tree
//      containing many pointers.  The other is a large array containing
//      double precision floating point numbers.  Both should be of comparable
//      size.
//
//      The results are only really meaningful together with a specification
//      of how much memory was used.  It is possible to trade memory for
//      better time performance.  This benchmark should be run in a 32 MB
//      heap, though we don't currently know how to enforce that uniformly.
//
//      Unlike the original Ellis and Kovac benchmark, we do not attempt
//      measure pause times.  This facility should eventually be added back
//      in.  There are several reasons for omitting it for now.  The original
//      implementation depended on assumptions about the thread scheduler
//      that don't hold uniformly.  The results really measure both the
//      scheduler and GC.  Pause time measurements tend to not fit well with
//      current benchmark suites.  As far as we know, none of the current
//      commercial Java implementations seriously attempt to minimize GC pause
//      times.

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
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

#include "LL.c"

#define HOLES

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

static const int kStretchTreeDepth   = 18; // about 16Mb (18)
static const int kLongLivedTreeDepth = 16; // about 4Mb (16)
static const int kArraySize = 500000;     // about 4Mb (500000)
static const int kMinTreeDepth = 4;        // (4)
static const int kMaxTreeDepth = 16;       // (16)

typedef struct Node0_struct
{
   struct Node0_struct *left;
   struct Node0_struct *right;
   int i, j;
} Node0;

typedef Node0 *Node;

#ifdef HOLES
#   define HOLE() GC_NEW(Node0);
#else
#   define HOLE()
#endif

void init_Node(Node me, Node l, Node r)
{
    me -> left = l;
    me -> right = r;
}

// Nodes used by a tree of a given size
static int TreeSize(int i)
{
   return ((1 << (i + 1)) - 1);
}

// Number of iterations to use for a given tree depth
static int NumIters(int i)
{
   return 2 * TreeSize(kStretchTreeDepth) / TreeSize(i);
}

// Build tree top down, assigning to older objects.
static void Populate(int iDepth, Node thisNode)
{
   if (iDepth <= 0) {
      return;
   } else {
      iDepth--;
      thisNode->left  = GC_NEW(Node0); HOLE();
      thisNode->right = GC_NEW(Node0); HOLE();
      Populate(iDepth, thisNode->left);
      Populate(iDepth, thisNode->right);
   }
}

// Build tree bottom-up
static Node MakeTree(int iDepth)
{
   Node result;
   if (iDepth <= 0) {
      result = GC_NEW(Node0); HOLE();
      /* result is implicitly initialized in both cases. */
      return result;
   } else {
      Node left = MakeTree(iDepth-1);
      Node right = MakeTree(iDepth-1);
      result = GC_NEW(Node0); HOLE();
      init_Node(result, left, right);
      return result;
   }
}

static void TimeConstruction(int depth)
{
   Node tempTree;
   long tStart, tFinish;
   int iNumIters = NumIters(depth);
   int i;

   printf("Creating %d trees of depth %d\n", iNumIters, depth);
        
   tStart = currentTime();
   for (i = 0; i < iNumIters; ++i) {
      tempTree = GC_NEW(Node0);
      Populate(depth, tempTree);
      tempTree = 0;
   }
   tFinish = currentTime();
   printf("\tTop down construction took %ld msec\n", tFinish - tStart);
             
   tStart = currentTime();
   for (i = 0; i < iNumIters; ++i) {
      tempTree = MakeTree(depth);
      tempTree = 0;
   }
   tFinish = currentTime();
   printf("\tBottom up construction took %ld msec\n", tFinish - tStart);
}

void init_gcbench(void)
{
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
}

int linkedListTest(void)
{
   long tStart, tFinish, size;

   printf("Creating and folding many linked lists...\n");

   tStart = currentTime();
   runLL();
   tFinish = currentTime();
   size = GC_get_heap_size();

   printf("Completed in %ld msec\n", tFinish - tStart);
   printf("Completed %ld collections\n", GC_gc_no);
   printf("Heap size is %ld bytes, %ld MB\n", size, size / (1024 * 1024));

   return 0;
}

int binaryTreeTest(void)
{
   Node root, longLivedTree, tempTree;
   long tStart, tFinish, size;
   int i, d;
   double *array;

   printf("Creating a big ass binary tree...\n");
   
   printf(" Live storage will peak at %ld bytes.\n\n",
      2 * sizeof(Node0) * TreeSize(kLongLivedTreeDepth) +
      sizeof(double) * kArraySize);
   printf(" Stretching memory with a binary tree of depth %d\n",
      kStretchTreeDepth);

   tStart = currentTime();
        
   // Stretch the memory space quickly
   tempTree = MakeTree(kStretchTreeDepth);
   tempTree = 0;

   // Create a long lived object
   printf(" Creating a long-lived binary tree of depth %d\n",
      kLongLivedTreeDepth);
   longLivedTree = GC_NEW(Node0);
   Populate(kLongLivedTreeDepth, longLivedTree);

   // Create long-lived array, filling half of it
   printf(" Creating a long-lived array of %d doubles\n", kArraySize);
#ifndef NO_PTRFREE
   array = GC_MALLOC_ATOMIC(sizeof(double) * kArraySize);
#else
   array = GC_MALLOC(sizeof(double) * kArraySize);
#endif
   for (i = 0; i < kArraySize/2; ++i) {
      array[i] = 1.0/i;
   }

   for (d = kMinTreeDepth; d <= kMaxTreeDepth; d += 2) {
      TimeConstruction(d);
   }

   // fake reference to LongLivedTree and array to keep them from being
   // optimized away
   if (longLivedTree == 0 || array[1000] != 1.0/1000)
      fprintf(stderr, "Failed\n");

   tFinish = currentTime();
   size = GC_get_heap_size();

   size = GC_get_total_bytes();
   printf("Total allocated memory is %ld bytes, %ld MB\n", size, size / (1024 * 1024));
   size = GC_get_heap_size();
   printf("Current heap size is %ld bytes, %ld MB\n", size, size / (1024 * 1024));
   size = GC_get_free_bytes();
   printf("Free heap is %ld bytes, %ld MB\n", size, size / (1024 * 1024));

   return 0;
}

int main(int argc, char *argv[])
{
   int r;
   init_gcbench();
   printf("\nGarbage Collector Test\n\n");

   if (argc > 1) {
      r = linkedListTest();
   } else {
      r = binaryTreeTest();
   }

   return r;
}

