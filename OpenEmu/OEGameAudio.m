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
#import "OELogging.h"
#import <OpenEmuBase/OpenEmuBase.h>

@import AudioToolbox;
@import AVFoundation;
@import CoreAudioKit;

@implementation OEGameAudio {
    AVAudioEngine           *_engine;
    id                      _token;
    AVAudioUnitGenerator    *_gen;
    __weak OEGameCore       *_gameCore;
    AudioDeviceID           _outputDeviceID;
    BOOL                    _running; // specifies the expected state of OEGameAudio
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
    _running  = NO;
    _engine   = [AVAudioEngine new];

    __weak typeof(self) weakSelf = self;
    _token = [NSNotificationCenter.defaultCenter addObserverForName:AVAudioEngineConfigurationChangeNotification object:nil queue:NSOperationQueue.mainQueue usingBlock:^(NSNotification * _Nonnull note) {
        os_log_info(OE_LOG_AUDIO, "AVAudioEngine configuration change");
        if (weakSelf) {
            __strong typeof(weakSelf) strongSelf = weakSelf;
            strongSelf->_outputDeviceID = [strongSelf defaultAudioOutputDeviceID];
            [strongSelf setDeviceAndConnections];
            [strongSelf resumeAudio];
        }
    }];

    return self;
}

- (void)dealloc {
    if (_token) {
        [NSNotificationCenter.defaultCenter removeObserver:_token];
    }
}

- (void)audioSampleRateDidChange {
    if (_running) {
        [_engine stop];
        [self configureNodes];
        [_engine prepare];
        [self performSelector:@selector(resumeAudio) withObject:nil afterDelay:0.020];
    }
}

- (void)startAudio {
    NSAssert1(_gameCore.audioBufferCount == 1, @"only one buffer supported; got=%lu", _gameCore.audioBufferCount);

    [self createNodes];
    [self configureNodes];
    [self attachNodes];
    [self setDeviceAndConnections];
    
    [_engine prepare];
    // per the following, we need to wait before resuming to allow devices to start 🤦🏻‍♂️
    //  https://github.com/AudioKit/AudioKit/blob/f2a404ff6cf7492b93759d2cd954c8a5387c8b75/Examples/macOS/OutputSplitter/OutputSplitter/Audio/Output.swift#L88-L95
    [self performSelector:@selector(resumeAudio) withObject:nil afterDelay:0.020];
}

- (void)stopAudio {
    [_engine stop];
    [self detachNodes];
    [self destroyNodes];
    _running = NO;
}

- (void)pauseAudio
{
    [_engine pause];
    _running = NO;
}

- (void)resumeAudio
{
    _running = YES;
    NSError *err;
    if (![_engine startAndReturnError:&err]) {
        os_log_error(OE_LOG_AUDIO, "unable to start AVAudioEngine: %{public}s", err.localizedDescription.UTF8String);
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

- (AudioDeviceID)defaultAudioOutputDeviceID {
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

- (void)configureNodes {
    AVAudioFormat *renderFormat = [self renderFormat];
    OEAudioUnit     *au  = (OEAudioUnit*)_gen.AUAudioUnit;
    NSError         *err;
    AUAudioUnitBus  *bus = au.inputBusses[0];
    if (![bus setFormat:renderFormat error:&err]) {
        os_log_error(OE_LOG_AUDIO, "unable to set input bus render format %{public}s: %{public}s",
                     renderFormat.description.UTF8String,
                     err.localizedDescription.UTF8String);
        return;
    }

    OEAudioBufferReadBlock read = [self readBlockForBuffer:[_gameCore audioBufferAtIndex:0]];
    AudioStreamBasicDescription const *src = renderFormat.streamDescription;
    NSUInteger      bytesPerFrame = src->mBytesPerFrame;
    UInt32          channelCount  = src->mChannelsPerFrame;
    
    au.outputProvider = ^AUAudioUnitStatus(AudioUnitRenderActionFlags *actionFlags, const AudioTimeStamp *timestamp, AUAudioFrameCount frameCount, NSInteger inputBusNumber, AudioBufferList *inputData) {
        NSUInteger bytesRequested = frameCount * bytesPerFrame;
        NSUInteger bytesCopied    = read(inputData->mBuffers[0].mData, bytesRequested);
        
        inputData->mBuffers[0].mDataByteSize = (UInt32)bytesCopied;
        inputData->mBuffers[0].mNumberChannels = channelCount;
        
        return noErr;
    };
}

- (void)createNodes {
    AudioComponentDescription desc = {
        .componentType          = kAudioUnitType_Generator,
        .componentSubType       = kAudioUnitSubType_Emulator,
        .componentManufacturer  = kAudioUnitManufacturer_OpenEmu,
    };
    _gen = [[AVAudioUnitGenerator alloc] initWithAudioComponentDescription:desc];
}

- (void)destroyNodes {
    _gen = nil;
}

- (void)setDeviceAndConnections {
    if (_outputDeviceID == 0) {
        _outputDeviceID = [self defaultAudioOutputDeviceID];
        os_log_info(OE_LOG_AUDIO, "using default audio device %d", _outputDeviceID);
    }

    NSError *err;
    if (![_engine.outputNode.AUAudioUnit setDeviceID:_outputDeviceID error:&err]) {
        os_log_error(OE_LOG_AUDIO, "unable to set output device ID %d: %{public}s",
                     _outputDeviceID,
                     err.localizedDescription.UTF8String);
        return;
    }

    [_engine connect:_gen to:_engine.mainMixerNode format:nil];
    _engine.mainMixerNode.outputVolume = _volume;
}

- (void)attachNodes {
    [_engine attachNode:_gen];
}

- (void)detachNodes {
    [_engine detachNode:_gen];
}

- (AudioDeviceID)outputDeviceID
{
    return _outputDeviceID;
}

- (void)setOutputDeviceID:(AudioDeviceID)outputDeviceID
{
    if(outputDeviceID != _outputDeviceID)
    {
        // 0 indicates use the current system default output
        if (outputDeviceID == 0) {
            outputDeviceID = [self defaultAudioOutputDeviceID];
            os_log_info(OE_LOG_AUDIO, "using default audio device %d", _outputDeviceID);
        }

        _outputDeviceID = outputDeviceID;
        
        [_engine stop];
        [self setDeviceAndConnections];
        
        if (_running && !_engine.isRunning) {
            [_engine prepare];
            [self performSelector:@selector(resumeAudio) withObject:nil afterDelay:0.020];
        }
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
