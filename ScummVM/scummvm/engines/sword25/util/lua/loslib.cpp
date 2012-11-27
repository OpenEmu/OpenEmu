/*
** $Id$
** Standard Operating System library
** See Copyright Notice in lua.h
*/

// FIXME: Get rid of all time.h stuff
#define FORBIDDEN_SYMBOL_EXCEPTION_time_h

#include <time.h>

#define loslib_c
#define LUA_LIB

#include "lua.h"

#include "lauxlib.h"
#include "lualib.h"

#include "common/system.h"
#include "common/textconsole.h"


static int os_execute (lua_State *L) {
  // Non-portable call, removed in ScummVM.
  // FIXME: Is this ever invoked? If so, investigate that code further.
  lua_pushinteger(L, -1);	// signal that an error occurred
  return 1;
}


static int os_remove (lua_State *L) {
  // Non-portable call that deletes a file. Removed in ScummVM.
  // This call is invoked in sword25 when loading games in order to remove the
  // temporary savegame thumbnail that the original engine code created. We
  // embed the thumbnail in the savegame instead, so this call is not needed at
  // all.
  return 1;
}


static int os_rename (lua_State *L) {
  // Non-portable call, removed in ScummVM.
  return 1;
}


static int os_tmpname (lua_State *L) {
  // Non-portable call, removed in ScummVM.
  // FIXME: Why do we return an error in tmpname, but for other
  // removed methods we just do nothing?
  return luaL_error(L, "unable to generate a unique filename");
}


static int os_getenv (lua_State *L) {
  // Non-portable call, removed in ScummVM.
  // FIXME: Is this ever invoked? If so, investigate that code further.
  lua_pushstring(L, NULL);
  return 1;
}


static int os_clock (lua_State *L) {
  // Non-portable call to clock() replaced by invocation of OSystem::getMillis.
  lua_pushnumber(L, ((lua_Number)g_system->getMillis())/(lua_Number)1000);
  return 1;
}


/*
** {======================================================
** Time/Date operations
** { year=%Y, month=%m, day=%d, hour=%H, min=%M, sec=%S,
**   wday=%w+1, yday=%j, isdst=? }
** =======================================================
*/

static void setfield (lua_State *L, const char *key, int value) {
  lua_pushinteger(L, value);
  lua_setfield(L, -2, key);
}

static void setboolfield (lua_State *L, const char *key, int value) {
  if (value < 0)  /* undefined? */
    return;  /* does not set field */
  lua_pushboolean(L, value);
  lua_setfield(L, -2, key);
}

static int getboolfield (lua_State *L, const char *key) {
  int res;
  lua_getfield(L, -1, key);
  res = lua_isnil(L, -1) ? -1 : lua_toboolean(L, -1);
  lua_pop(L, 1);
  return res;
}


static int getfield (lua_State *L, const char *key, int d) {
  int res;
  lua_getfield(L, -1, key);
  if (lua_isnumber(L, -1))
    res = (int)lua_tointeger(L, -1);
  else {
    if (d < 0)
      return luaL_error(L, "field " LUA_QS " missing in date table", key);
    res = d;
  }
  lua_pop(L, 1);
  return res;
}


