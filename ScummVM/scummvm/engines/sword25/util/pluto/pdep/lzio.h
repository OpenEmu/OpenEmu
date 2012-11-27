/*
** $Id$
** Buffered streams
** See Copyright Notice in lua.h
*/


#ifndef lzio_h
#define lzio_h

#include "sword25/util/lua/lua.h"


#define EOZ	(-1)			/* end of stream */

typedef struct Zio ZIO;

#define char2int(c)	cast(int, cast(unsigned char, (c)))

#define zgetc(z)  (((z)->n--)>0 ?  char2int(*(z)->p++) : pdep_fill(z))

typedef struct Mbuffer {
  char *buffer;
  size_t n;
  size_t buffsize;
} Mbuffer;

#define pdep_initbuffer(L, buff) ((buff)->buffer = NULL, (buff)->buffsize = 0)

#define pdep_buffer(buff)	((buff)->buffer)
#define pdep_sizebuffer(buff)	((buff)->buffsize)
#define pdep_bufflen(buff)	((buff)->n)

#define pdep_resetbuffer(buff) ((buff)->n = 0)


#define pdep_resizebuffer(L, buff, size) \
	(pdep_reallocvector(L, (buff)->buffer, (buff)->buffsize, size, char), \
	(buff)->buffsize = size)

#define pdep_freebuffer(L, buff)	pdep_resizebuffer(L, buff, 0)


LUAI_FUNC char *pdep_openspace (lua_State *L, Mbuffer *buff, size_t n);
LUAI_FUNC void pdep_init (lua_State *L, ZIO *z, lua_Reader reader,
                                        void *data);
LUAI_FUNC size_t pdep_read (ZIO* z, void* b, size_t n);	/* read next n bytes */
LUAI_FUNC int pdep_lookahead (ZIO *z);



/* --------- Private Part ------------------ */

struct Zio {
  size_t n;			/* bytes still unread */
  const char *p;		/* current position in buffer */
  lua_Reader reader;
  void* data;			/* additional data */
  lua_State *L;			/* Lua state (for reader) */
};


LUAI_FUNC int pdep_fill (ZIO *z);

#endif
