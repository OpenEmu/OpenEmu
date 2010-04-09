#ifndef _MEDNAFEN_H

#include "types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gettext.h"

#define _(String) gettext (String)

#include "git.h"

extern MDFNGI *MDFNGameInfo;

typedef struct {
           int NetworkPlay;
	   int SoundVolume;
	   uint32 SndRate;
	   double soundmultiplier;
	   int rshift, gshift, bshift, ashift;
} MDFNS;

#include "settings.h"

extern MDFNS FSettings;

void MDFN_PrintError(const char *format, ...) __attribute__ ((format (printf, 1, 2)));
void MDFN_printf(const char *format, ...) __attribute__ ((format (printf, 1, 2)));
void MDFN_DispMessage(const char *format, ...) __attribute__ ((format (printf, 1, 2)));

class MDFNException
{
	public:

	MDFNException();
	~MDFNException();

	char TheMessage[1024];

	void AddPre(const char *format, ...);
	void AddPost(const char *format, ...);
};


void MDFN_LoadGameCheats(FILE *override);
void MDFN_FlushGameCheats(int nosave);
void MDFN_DoSimpleCommand(int cmd);
void MDFN_QSimpleCommand(int cmd);

#include "state.h"
int MDFN_RawInputStateAction(StateMem *sm, int load, int data_only);

#include "mednafen-driver.h"

#define _MEDNAFEN_H
#endif
