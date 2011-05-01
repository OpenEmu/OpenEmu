#ifndef __MDFN_MD_CD_H
#define __MDFN_MD_CD_H

extern c68k_struc Sub68K;

void MDCD_Run(int32 md_master_cycles);
void MDCD_PCM_Run(int32 cycles);
void MDCD_Reset(void);
bool MDCD_Load(md_game_info *);
bool MDCD_TestMagic(void);
void MDCD_Close(void);

#endif
