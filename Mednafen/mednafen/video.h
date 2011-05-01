#ifndef __MDFN_VIDEO_H
#define __MDFN_VIDEO_H

#include "video/surface.h"
#include "video/primitives.h"
#include "video/text.h"

#include <algorithm>

void MDFN_ResetMessages(void);
void MDFN_InitFontData(void);
void MDFN_DispMessage(const char *format, ...) throw() MDFN_FORMATSTR(printf, 1, 2);

int MDFN_InitVirtualVideo(void);
void MDFN_KillVirtualVideo(void);


#endif
