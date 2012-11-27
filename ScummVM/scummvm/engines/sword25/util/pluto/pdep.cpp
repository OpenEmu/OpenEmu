/* This file is derived from the Lua source code. Please see lua.h for
the copyright statement.
*/

#include "pdep/pdep.h"

#define api_incr_top(L)   {api_check(L, L->top < L->ci->top); L->top++;}

void pdep_pushobject (lua_State *L, const TValue *o) {
	setobj2s(L, L->top, o);
	api_incr_top(L);
}

void *pdep_realloc_ (lua_State *L, void *block, size_t osize, size_t nsize) {
	global_State *g = G(L);
	lua_assert((osize == 0) == (block == NULL));
	block = (*g->frealloc)(g->ud, block, osize, nsize);
	lua_assert((nsize == 0) == (block == NULL));
	g->totalbytes = (g->totalbytes - osize) + nsize;
	return block;
}

void pdep_link (lua_State *L, GCObject *o, lu_byte tt) {
	global_State *g = G(L);
	o->gch.next = g->rootgc;
	g->rootgc = o;
	o->gch.marked = luaC_white(g);
	o->gch.tt = tt;
}

Proto *pdep_newproto (lua_State *L) {
	Proto *f = pdep_new(L, Proto);
	pdep_link(L, obj2gco(f), LUA_TPROTO);
	f->k = NULL;
	f->sizek = 0;
	f->p = NULL;
	f->sizep = 0;
	f->code = NULL;
	f->sizecode = 0;
	f->sizelineinfo = 0;
	f->sizeupvalues = 0;
	f->nups = 0;
	f->upvalues = NULL;
	f->numparams = 0;
	f->is_vararg = 0;
	f->maxstacksize = 0;
	f->lineinfo = NULL;
	f->sizelocvars = 0;
	f->locvars = NULL;
	f->linedefined = 0;
	f->lastlinedefined = 0;
	f->source = NULL;
	return f;
}

Closure *pdep_newLclosure (lua_State *L, int nelems, Table *e) {
	Closure *c = cast(Closure *, pdep_malloc(L, sizeLclosure(nelems)));
	pdep_link(L, obj2gco(c), LUA_TFUNCTION);
	c->l.isC = 0;
	c->l.env = e;
	c->l.nupvalues = cast_byte(nelems);
	while (nelems--) c->l.upvals[nelems] = NULL;
	return c;
}

static void correctstack (lua_State *L, TValue *oldstack) {
  CallInfo *ci;
  GCObject *up;
  L->top = (L->top - oldstack) + L->stack;
  for (up = L->openupval; up != NULL; up = up->gch.next)
    gco2uv(up)->v = (gco2uv(up)->v - oldstack) + L->stack;
  for (ci = L->base_ci; ci <= L->ci; ci++) {
    ci->top = (ci->top - oldstack) + L->stack;
    ci->base = (ci->base - oldstack) + L->stack;
    ci->func = (ci->func - oldstack) + L->stack;
  }
  L->base = (L->base - oldstack) + L->stack;
}


void pdep_reallocstack (lua_State *L, int newsize) {
	TValue *oldstack = L->stack;
	int realsize = newsize + 1 + EXTRA_STACK;
	lua_assert(L->stack_last - L->stack == L->stacksize - EXTRA_STACK - 1);
	pdep_reallocvector(L, L->stack, L->stacksize, realsize, TValue);
	L->stacksize = realsize;
	L->stack_last = L->stack+newsize;
	correctstack(L, oldstack);
}

void pdep_growstack (lua_State *L, int n) {
	if (n <= L->stacksize)  /* double size is enough? */
		pdep_reallocstack(L, 2*L->stacksize);
	else
		pdep_reallocstack(L, L->stacksize + n);
}

void pdep_reallocCI (lua_State *L, int newsize) {
	CallInfo *oldci = L->base_ci;
	pdep_reallocvector(L, L->base_ci, L->size_ci, newsize, CallInfo);
	L->size_ci = newsize;
	L->ci = (L->ci - oldci) + L->base_ci;
	L->end_ci = L->base_ci + L->size_ci - 1;
}

TString *pdep_newlstr (lua_State *L, const char *str, size_t l) {
	TString *res;
	lua_pushlstring(L, str, l);
	res = rawtsvalue(L->top-1);
	lua_pop(L, 1);
	return res;
}
