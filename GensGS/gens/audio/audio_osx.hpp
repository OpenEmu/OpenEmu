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

#ifndef GENS_AUDIO_OSX_HPP
#define GENS_AUDIO_OSX_HPP

#include "audio.hpp"
class Audio_OSX : public Audio
{
private:
    void *_ringBuffer;
public:
    Audio_OSX(void *ringBuffer);
    ~Audio_OSX();
    
    // Initialize / End sound.
    int initSound(void);
    void endSound(void);
    
    // Audio callback functions.
    static void AudioCallback(void *user, UInt8 *buffer, int len);
    void audioCallback_int(UInt8 *buffer, int len);
    
    // Miscellaneous functions that need to be sorted through.
    int getCurrentSeg(void);
    bool lotsInAudioBuffer(void);
    int checkSoundTiming(void);
    int writeSoundBuffer(void *dumpBuf);
    int clearSoundBuffer(void);
    int playSound(void);
    int stopSound(void);
    int readSoundBuffer(void* buffer, int len);
    // Auto Frame Skip functions
    //void wpSegWait(void) { }
    void waitForAudioBuffer(void);
    void lock();
    void unlock();
    unsigned char *pMsndOut;
    unsigned char *audiobuf;
};

#endif /* GENS_AUDIO_SDL_HPP */
