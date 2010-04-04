#ifndef _GENERAL_H
#define _GENERAL_H

#include <string>

void GetFileBase(const char *f);
extern uint32 uppow2(uint32 n);

std::string MDFN_MakeFName(int type, int id1, const char *cd1);
char *MDFN_FixString(char *str);

void MDFN_ltrim(char *string);
void MDFN_rtrim(char *string);
void MDFN_trim(char *string);

typedef enum
{
 MDFNMKF_STATE = 0,
 MDFNMKF_SNAP,
 MDFNMKF_SAV,
 MDFNMKF_CHEAT,
 MDFNMKF_FDSROM,
 MDFNMKF_PALETTE,
 MDFNMKF_GGROM,
 MDFNMKF_IPS,
 MDFNMKF_MOVIE,
 MDFNMKF_AUX,
 MDFNMKF_LYNXROM,
 MDFNMKF_SNAP_DAT,
 MDFNMKF_CHEAT_TMP,
} MakeFName_Type;

std::string MDFN_MakeFName(MakeFName_Type type, int id1, const char *cd1);

const char * GetFNComponent(const char *str);

#endif
