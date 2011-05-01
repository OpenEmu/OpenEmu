#ifndef __VIDEO_DRIVER_H
#define __VIDEO_DRIVER_H

#include "video.h"

void MDFND_DispMessage(UTF8 *text);
void MDFNI_SaveSnapshot(const MDFN_Surface *src, const MDFN_Rect *rect, const MDFN_Rect *LineWidths);

#endif
