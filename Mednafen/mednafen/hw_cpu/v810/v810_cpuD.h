////////////////////////////////////////////////////////////////
// Defines for the V810 CPU debug

#ifndef V810_CPU_D_H_
#define V810_CPU_D_H_

#include <stdio.h>
#include <string>
#include "v810_opt.h"

// Dissasemble num lines of code  starting at tPC (If tPC == -1 start at PC)
void v810_dis(uint32 &PC, int num, char *, uint16 (*rhword)(uint32));

#endif

