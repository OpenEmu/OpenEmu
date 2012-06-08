#ifndef _GENERAL_H
#define _GENERAL_H

#include <string>

extern uint32 MDFN_RoundUpPow2(uint32);

void GetFileBase(const char *f);

// File-inclusion for-read-only path, for PSF and CUE/TOC sheet usage.
bool MDFN_IsFIROPSafe(const std::string &path);

std::string MDFN_MakeFName(int type, int id1, const char *cd1);
char *MDFN_RemoveControlChars(char *str);

void MDFN_ltrim(char *string);
void MDFN_rtrim(char *string);
void MDFN_trim(char *string);

typedef enum
{
 MDFNMKF_STATE = 0,
 MDFNMKF_SNAP,
 MDFNMKF_SAV,
 MDFNMKF_CHEAT,
 MDFNMKF_PALETTE,
 MDFNMKF_IPS,
 MDFNMKF_MOVIE,
 MDFNMKF_AUX,
 MDFNMKF_SNAP_DAT,
 MDFNMKF_CHEAT_TMP,
 MDFNMKF_FIRMWARE
} MakeFName_Type;

std::string MDFN_MakeFName(MakeFName_Type type, int id1, const char *cd1);

const char * GetFNComponent(const char *str);

void MDFN_GetFilePathComponents(const std::string &file_path, std::string *dir_path_out, std::string *file_base_out = NULL, std::string *file_ext_out = NULL);
std::string MDFN_EvalFIP(const std::string &dir_path, const std::string &rel_path, bool skip_safety_check = false);
#endif
