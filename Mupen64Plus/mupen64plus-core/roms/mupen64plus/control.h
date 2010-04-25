/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - control.h                                               *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2008 Marshallh                                          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <ultra64.h>

#include "include\config.h"

extern OSMesgQueue SiMessageQ;
/* extern OSPfs pfs[NUM_CONT]; */

OSContStatus statusData[NUM_CONT];
OSContPad    dummyControllerData = {0, 0, 0, 0};
OSContPad    controllerData[NUM_CONT];
OSContPad*   validControllerData[NUM_CONT];

int activeControllers[NUM_CONT];
int numControllers=0;

/* Return how many controllers are connected
 * if there are more than <NUM_CONT> connected, return NUM_CONT
 * (ie specify how many controllers you want with NUM_CONT, and
 *  the return result is the number of controllers actually hooked up)
 */
void initControllers(void)
{
    int i;
    u8 pattern;
    osContInit(&SiMessageQ,&pattern,statusData);
}

u32 initRumblePack(int contno)
{
    return -1;
}

/* Return pointer to controller data for each connected controller
 * oneshot = which buttons to treat as one-shots ("fire" buttons)
 * oneshot is any of the button macros (eg CONT_B, CONT_LEFT) ored together)
 */
void readController(void)
{
    /* MotorSiGetAccess(); */
    osContStartReadData(&SiMessageQ);
    osWritebackDCacheAll();
    osRecvMesg(&SiMessageQ, NULL, OS_MESG_BLOCK);
    /* MotorSiRelAccess(); */
    osContGetReadData(controllerData);
}

