/*
 Copyright (c) 2012, OpenEmu Team
 
 
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

#import "OERewindBuffer.h"
#import <vector>
#import <memory>

struct OERewindDiffData
{
    uint32_t offset;
    uint32_t diff;
};

struct OERewindPatch
{
    std::vector<OERewindDiffData> diffs;
};

@interface OERewindBuffer ()
{
    uint32_t *_stateData;
    std::vector<std::unique_ptr<OERewindPatch> > patches;
    NSUInteger _stateSize;
}
@end

@implementation OERewindBuffer

- (id)initWithStateSize:(NSUInteger)byteSize
{
    if ((self = [super init]))
    {
        _stateSize = (byteSize / 4) + 1;
        _stateData = (uint32_t*)calloc(_stateSize, sizeof(uint32_t));
    }
    return self;
}

- (void)dealloc
{
    free(_stateData);
}

- (void)pushState:(NSData *)stateData
{
    uint32_t *incomingData = (uint32_t*)[stateData bytes];
    
    OERewindPatch *patch = new OERewindPatch();
    
    for (uint32_t offset = 0; offset < [stateData length] / 4; ++offset)
    {
        uint32_t currentData = _stateData[offset];
        uint32_t newData = incomingData[offset];
        uint32_t diff = currentData ^ newData;
        
        if (diff)
        {
            patch->diffs.push_back((OERewindDiffData){offset, diff});
        }
        
        _stateData[offset] = newData;
    }
    
    
    patches.push_back(std::unique_ptr<OERewindPatch>(patch));
}

- (NSData *)popState
{
    NSData *data = [NSData dataWithBytes:_stateData length:_stateSize * 4];
    
    OERewindPatch *patch = patches.back().get();
    
    for (auto itr = patch->diffs.begin(); itr < patch->diffs.end(); ++itr)
    {
        auto diff = *itr;
        _stateData[diff.offset] ^= diff.diff;
    }

    patches.pop_back();
    
    return data;
}

- (BOOL)isEmpty
{
    return patches.empty();
}

@end
