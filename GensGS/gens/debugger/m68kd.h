#ifndef GENS_M68KD_H
#define GENS_M68KD_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef GENS_DEBUGGER

char *M68KDisasm(unsigned short (*NW)(), unsigned int (*NL)());

#endif /* GENS_DEBUGGER */

#ifdef __cplusplus
}
#endif

#endif /* GENS_M68KD_H */
