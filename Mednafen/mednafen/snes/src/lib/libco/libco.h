/*
  libco
  version: 0.15 (2009-10-12)
  license: public domain
*/

#ifndef LIBCO_H
#define LIBCO_H

#ifdef LIBCO_C
  #ifdef LIBCO_MP
    #define thread_local __thread
  #else
    #define thread_local
  #endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef void* cothread_t;

cothread_t co_active();
cothread_t co_create(unsigned int, void (*)(void));
void co_delete(cothread_t);
void co_switch(cothread_t);

#ifdef __cplusplus
}
#endif

/* ifndef LIBCO_H */
#endif
