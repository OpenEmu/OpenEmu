#ifndef _MEDNAFEN_H

#include "types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gettext.h"

#define _(String) gettext (String)

#include "math_ops.h"
#include "git.h"

extern MDFNGI *MDFNGameInfo;

#include "settings.h"

void MDFN_PrintError(const char *format, ...) throw() MDFN_FORMATSTR(printf, 1, 2);
void MDFN_printf(const char *format, ...) throw() MDFN_FORMATSTR(printf, 1, 2);
void MDFN_DispMessage(const char *format, ...) throw() MDFN_FORMATSTR(printf, 1, 2);

void MDFN_DebugPrintReal(const char *file, const int line, const char *format, ...) MDFN_FORMATSTR(printf, 3, 4);

#define MDFN_DebugPrint(format, ...) MDFN_DebugPrintReal(__FILE__, __LINE__, format, ## __VA_ARGS__)


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

void MDFN_MidSync(EmulateSpecStruct *espec);

#include "state.h"
int MDFN_RawInputStateAction(StateMem *sm, int load, int data_only);

#include "mednafen-driver.h"

#include "endian.h"
#include "memory.h"

#define _MEDNAFEN_H
#endif
