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

#import "OEGameAudio.h"
#import "OEAudioUnit.h"
#import <OpenEmuBase/OpenEmuBase.h>
#import <objc/runtime.h>

@import AudioToolbox;
@import AVFoundation;
@import CoreAudioKit;

typedef OSStatus (^AudioConverterInputBlock)(AVAudioPacketCount * ioNumberDataPackets, AudioBufferList * ioData);

static OSStatus audioConverterComplexInputDataProc(AudioConverterRef inAudioConverter, AVAudioPacketCount * ioNumberDataPackets, AudioBufferList * ioData, AudioStreamPacketDescription * __nullable * __nullable ioDataPacketDescription, void * inUserData)
{
    AudioConverterInputBlock block = (__bridge AudioConverterInputBlock)inUserData;
    return block(ioNumberDataPackets, ioData);
}

static OSStatus AudioConverterFillComplexBufferBlock(AudioConverterRef inAudioConverter, AVAudioPacketCount * ioOutputDataPacketSize, AudioBufferList * outOutputData, AudioStreamPacketDescription * __nullable outPacketDescription, AudioConverterInputBlock block)
{
    return AudioConverterFillComplexBuffer(inAudioConverter,
                                           audioConverterComplexInputDataProc,
                                           (__bridge void *)block,
                                           ioOutputDataPacketSize,
                                           outOutputData,
                                           outPacketDescription);
}


@implementation OEGameAudio {
    AVAudioEngine           *_engine;
    __weak OEGameCore       *_gameCore;
    AudioDeviceID           _outputDeviceID;
    AudioConverterRef       _conv;
    void                    *_convBuffer;
}

- (id)initWithCore:(OEGameCore *)core
{
    if((self = [super init]) == nil)
    {
        return nil;
    }
    
    [OEAudioUnit registerSelf];
    _gameCore = core;
    _volume   = 1.0;
    
    return self;
}

- (void)dealloc {
    [self freeResources];
}

- (void)freeResources {
    
    if (_engine) {
        [self stopAudio];
        _engine = nil;
    }
    
    if (_convBuffer) {
        free(_convBuffer);
        _convBuffer = nil;
    }
    
    if (_conv) {
        AudioConverterDispose(_conv);
        _conv = nil;
    }
}

- (void)startAudio {
    [self createAudioEngine];
}

- (void)stopAudio {
    [_engine stop];
}

- (void)pauseAudio
{
    [_engine pause];
}

- (void)resumeAudio
{
    NSError *err;
    if (![_engine startAndReturnError:&err]) {
        NSLog(@"failed to start audio hardware, %@", err.localizedDescription);
        return;
    }
}

- (OEAudioBufferReadBlock)readBlockForBuffer:(id<OEAudioBuffer>)buffer {
    if ([buffer respondsToSelector:@selector(readBlock)]) {
        return [buffer readBlock];
    }

    return ^NSUInteger(void * buf, NSUInteger max) {
        return [buffer read:buf maxLength:max];
    };
}

- (AudioConverterInputBlock)createConverterBlockFromStream:(AudioStreamBasicDescription const *)src
                                                  toStream:(AudioStreamBasicDescription const *)dst
                                                    buffer:(id<OEAudioBuffer>)buffer {
    OSStatus status = AudioConverterNew(src, dst, &_conv);
    if (status != noErr) {
        NSLog(@"unable to create audio converter: %d", status);
        return nil;
    }
    _convBuffer = malloc(buffer.length);

    // block state
    OEAudioBufferReadBlock read = [self readBlockForBuffer:buffer];
    
    NSUInteger      bytesPerFrame   = src->mBytesPerFrame;
    UInt32          channelCount    = src->mChannelsPerFrame;
    void            *outBuffer      = _convBuffer;
    
    return ^OSStatus(AVAudioPacketCount * ioNumberDataPackets, AudioBufferList * ioData) {
        NSUInteger bytesRequested = *ioNumberDataPackets * bytesPerFrame;
        NSUInteger bytesCopied    = read(outBuffer, bytesRequested);
        
        ioData->mBuffers[0].mData = outBuffer;
        ioData->mBuffers[0].mDataByteSize = (UInt32)bytesCopied;
        ioData->mBuffers[0].mNumberChannels = channelCount;
        
        return noErr;
    };
}

