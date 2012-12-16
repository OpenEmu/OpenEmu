/*
  www.freedo.org
The first and only working 3DO multiplayer emulator.

The FreeDO licensed under modified GNU LGPL, with following notes:

*   The owners and original authors of the FreeDO have full right to develop closed source derivative work.
*   Any non-commercial uses of the FreeDO sources or any knowledge obtained by studying or reverse engineering
    of the sources, or any other material published by FreeDO have to be accompanied with full credits.
*   Any commercial uses of FreeDO sources or any knowledge obtained by studying or reverse engineering of the sources,
    or any other material published by FreeDO is strictly forbidden without owners approval.

The above notes are taking precedence over GNU LGPL in conflicting situations.

Project authors:

Alexander Troosh
Maxim Grishin
Allen Wright
John Sammons
Felix Lazarev
*/

#ifndef FREEDOCONFIG_H
#define FREEDOCONFIG_H

#ifdef __MSVC__
#include <stdafx.h>
#include <windows.h>
#else
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>



extern bool __temporalfixes;
extern int HightResMode;
#define RESSCALE        HightResMode
#define DEBUG_CORE
#define _T(a) (a)

#endif

#include "types.h"


#endif // FREEDOCONFIG_H
