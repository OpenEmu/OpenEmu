/*
 Copyright (c) 2009, OpenEmu Team
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
     * Neither the name of the OpenEmu Team nor the
       names of its contributors may be used to endorse or promote products
       derived from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY OpenEmu Team ''AS IS'' AND ANY
 EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL OpenEmu Team BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _MDFN_DRIVERS_INPUT_H
#define _MDFN_DRIVERS_INPUT_H

typedef struct
{
    uint8  ButtType;
    uint8  DeviceNum;
    uint32 ButtonNum;
    uint64 DeviceID;
} ButtConfig;

//extern CFGSTRUCT InputConfig[];

#define BUTTC_NONE     0x00
#define BUTTC_KEYBOARD 0x01
#define BUTTC_JOYSTICK 0x02
#define BUTTC_MOUSE    0x03

#define MKK(k) SDLK_##k
#define MKK_COUNT (SDLK_LAST+1)

// Called after a game is loaded.
void InitGameInput(MDFNGI *GI);

// Called to synch
void ResyncGameInputSettings(void);

// Called when a game is closed.
void KillGameInput(void);

void MDFND_UpdateInput(void);

void MakeInputSettings(std::vector <MDFNSetting> &settings);
void KillInputSettings(void); // Called after MDFNI_Kill() is called

extern bool DNeedRewind; // Only read/write in game thread.

bool InitCommandInput(void);
void KillCommandInput(void);

#endif
