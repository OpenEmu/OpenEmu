// C99, build as shared library and LD_PRELOAD ;)

#include <stdlib.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdbool.h>

#define RTLD_NEXT ((void *) -1l)

static void *(*real_malloc)(size_t);
static void (*real_free)(void *);

static long malloc_count = 0;
static long free_count = 0;

static bool block = false;

static bool lib_block = true;

void libmalloc_start(void)
{
   lib_block = false;
}

void libmalloc_stop(void)
{
   lib_block = true;
}

void *malloc(size_t size)
{
   if (!real_malloc)
      real_malloc = dlsym(RTLD_NEXT, "malloc");

   void *res = real_malloc(size);
   if (block || lib_block)
      return res;

   if (size && res)
      malloc_count++;

   block = true;
   printf("malloc =>\t%p (size = %u)\n", res, (unsigned)size);
   printf("Delta = %ld\n", malloc_count - free_count);
   block = false;

   return res;
}

void free(void *ptr)
{
   if (!real_free)
      real_free = dlsym(RTLD_NEXT, "free");

   if (block || lib_block)
   {
      real_free(ptr);
      return;
   }

   if (ptr)
      free_count++;

   block = true;
   printf("free   =>\t%p\n", ptr);
   printf("Delta = %ld\n", malloc_count - free_count);

   real_free(ptr);
   block = false;
}

