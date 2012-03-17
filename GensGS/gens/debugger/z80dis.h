#ifndef GENS_Z80DIS_H
#define GENS_Z80DIS_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef GENS_DEBUGGER

int z80dis(unsigned char *buf, int *Counter, char str[128]);

#endif /* GENS_DEBUG */

#ifdef __cplusplus
}
#endif /* GENS_DEBUGGER */

#endif /* GENS_Z80DIS_H */