static int os_date (lua_State *L) {
  const char *s = luaL_optstring(L, 1, "%c");
  // FIXME: Rewrite the code below to use OSystem::getTimeAndDate
  // Alternatively, remove it, if sword25 does not use it.
  //
  // The former would mean sacrificing the ability to choose the timezone, *or*
  // we would have to drive an effort to add time zone support to OSystem (is it
  // worth that, though???)
  time_t t = luaL_opt(L, (time_t)luaL_checknumber, 2, time(NULL));
  struct tm *stm;
  if (*s == '!') {  /* UTC? */
    stm = gmtime(&t);
    s++;  /* skip `!' */
  }
  else
    stm = localtime(&t);
  if (stm == NULL)  /* invalid date? */
    lua_pushnil(L);
  else if (strcmp(s, "*t") == 0) {
    lua_createtable(L, 0, 9);  /* 9 = number of fields */
    setfield(L, "sec", stm->tm_sec);
    setfield(L, "min", stm->tm_min);
    setfield(L, "hour", stm->tm_hour);
    setfield(L, "day", stm->tm_mday);
    setfield(L, "month", stm->tm_mon+1);
    setfield(L, "year", stm->tm_year+1900);
    setfield(L, "wday", stm->tm_wday+1);
    setfield(L, "yday", stm->tm_yday+1);
    setboolfield(L, "isdst", stm->tm_isdst);
  }
  else {
    char cc[3];
    luaL_Buffer b;
    cc[0] = '%'; cc[2] = '\0';
    luaL_buffinit(L, &b);
    for (; *s; s++) {
      if (*s != '%' || *(s + 1) == '\0')  /* no conversion specifier? */
        luaL_addchar(&b, *s);
      else {
        size_t reslen;
        char buff[200];  /* should be big enough for any conversion result */
        cc[1] = *(++s);
        reslen = strftime(buff, sizeof(buff), cc, stm);
        luaL_addlstring(&b, buff, reslen);
      }
    }
    luaL_pushresult(&b);
  }
  return 1;
}


static int os_time (lua_State *L) {
  // FIXME: Rewrite the code below to use OSystem::getTimeAndDate.
  // Alternatively, remove it, if sword25 does not use it.
  time_t t;
  if (lua_isnoneornil(L, 1))  /* called without args? */
    t = time(NULL);  /* get current time */
  else {
    struct tm ts;
    luaL_checktype(L, 1, LUA_TTABLE);
    lua_settop(L, 1);  /* make sure table is at the top */
    ts.tm_sec = getfield(L, "sec", 0);
    ts.tm_min = getfield(L, "min", 0);
    ts.tm_hour = getfield(L, "hour", 12);
    ts.tm_mday = getfield(L, "day", -1);
    ts.tm_mon = getfield(L, "month", -1) - 1;
    ts.tm_year = getfield(L, "year", -1) - 1900;
    ts.tm_isdst = getboolfield(L, "isdst");
    t = mktime(&ts);
  }
  if (t == (time_t)(-1))
    lua_pushnil(L);
  else
    lua_pushnumber(L, (lua_Number)t);
  return 1;
}


static int os_difftime (lua_State *L) {
  // FIXME: difftime is not portable, unfortunately.
  // So we either have to replace this code, or just remove it,
  // depending on whether sword25 actually uses it.
  lua_pushnumber(L, difftime((time_t)(luaL_checknumber(L, 1)),
                             (time_t)(luaL_optnumber(L, 2, 0))));
  return 1;
}

/* }====================================================== */


static int os_setlocale (lua_State *L) {
  // Non-portable call to set the numeric locale. Removed in ScummVM, as it's
  // not used in sword25.
  return 1;
}


static int os_exit (lua_State *L) {
  // FIXME: Using exit is not portable!
  // Using OSystem::quit() isn't really a great idea, either.
  // We really would prefer to let the main run loop exit, so that
  // our main() can perform cleanup.
  if (0 == luaL_optint(L, 1, EXIT_SUCCESS))
	  g_system->quit();
  error("LUA os_exit invokes with non-zero exit value");
}

static const luaL_Reg syslib[] = {
  {"clock",     os_clock},
  {"date",      os_date},
  {"difftime",  os_difftime},
  {"execute",   os_execute},
  {"exit",      os_exit},
  {"getenv",    os_getenv},
  {"remove",    os_remove},
  {"rename",    os_rename},
  {"setlocale", os_setlocale},
  {"time",      os_time},
  {"tmpname",   os_tmpname},
  {NULL, NULL}
};

/* }====================================================== */



LUALIB_API int luaopen_os (lua_State *L) {
  luaL_register(L, LUA_OSLIBNAME, syslib);
  return 1;
}
