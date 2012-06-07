#ifndef __MDFN_TBLUR_H
#define __MDFN_TBLUR_H

#include "../video.h"

void TBlur_Init(void);
void TBlur_Kill(void);
void TBlur_Run(EmulateSpecStruct *espec);
bool TBlur_IsOn(void);

#endif
