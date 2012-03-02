/*
  libco.x86 (2009-10-12)
  author: byuu
  license: public domain
*/

#define LIBCO_C
#include "libco.h"
#include <assert.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_MSC_VER)
  #define fastcall __fastcall
#elif defined(__GNUC__)
  #define fastcall __attribute__((fastcall))
#else
  #error "libco: please define fastcall macro"
#endif

static thread_local long co_active_buffer[64];
static thread_local cothread_t co_active_handle = 0;

// ASM co_swap
void fastcall co_swap(cothread_t new_thread, cothread_t old_thread);

#ifdef _WIN32
#define ASM_CO_SWAP_DEF \
   ".globl @co_swap@8\n" \
   "@co_swap@8:\n"
#else
#define ASM_CO_SWAP_DEF \
   ".globl co_swap\n" \
   ".globl _co_swap\n" \
   "_co_swap:\n" \
   "co_swap:\n"
#endif

// ABI: fastcall
asm (
   ".text\n"
   ASM_CO_SWAP_DEF
   "movl %esp, (%edx) # Save stack pointer, and restore it again.\n"
   "movl (%ecx), %esp\n"
   "popl %eax\n"

   "movl %ebp, 0x4(%edx) # Save non-volatile registers to buffer.\n"
   "movl %esi, 0x8(%edx)\n"
   "movl %edi, 0xc(%edx)\n"
   "movl %ebx, 0x10(%edx)\n"

   "movl 0x4(%ecx),  %ebp # Restore non-volatile registers from buffer.\n"
   "movl 0x8(%ecx),  %esi\n"
   "movl 0xc(%ecx),  %edi\n"
   "movl 0x10(%ecx), %ebx\n"

   "jmp  *%eax # Jump back to saved PC.\n"
);

static void crash() {
  assert(0); /* called only if cothread_t entrypoint returns */
}

cothread_t co_active() {
  if(!co_active_handle) co_active_handle = &co_active_buffer;
  return co_active_handle;
}

cothread_t co_create(unsigned int size, void (*entrypoint)(void)) {
  cothread_t handle;
  if(!co_active_handle) co_active_handle = &co_active_buffer;
  size += 256; /* allocate additional space for storage */
  size &= ~15; /* align stack to 16-byte boundary */

  if(handle = (cothread_t)malloc(size)) {
    long *p = (long*)((char*)handle + size); /* seek to top of stack */
    *--p = (long)crash;                      /* crash if entrypoint returns */
    *--p = (long)entrypoint;                 /* start of function */
    *(long*)handle = (long)p;                /* stack pointer */
  }

  return handle;
}

void co_delete(cothread_t handle) {
  free(handle);
}

void co_switch(cothread_t handle) {
  register cothread_t co_previous_handle = co_active_handle;
  co_swap(co_active_handle = handle, co_previous_handle);
}

#ifdef __cplusplus
}
#endif
