/**
 * Gens: Sega 32X initialization and main loop code.
 */

#ifndef GENS_32X_HPP
#define GENS_32X_HPP

#ifdef __cplusplus
extern "C" {
#endif

#include "util/file/rom.hpp"

int Init_32X(ROM_t* MD_ROM);
void Reset_32X();
int Do_32X_VDP_Only(void);
int Do_32X_Frame_No_VDP(void);
int Do_32X_Frame(void);

#ifdef __cplusplus
}
#endif

#endif /* GENS_32X_HPP */
