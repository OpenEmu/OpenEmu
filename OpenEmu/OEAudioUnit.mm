// Copyright (c) 2019, OpenEmu Team
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the OpenEmu Team nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY OpenEmu Team ''AS IS'' AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL OpenEmu Team BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#import <AudioUnit/AudioUnit.h>
#import <AVFoundation/AVFoundation.h>

#import "OEAudioUnit.h"

@interface OEAudioUnit ()

@property AUAudioUnitBus *outputBus;
@property AUAudioUnitBusArray *outputBusArray;

@end

@implementation OEAudioUnit

+ (void)registerSelf {
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        AudioComponentDescription desc = {
            .componentType          = kAudioUnitType_Generator,
            .componentSubType       = kAudioUnitSubType_Emulator,
            .componentManufacturer  = kAudioUnitManufacturer_OpenEmu,
            .componentFlags         = 0,
            .componentFlagsMask     = 0,
        };
        
        [AUAudioUnit registerSubclass: self.class
               asComponentDescription: desc
                                 name: @"OEAudioUnit"
                              version: UINT32_MAX];
    });
}

- (instancetype)initWithComponentDescription:(AudioComponentDescription)componentDescription options:(AudioComponentInstantiationOptions)options error:(NSError **)outError {
    self = [super initWithComponentDescription:componentDescription options:options error:outError];
    
    if (self == nil) {
        return nil;
    }
    
    // Initialize a default format for the busses. It doesn't matter what you put here.
    AVAudioFormat *defaultFormat = [[AVAudioFormat alloc] initStandardFormatWithSampleRate:44100 channels:2];
    
    // Create the output bus.
    _outputBus = [[AUAudioUnitBus alloc] initWithFormat:defaultFormat error:nil];
   
    // Create the input and output bus arrays.
    _outputBusArray = [[AUAudioUnitBusArray alloc] initWithAudioUnit:self
                                                             busType:AUAudioUnitBusTypeOutput
                                                              busses: @[_outputBus]];
        
    return self;
}

#pragma mark - AUAudioUnit (Overrides)

- (AUAudioUnitBusArray *)outputBusses {
    return _outputBusArray;
}

#pragma mark - AUAudioUnit (AUAudioUnitImplementation)

- (AUInternalRenderBlock)internalRenderBlock {
    AURenderPullInputBlock pullInput = _outputProvider;
    
    return ^AUAudioUnitStatus(
                              AudioUnitRenderActionFlags *actionFlags,
                              const AudioTimeStamp       *timestamp,
                              AVAudioFrameCount           frameCount,
                              NSInteger                   outputBusNumber,
                              AudioBufferList            *outputData,
                              const AURenderEvent        *realtimeEventListHead,
                              AURenderPullInputBlock      pullInputBlock) {
        
        if (pullInput == nil) {
            return kAudioUnitErr_NoConnection;
        }
        
        AudioUnitRenderActionFlags pullFlags = 0;
        return pullInput(&pullFlags, timestamp, frameCount, 0, outputData);
    };
}

@end
