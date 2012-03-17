/**
 * Gens: Sega CD (Mega CD) initialization and main loop code.
 */

#ifndef GENS_MCD_HPP
#define GENS_MCD_HPP

#ifdef __cplusplus
extern "C" {
#endif

#include "util/file/rom.hpp"

const char* Detect_Country_SegaCD(void);

int Init_SegaCD(const char* iso_name);
int Reload_SegaCD(const char* iso_name);
void Reset_SegaCD(void);
int Do_SegaCD_Frame_No_VDP(void);
int Do_SegaCD_Frame(void);
int Do_SegaCD_Frame_Cycle_Accurate(void);
int Do_SegaCD_Frame_No_VDP_Cycle_Accurate(void);

#ifdef __cplusplus
}
#endif

#endif /* GENS_MCD_HPP */
