/*
 Copyright (c) 2008 Francisco Muñoz 'Hermes' <www.elotrolado.net>
 All rights reserved.

 Redistribution and use in source and binary forms, with or without modification, are
 permitted provided that the following conditions are met:

 - Redistributions of source code must retain the above copyright notice, this list of
 conditions and the following disclaimer.
 - Redistributions in binary form must reproduce the above copyright notice, this list
 of conditions and the following disclaimer in the documentation and/or other
 materials provided with the distribution.
 - The names of the contributors may not be used to endorse or promote products derived
 from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
 EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef NO_SOUND

#ifndef __OGGPLAYER_H__
#define __OGGPLAYER_H__

#include <asndlib.h>
#include "tremor/ivorbiscodec.h"
#include "tremor/ivorbisfile.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define OGG_ONE_TIME         0
#define OGG_INFINITE_TIME    1

#define OGG_STATUS_RUNNING   1
#define OGG_STATUS_ERR      -1
#define OGG_STATUS_PAUSED    2
#define OGG_STATUS_EOF     255

/*------------------------------------------------------------------------------------------------------------------------------------------------------*/
/* Player OGG functions                                                                                                                                 */
/*------------------------------------------------------------------------------------------------------------------------------------------------------*/

/* int PlayOgg(int fd, int time_pos, int mode);

 Play an Ogg file. This file can be loaded from memory (mem_open(void *ogg, int size_ogg)) or from device with open("device:file.ogg",O_RDONLY,0);

 NOTE: The file is closed by the player when you call PlayOgg(), StopOgg() or if it fail.

 -- Params ---

 buf: pointer to sound data

 buflen: buffer size in bytes

 time_pos: initial time position in the file (in milliseconds). For example, use 30000 to advance 30 seconds

 mode: Use OGG_ONE_TIME or OGG_INFINITE_TIME. When you use OGG_ONE_TIME the sound stops and StatusOgg() return OGG_STATUS_EOF

 return: 0- Ok, -1 Error

 */

int PlayOgg(char * buf, int buflen, int time_pos, int mode);

/*------------------------------------------------------------------------------------------------------------------------------------------------------*/

/* void StopOgg();

 Stop an Ogg file.

 NOTE: The file is closed and the player thread is released

 -- Params ---


 */

void StopOgg();

/*------------------------------------------------------------------------------------------------------------------------------------------------------*/

/* void PauseOgg(int pause);

 Pause an Ogg file.

 -- Params ---

 pause: 0 -> continue, 1-> pause

 */

void PauseOgg(int pause);

/*------------------------------------------------------------------------------------------------------------------------------------------------------*/

/* int StatusOgg();

 Return the Ogg status

 -- Params ---


 return: OGG_STATUS_RUNNING
 OGG_STATUS_ERR    -> not initialized?
 OGG_STATUS_PAUSED
 OGG_STATUS_EOF    -> player stopped by End Of File

 */

int StatusOgg();

/*------------------------------------------------------------------------------------------------------------------------------------------------------*/

/* void SetVolumeOgg(int volume);

 Set the Ogg playing volume.
 NOTE: it change the volume of voice 0 (used for the Ogg player)

 -- Params ---

 volume: 0 to 255 (max)

 */

void SetVolumeOgg(int volume);

/*------------------------------------------------------------------------------------------------------------------------------------------------------*/

/* s32 GetTimeOgg();

 Return the Ogg time from the starts of the file

 -- Params ---

 return:  0 -> Ok or error condition  (you must ignore this value)
 >0 -> time in milliseconds from the starts

 */

s32 GetTimeOgg();

/*------------------------------------------------------------------------------------------------------------------------------------------------------*/

/* void SetTimeOgg(s32 time_pos);

 Set the time position

 NOTE: The file is closed by the player when you call PlayOgg(), StopOgg() or if it fail.

 -- Params ---

 time_pos: time position in the file (in milliseconds). For example, use 30000 to advance 30 seconds

 */

void SetTimeOgg(s32 time_pos);

/*------------------------------------------------------------------------------------------------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif

#endif
