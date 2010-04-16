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

#include <base.hpp>
#include "BSNESInterface.h"

#define conv555to565(a) ((((a)&~0x1f)<<1)|((a)&0x1f))

// TODO: Make a LUT for proper SNES pallated conversion from 555 to 565
void BSNESInterface::video_refresh(uint16_t *input, unsigned apitch, unsigned *line, unsigned awidth, unsigned aheight)
{
    width = awidth;
    height = aheight;
    pitch = apitch;
    
    
    pitch >>= 1;
    
    uint16_t *output = video;
    for(unsigned y = 0; y < height; y++)
    {
        if(width == 512 && line[y] == 256)
        {
            for(unsigned x = 0; x < 256; x++)
            {
                uint16_t p = *input++;
                p = conv555to565(p);
                *output++ = p;
                *output++ = p;
            }
            input += 256;
        }
        else
        {
            for(unsigned x = 0; x < width; x++)
            {
                uint16_t p = *input++;
                p = conv555to565(p);
                *output++ = p;
            }
        }
        input  += pitch - width;
        output += pitch - width;
    }
}

void BSNESInterface::audio_sample(uint16_t left, uint16_t right) {
    // if(config.audio.mute) left = right = 0;
    // audio.sample(left, right);
    //printf("sampled");
    
    [ringBuffer write:&left maxLength:2];
    [ringBuffer write:&right maxLength:2];
}

void BSNESInterface::input_poll() {
    // inputManager.poll();
}

int16_t BSNESInterface::input_poll(unsigned deviceid, unsigned a) {
    //NSLog(@"polled input: device: %d id: %d", deviceid, id);
    if (deviceid == SNES::Input::DeviceIDJoypad1) {
        return pad[0][a];
    }
    else if(deviceid == SNES::Input::DeviceIDJoypad2) {
        return pad[1][a];
    }
    
    return 0;//inputManager.getStatus(deviceid, id);
}
