#ifndef __MDFN_VBLUR_H
#define __MDFN_VBLUR_H

void VBlur_Init(void);
void VBlur_Kill(void);
void VBlur_Run(EmulateSpecStruct *espec);
bool VBlur_IsOn(void);

#endif
