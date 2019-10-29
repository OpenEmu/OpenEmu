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

typedef struct {
    AURenderPullInputBlock  pullInput;
    AudioTimeStamp const    *timestamp;
    void                    **buffer;
    UInt32                  *bufferSizeBytes;
} inputData;

static OSStatus audioConverterComplexInputDataProc(AudioConverterRef inAudioConverter, AVAudioPacketCount * ioNumberDataPackets, AudioBufferList * ioData, AudioStreamPacketDescription * __nullable * __nullable ioDataPacketDescription, void * inUserData)
{
    inputData *inp = (inputData *)inUserData;
    
    AudioUnitRenderActionFlags pullFlags = 0;
    ioData->mBuffers[0].mData = *inp->buffer;
    ioData->mBuffers[0].mDataByteSize = *inp->bufferSizeBytes;

    return inp->pullInput(&pullFlags, inp->timestamp, *ioNumberDataPackets, 0, ioData);
}

@interface CustomBus: AUAudioUnitBus
@end

@interface OEAudioUnit () {
    AudioConverterRef   _conv;
    void                *_convBuffer;
    UInt32              _convSizeBytes;
}

@property (nonatomic, readonly) BOOL    requiresConversion;
@property AUAudioUnitBus                *inputBus;
@property AUAudioUnitBusArray           *inputBusArray;
@property AUAudioUnitBus                *outputBus;
@property AUAudioUnitBusArray           *outputBusArray;

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
    AVAudioFormat *defaultFormat = [[AVAudioFormat alloc] initStandardFormatWithSampleRate:48000 channels:2];
    
    _inputBus       = [[CustomBus alloc] initWithFormat:defaultFormat error:nil];
    _inputBusArray  = [[AUAudioUnitBusArray alloc] initWithAudioUnit:self
                                                            busType:AUAudioUnitBusTypeInput
                                                             busses:@[_inputBus]];
    
    _outputBus      = [[AUAudioUnitBus alloc] initWithFormat:defaultFormat error:nil];
    _outputBusArray = [[AUAudioUnitBusArray alloc] initWithAudioUnit:self
                                                             busType:AUAudioUnitBusTypeOutput
                                                              busses: @[_outputBus]];
    
        
    self.maximumFramesToRender = 512;
    
    return self;
}

- (void)dealloc {
    [self _freeResources];
}

#pragma mark - AUAudioUnit (Overrides)

- (AUAudioUnitBusArray *)inputBusses {
    return _inputBusArray;
}

- (AUAudioUnitBusArray *)outputBusses {
    return _outputBusArray;
}

- (BOOL)requiresConversion {
    return ![_inputBus.format isEqual:_outputBus.format];
}

- (BOOL)allocateRenderResourcesAndReturnError:(NSError *__autoreleasing  _Nullable *)outError {
    if (![super allocateRenderResourcesAndReturnError:outError]) {
        return NO;
    }
    
    if (!self.requiresConversion) {
        return YES;
    }
    
    AudioStreamBasicDescription const *srcDesc = _inputBus.format.streamDescription;
    AudioStreamBasicDescription const *dstDesc = _outputBus.format.streamDescription;
    
    OSStatus status = AudioConverterNew(srcDesc, dstDesc, &_conv);
    if (status != noErr) {
        NSLog(@"unable to create audio converter: %d", status);
        return nil;
    }
    _convSizeBytes  = (UInt32)(srcDesc->mBytesPerFrame * self.maximumFramesToRender);
    _convBuffer     = malloc(_convSizeBytes);
    
    return YES;
}

- (void)deallocateRenderResources {
    [super deallocateRenderResources];
    [self _freeResources];
}

- (void)_freeResources {
    _convSizeBytes = 0;
    if (_convBuffer) {
        free(_convBuffer);
        _convBuffer = nil;
    }
    
    if (_conv) {
        AudioConverterDispose(_conv);
        _conv = nil;
    }
}

#pragma mark - AUAudioUnit (AUAudioUnitImplementation)

- (AUInternalRenderBlock)internalRenderBlock {
    AURenderPullInputBlock pullInput = _outputProvider;

    if (self.requiresConversion) {
        AudioConverterRef *conv = &_conv;
        
        __block inputData data = {
            .buffer             = &_convBuffer,
            .bufferSizeBytes    = &_convSizeBytes,
            .pullInput          = _outputProvider,
        };
        
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
            
            data.timestamp = timestamp;
            UInt32 packetSize = frameCount;
            
            OSStatus res = AudioConverterFillComplexBuffer(*conv,
                                                   audioConverterComplexInputDataProc,
                                                   (void *)&data,
                                                   &packetSize,
                                                   outputData,
                                                   nil);
            
            return res;
        };
    }
    
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

@implementation CustomBus {
    AVAudioFormat *_format;
}

- (BOOL)setFormat:(AVAudioFormat *)format error:(NSError *__autoreleasing  _Nullable *)outError {
    if (outError) {
        *outError = nil;
    }
    
    if ([_format isEqual:format]) {
        return YES;
    }
    
    [self willChangeValueForKey:@"format"];
    _format = format;
    [self didChangeValueForKey:@"format"];
    return YES;
}

- (AVAudioFormat *)format {
    return _format;
}

@end