- (AVAudioFormat *)renderFormat {
    UInt32 channelCount      = (UInt32)[_gameCore channelCountForBuffer:0];
    UInt32 bytesPerSample    = (UInt32)[_gameCore audioBitDepth] / 8;
    
    AudioStreamBasicDescription mDataFormat = {
        .mSampleRate       = [_gameCore audioSampleRateForBuffer:0],
        .mFormatID         = kAudioFormatLinearPCM,
        .mFormatFlags      = kLinearPCMFormatFlagIsSignedInteger | kAudioFormatFlagsNativeEndian,
        .mBytesPerPacket   = bytesPerSample * channelCount,
        .mFramesPerPacket  = 1,
        .mBytesPerFrame    = bytesPerSample * channelCount,
        .mChannelsPerFrame = channelCount,
        .mBitsPerChannel   = 8 * bytesPerSample,
    };
    
    return [[AVAudioFormat alloc] initWithStreamDescription:&mDataFormat];
}

- (AudioDeviceID)currentAudioOutputDeviceID {
    AudioObjectPropertyAddress addr = {
        .mSelector = kAudioHardwarePropertyDefaultOutputDevice,
        .mScope    = kAudioObjectPropertyScopeGlobal,
        .mElement  = kAudioObjectPropertyElementMaster,
    };
    
    AudioObjectID deviceID = kAudioDeviceUnknown;
    UInt32 size = sizeof(deviceID);
    AudioObjectGetPropertyData(kAudioObjectSystemObject, &addr, 0, nil, &size, &deviceID);
    return deviceID;
}

- (void)createAudioEngine {
    [self freeResources];
    
    _engine = [AVAudioEngine new];
    
    if (_outputDeviceID == 0) {
        _outputDeviceID = [self currentAudioOutputDeviceID];
    }
    
    NSError *err;
    if (![_engine.outputNode.AUAudioUnit setDeviceID:_outputDeviceID error:&err]) {
        NSLog(@"unable to set output device: %@", err.localizedDescription);
        return;
    }
    
    NSAssert1(_gameCore.audioBufferCount == 1, @"only one buffer supported; got=%lu", _gameCore.audioBufferCount);
    
    AVAudioFormat *renderFormat = [self renderFormat];
    AVAudioFormat *floatRenderFormat = [[AVAudioFormat alloc] initStandardFormatWithSampleRate:renderFormat.sampleRate channels:renderFormat.channelCount];

    // AVAudioEngine only supports 32-bit float audio. Feed it our sample rate so it can figure out buffering.
    AudioConverterInputBlock b  = [self createConverterBlockFromStream:renderFormat.streamDescription
                                                              toStream:floatRenderFormat.streamDescription
                                                                buffer:[_gameCore audioBufferAtIndex:0]];
    if (b == nil) {
        return;
    }
    
    AudioComponentDescription desc = {
        .componentType          = kAudioUnitType_Generator,
        .componentSubType       = kAudioUnitSubType_Emulator,
        .componentManufacturer  = kAudioUnitManufacturer_OpenEmu,
    };
    AVAudioUnitGenerator *gen = [[AVAudioUnitGenerator alloc] initWithAudioComponentDescription:desc];
    OEAudioUnit          *au  = (OEAudioUnit*)gen.AUAudioUnit;
    __block AudioConverterRef conv = _conv;
    au.outputProvider = ^AUAudioUnitStatus(AudioUnitRenderActionFlags *actionFlags, const AudioTimeStamp *timestamp, AUAudioFrameCount frameCount, NSInteger inputBusNumber, AudioBufferList *inputData) {
        return AudioConverterFillComplexBufferBlock(conv, &frameCount, inputData, nil, b);
    };
    
    [_engine attachNode:gen];
    [_engine connect:gen to:_engine.mainMixerNode format:floatRenderFormat];
    _engine.mainMixerNode.outputVolume = _volume;
    [_engine prepare];
    // per the following, we need to wait before resuming to allow devices to start 🤦🏻‍♂️
    //  https://github.com/AudioKit/AudioKit/blob/f2a404ff6cf7492b93759d2cd954c8a5387c8b75/Examples/macOS/OutputSplitter/OutputSplitter/Audio/Output.swift#L88-L95
    [self performSelector:@selector(resumeAudio) withObject:nil afterDelay:0.020];
}


- (AudioDeviceID)outputDeviceID
{
    return _outputDeviceID;
}

- (void)setOutputDeviceID:(AudioDeviceID)outputDeviceID
{
    AudioDeviceID currentID = _outputDeviceID;
    if(outputDeviceID != currentID)
    {
        _outputDeviceID = outputDeviceID;
        [self stopAudio];
        [self performSelector:@selector(createAudioEngine) withObject:nil afterDelay:0.020];
        //[self createAudioEngine];
    }
}

- (void)setVolume:(CGFloat)aVolume
{
    _volume = aVolume;
    if (_engine) {
        _engine.mainMixerNode.outputVolume = _volume;
    }
}


@end
