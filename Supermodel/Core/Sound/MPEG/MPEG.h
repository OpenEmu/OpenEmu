/**
 ** Supermodel
 ** A Sega Model 3 Arcade Emulator.
 ** Copyright 2011 Bart Trzynadlowski, Nik Henson
 **
 ** This file is part of Supermodel.
 **
 ** Supermodel is free software: you can redistribute it and/or modify it under
 ** the terms of the GNU General Public License as published by the Free 
 ** Software Foundation, either version 3 of the License, or (at your option)
 ** any later version.
 **
 ** Supermodel is distributed in the hope that it will be useful, but WITHOUT
 ** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 ** FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 ** more details.
 **
 ** You should have received a copy of the GNU General Public License along
 ** with Supermodel.  If not, see <http://www.gnu.org/licenses/>.
 **/
 
/*
 * MPEG.h
 * 
 * Header file for MPEG decoder based on AMP by Tomislav Uzalec. Modified to
 * play from memory buffers by R. Belmont for his arcade music player, M1.
 */
 
#ifndef INCLUDED_MPEG_H
#define INCLUDED_MPEG_H

#include "Types.h"


/******************************************************************************
 Functions
 
 The MPEG decoder is not thread-safe and must not be used by multiple sources.
 These functions are located in audio.cpp and getbits.cpp.
******************************************************************************/

/*
 * MPEG_IsPlaying(void):
 *
 * Returns:
 *		TRUE if an MPEG stream is currently playing, otherwise false.
 */
extern bool MPEG_IsPlaying(void);

/*
 * MPEG_GetPlayPosition(playOffset, endOffset):
 *
 * Obtains the current playback and end offsets of the MPEG stream.
 *
 * Parameters:
 *		playOffset	Pointer to which playback byte offset will be written.
 *		endOffset	Pointer to which end offset will be written.
 */
extern void MPEG_GetPlayPosition(int *playOffset, int *endOffset);

/*
 * MPEG_SetPlayPosition(playOffset, endOffset):
 *
 * Sets the playback position within an MPEG stream.
 *
 * Parameters:
 *		playOffset	Playback byte offset (relative to beginning of the current
 *					MPEG stream).
 *		endOffset	End offset.
 */
extern void MPEG_SetPlayPosition(int playOffset, int endOffset);

/*
 * MPEG_SetLoop(loop, loopend):
 *
 * Sets the start and end offsets for looped playback.
 *
 * Parameters:
 *		loop		Start address.
 *		loopend		End offset. Must NOT be 0!
 */
extern void MPEG_SetLoop(const char *loop, int loopend);		

/*
 * MPEG_Decode(outputs, length):
 *
 * Decodes the requested number of samples from the currently playing MPEG
 * stream and updates the internal play position. If an MPEG is not playing,
 * writes silence (zeros).
 *
 * Parameters:
 *		outputs		A two-element array of pointers to equal-length signed 16-
 *					bit sample buffers. The first is the left channel and the
 *					second is the right channel. Audio is decoded to these.
 *		length		Number of samples to decode.
 */
extern void MPEG_Decode(INT16 **outputs, int length);			

/*
 * MPEG_PlayMemory(sa, length):
 *
 * Specifies the memory buffer to decode from. This initializes the playback
 * process and will decode the first MPEG frame internally. The loop start
 * position is cleared (set looping after this call).
 *
 * Parameters:
 *		sa		Start address of MPEG stream.
 *		length	Length in bytes.
 */
extern void MPEG_PlayMemory(const char *sa, int length);		

/*
 * MPEG_StopPlaying(void):
 *
 * Stop playing the current MPEG stream. The decoder will return silence.
 */
extern void MPEG_StopPlaying(void);									

/*
 * MPEG_Init(void):
 *
 * Initializes the MPEG decoder. This should be called once per program
 * session. Allocates an internal buffer for MPEG decoding.
 *
 * Returns:
 *		OKAY if successful, FAIL if internal buffer could not be allocated.
 */
extern bool MPEG_Init(void);

/*
 * MPEG_Shutdown(void):
 *
 * Shuts down the MPEG decoder. Releases internal memory.
 */
extern void MPEG_Shutdown(void);									


#endif	// INCLUDED_MPEG_H