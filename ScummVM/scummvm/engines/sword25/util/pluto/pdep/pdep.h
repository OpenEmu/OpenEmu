#ifndef PDEP_H
#define PDEP_H

#include "sword25/util/lua/lua.h"
#include "sword25/util/pluto/pdep/lzio.h"
#include "sword25/util/lua/ldo.h"
#include "sword25/util/lua/lfunc.h"
#include "sword25/util/lua/lgc.h"
#include "sword25/util/lua/llimits.h"
#include "sword25/util/lua/lobject.h"
#include "sword25/util/lua/lopcodes.h"
#include "sword25/util/lua/lstate.h"
#include "sword25/util/lua/lstring.h"
#include "sword25/util/lua/lauxlib.h"


#define pdep_reallocv(L,b,on,n,e) \
	pdep_realloc_(L, (b), (on)*(e), (n)*(e))
#define pdep_reallocvector(L, v,oldn,n,t) \
	((v)=cast(t *, pdep_reallocv(L, v, oldn, n, sizeof(t))))
#define pdep_freearray(L, b, n, t)   pdep_reallocv(L, (b), n, 0, sizeof(t))
#define pdep_newvector(L,n,t) \
	cast(t *, pdep_reallocv(L, NULL, 0, n, sizeof(t)))
#define pdep_new(L,t)		cast(t *, pdep_malloc(L, sizeof(t)))
#define pdep_malloc(L,t)	pdep_realloc_(L, NULL, 0, (t))
#define pdep_checkstack(L,n)	\
  if ((char *)L->stack_last - (char *)L->top <= (n)*(int)sizeof(TValue)) \
    pdep_growstack(L, n); \
  else pdep_reallocstack(L, L->stacksize - EXTRA_STACK - 1);


void pdep_pushobject (lua_State *L, const TValue *o);
void *pdep_realloc_ (lua_State *L, void *block, size_t osize, size_t nsize);
void pdep_link (lua_State *L, GCObject *o, lu_byte tt);
Proto *pdep_newproto (lua_State *L);
Closure *pdep_newLclosure (lua_State *L, int nelems, Table *e);
void pdep_reallocstack (lua_State *L, int newsize);
void pdep_growstack (lua_State *L, int n);
void pdep_reallocCI (lua_State *L, int newsize);
TString *pdep_newlstr (lua_State *L, const char *str, size_t l);

#endif
