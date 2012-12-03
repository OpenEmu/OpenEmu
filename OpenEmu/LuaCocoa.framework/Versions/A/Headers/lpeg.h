/*
** $Id: lpeg.h,v 1.1 2009/12/23 16:15:36 roberto Exp $
** LPeg - PEG pattern matching for Lua
** Copyright 2009, Lua.org & PUC-Rio  (see 'lpeg.html' for license)
** written by Roberto Ierusalimschy
*/

#ifndef lpeg_h
#define lpeg_h

#include "lua.h"


#define KEYNEWPATT	"lpeg.newpf"


/*
** type of extension functions that define new "patterns" for LPEG
** It should return the new current position or NULL if match fails
*/
typedef const char *(*PattFunc) (const char *s,  /* current position */
                                 const char *e,  /* string end */
                                 const char *o,  /* string start */
                                 const void *ud);  /* user data */

/*
** function to create new patterns based on 'PattFunc' functions.
** This function is available at *registry[KEYNEWPATT]. (Notice
** the extra indirection; the userdata at the registry points to
** a variable that points to the function. In ANSI C a void* cannot
** point to a function.)
*/
typedef void (*Newpf) (lua_State *L,
                       PattFunc f,  /* pattern */
                       const void *ud,  /* (user) data to be passed to 'f' */
                       size_t l);  /* size of data to be passed to 'f' */

#endif
