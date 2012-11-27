/*
** $Id$
** a generic input stream interface
** See Copyright Notice in lua.h
*/


#include <string.h>

#define lzio_c
#define LUA_CORE

#include "pdep/pdep.h"

int pdep_fill (ZIO *z) {
  size_t size;
  lua_State *L = z->L;
  const char *buff;
  lua_unlock(L);
  buff = z->reader(L, z->data, &size);
  lua_lock(L);
  if (buff == NULL || size == 0) return EOZ;
  z->n = size - 1;
  z->p = buff;
  return char2int(*(z->p++));
}


int pdep_lookahead (ZIO *z) {
  if (z->n == 0) {
    if (pdep_fill(z) == EOZ)
      return EOZ;
    else {
      z->n++;  /* pdep_fill removed first byte; put back it */
      z->p--;
    }
  }
  return char2int(*z->p);
}


void pdep_init (lua_State *L, ZIO *z, lua_Reader reader, void *data) {
  z->L = L;
  z->reader = reader;
  z->data = data;
  z->n = 0;
  z->p = NULL;
}


/* --------------------------------------------------------------- read --- */
size_t pdep_read (ZIO *z, void *b, size_t n) {
  while (n) {
    size_t m;
    if (pdep_lookahead(z) == EOZ)
      return n;  /* return number of missing bytes */
    m = (n <= z->n) ? n : z->n;  /* min. between n and z->n */
    memcpy(b, z->p, m);
    z->n -= m;
    z->p += m;
    b = (char *)b + m;
    n -= m;
  }
  return 0;
}

/* ------------------------------------------------------------------------ */
char *pdep_openspace (lua_State *L, Mbuffer *buff, size_t n) {
  if (n > buff->buffsize) {
    if (n < LUA_MINBUFFER) n = LUA_MINBUFFER;
    pdep_resizebuffer(L, buff, n);
  }
  return buff->buffer;
}
