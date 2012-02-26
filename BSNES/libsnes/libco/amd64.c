/*
  libco.amd64 (2009-10-12)
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

// ASM co_swap.
void co_swap(cothread_t new_active, cothread_t old_active);

static thread_local long long co_active_buffer[64];
static thread_local cothread_t co_active_handle = 0;

#ifdef _WIN32
asm (
      ".text\n"
      ".globl co_swap\n"
      "co_swap:\n"

      "movq %rsp, (%rdx) # Save stack pointer and pop old back\n"
      "movq (%rcx), %rsp\n"

      "popq %rax # Pop saved PC to %rax\n"

      "movq %rbp, 0x8(%rdx) # Save non-volatile registers\n"
      "movq %rsi, 0x10(%rdx)\n"
      "movq %rdi, 0x18(%rdx)\n"
      "movq %rbx, 0x20(%rdx)\n"
      "movq %r12, 0x28(%rdx)\n"
      "movq %r13, 0x30(%rdx)\n"
      "movq %r14, 0x38(%rdx)\n"
      "movq %r15, 0x40(%rdx)\n"

      "addq $0x80, %rdx\n"
      "andq $0xfffffffffffffff0, %rdx # Align buffer to 16 bytes (SSE).\n"
      "movaps %xmm6, (%rdx) # Also save non-volatile vector registers.\n"
      "movaps %xmm7, 0x10(%rdx)\n"
      "movaps %xmm8, 0x20(%rdx)\n"
      "movaps %xmm9, 0x30(%rdx)\n"
      "movaps %xmm10, 0x40(%rdx)\n"
      "movaps %xmm11, 0x50(%rdx)\n"
      "movaps %xmm12, 0x60(%rdx)\n"
      "movaps %xmm13, 0x70(%rdx)\n"
      "movaps %xmm14, 0x80(%rdx)\n"
      "movaps %xmm15, 0x90(%rdx)\n"

      "movq 0x8(%rcx),  %rbp # Pop back our saved registers and vector registers.\n"
      "movq 0x10(%rcx), %rsi\n"
      "movq 0x18(%rcx), %rdi\n"
      "movq 0x20(%rcx), %rbx\n"
      "movq 0x28(%rcx), %r12\n"
      "movq 0x30(%rcx), %r13\n"
      "movq 0x38(%rcx), %r14\n"
      "movq 0x40(%rcx), %r15\n"

      "addq $0x80, %rcx\n"
      "andq $0xfffffffffffffff0, %rcx\n"
      "movaps %xmm6, (%rcx)\n"
      "movaps %xmm7, 0x10(%rcx)\n"
      "movaps %xmm8, 0x20(%rcx)\n"
      "movaps %xmm9, 0x30(%rcx)\n"
      "movaps %xmm10, 0x40(%rcx)\n"
      "movaps %xmm11, 0x50(%rcx)\n"
      "movaps %xmm12, 0x60(%rcx)\n"
      "movaps %xmm13, 0x70(%rcx)\n"
      "movaps %xmm14, 0x80(%rcx)\n"
      "movaps %xmm15, 0x90(%rcx)\n"
      "jmpq *%rax # Jump back to old saved PC.\n"
);
#else
asm (
      ".text\n"
      ".globl co_swap\n"
      ".globl _co_swap\n"
      "_co_swap:\n"
      "co_swap:\n"
      "movq %rsp, (%rsi) # Save stack pointer, and pop the old one back.\n"
      "movq (%rdi), %rsp\n"
      "popq %rax\n"

      "movq %rbp, 0x8(%rsi) # Save our non-volatile registers to buffer.\n"
      "movq %rbx, 0x10(%rsi)\n"
      "movq %r12, 0x18(%rsi)\n"
      "movq %r13, 0x20(%rsi)\n"
      "movq %r14, 0x28(%rsi)\n"
      "movq %r15, 0x30(%rsi)\n"

      "movq 0x8(%rdi),  %rbp # Pop back our saved registers.\n"
      "movq 0x10(%rdi), %rbx\n"
      "movq 0x18(%rdi), %r12\n"
      "movq 0x20(%rdi), %r13\n"
      "movq 0x28(%rdi), %r14\n"
      "movq 0x30(%rdi), %r15\n"
      "jmpq *%rax # Jump back to saved PC.\n"
);
#endif

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
  size += 512; /* allocate additional space for storage */
  size &= ~15; /* align stack to 16-byte boundary */

  if((handle = (cothread_t)malloc(size))) {
    long long *p = (long long*)((char*)handle + size); /* seek to top of stack */
    *--p = (long long)crash;                           /* crash if entrypoint returns */
    *--p = (long long)entrypoint;                      /* start of function */
    *(long long*)handle = (long long)p;                /* stack pointer */
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
