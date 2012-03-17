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

#import <Cocoa/Cocoa.h>
#import <OERingBuffer.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "audio_osx.hpp"
#include "emulator/g_main.hpp"

#include "gens_core/sound/psg.h"
#include "gens_core/sound/ym2612.h"
#include "gens_core/mem/mem_m68k.h"
#include "gens_core/vdp/vdp_io.h"
#include "gens_core/sound/pcm.h"
#include "gens_core/misc/cpuflags.h"

Audio_OSX::Audio_OSX(void *ringBuffer)
{
//	soundLock = [NSLock new];
//	audio_len = 0;
    _ringBuffer = ringBuffer;
	pMsndOut = NULL;
	audiobuf = NULL;
}

Audio_OSX::~Audio_OSX()
{
}

/**
 * initSound(): Initialize sound.
 * @return 1 on success; -1 if sound was already initialized; 0 on error.
 */
int Audio_OSX::initSound(void)
{
	int i;
	int videoLines;
	
	if (m_soundInitialized)
		return -1;
	
	// Make sure sound is ended first.
	endSound();
	
	switch (m_soundRate)
	{
		case 11025:
			m_segLength = (CPU_Mode ? 220 : 184);
			break;
		case 22050:
			m_segLength = (CPU_Mode ? 441 : 368);
			break;
		case 44100:
			m_segLength = (CPU_Mode ? 882 : 735);
			break;
	}
	
	videoLines = (CPU_Mode ? 312 : 262);
	for (i = 0; i < videoLines; i++)
	{
		Sound_Extrapol[i][0] = ((m_segLength * i) / videoLines);
		Sound_Extrapol[i][1] = (((m_segLength * (i + 1)) / videoLines) - Sound_Extrapol[i][0]);
	}
	for (i = 0; i < m_segLength; i++)
		Sound_Interpol[i] = ((videoLines * i) / m_segLength);
	
	// Clear the segment buffers.
	memset(Seg_L, 0, m_segLength << 2);
	memset(Seg_R, 0, m_segLength << 2);
	
	// Allocate the segment buffer.
	pMsndOut = static_cast<unsigned char*>(malloc(m_segLength << 2));

	
    [(OERingBuffer *)_ringBuffer setLength:(m_segLength * 2 * 2 * 4) * sizeof(short) * 5];
	//ringBuffer = [[OERingBuffer alloc] initWithLength:(m_segLength * 2 * 2 * 4) * sizeof(short) * 5];
	
	//audiobuf = static_cast<unsigned char*>(malloc((m_segLength * 2 * 2 * 4) * sizeof(short)));
	
	//memset(audiobuf, 0, (m_segLength * 2 * 2 * 4) * sizeof(short));
	
	m_soundInitialized = true;
	return 1;
}


/**
 * endSound(): Stop sound output.
 */
void Audio_OSX::endSound(void)
{
	//free(audiobuf);
	//audiobuf = NULL;
	
	//[ringBuffer release];
	
	free(pMsndOut);
	pMsndOut = NULL;
	
	if (m_soundInitialized)
	{
		m_soundIsPlaying = false;
		m_soundInitialized = false;
	}
}


int Audio_OSX::getCurrentSeg(void)
{	
	return 0;
}


int Audio_OSX::checkSoundTiming(void)
{
	return 0;
}


/**
 * writeSoundBuffer(): Write the sound buffer to the audio output.
 * @param dumpBuf Sound dumping buffer.
 * @return 1 on success.
 */
int Audio_OSX::writeSoundBuffer(void *dumpBuf)
{
//	[soundLock lock];
	
	if (m_stereo)
	{
		writeSoundStereo(reinterpret_cast<short*>(pMsndOut), m_segLength);
	}
	
	[(OERingBuffer *)_ringBuffer write:pMsndOut maxLength:m_segLength * 4];
//	memcpy(audiobuf + audio_len, pMsndOut, m_segLength * 4);
//	audio_len += m_segLength * 4;

//	[soundLock unlock];
	return 1;
}

int Audio_OSX::readSoundBuffer(void *buffer, int len)
{
	return [(OERingBuffer *)_ringBuffer read:buffer maxLength:len];
}

void Audio_OSX::lock(void)
{
//	[soundLock lock];
}

void Audio_OSX::unlock(void)
{
//	[soundLock unlock];
}


int Audio_OSX::clearSoundBuffer(void)
{
	return 0;
}


int Audio_OSX::playSound(void)
{
	m_soundIsPlaying = true;
	return 1;
}


int Audio_OSX::stopSound(void)
{
	m_soundIsPlaying = false;
	return 1;
}

/**
 * waitForAudioBuffer(): Wait for the audio buffer to empty out.
 * This function is used for Auto Frame Skip.
 */
void Audio_OSX::waitForAudioBuffer(void)
{
	writeSoundBuffer(NULL);
/*	while (audio_len <= (m_segLength * Seg_To_Buffer))
	{
		Update_Frame_Fast();
		writeSoundBuffer(NULL);
	}*/
}
