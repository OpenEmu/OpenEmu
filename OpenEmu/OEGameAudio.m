/*
 Copyright (c) 2009, OpenEmu Team
 
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

#import "OEGameAudio.h"
#import <OpenEmuBase/OpenEmuBase.h>
@import AudioToolbox;

typedef struct
{
    void *buffer; /* id<OEAudioBuffer> */
    int channelCount;
    int bytesPerSample;
} OEGameAudioContext;

ExtAudioFileRef recordingFile;


static OSStatus RenderCallback(void                       *in,
                               AudioUnitRenderActionFlags *ioActionFlags,
                               const AudioTimeStamp       *inTimeStamp,
                               UInt32                      inBusNumber,
                               UInt32                      inNumberFrames,
                               AudioBufferList            *ioData)
{
    OEGameAudioContext *context = (OEGameAudioContext *)in;
    id<OEAudioBuffer> buffer = (__bridge id<OEAudioBuffer>)context->buffer;
    
    NSInteger bytesRequested = inNumberFrames * context->bytesPerSample * context->channelCount;
    char *outBuffer = ioData->mBuffers[0].mData;
    
    NSInteger bytesCopied = [buffer read:outBuffer maxLength:bytesRequested];
    NSInteger rest = bytesRequested - bytesCopied;
    if (rest)
        memset(outBuffer+bytesCopied, 0, rest);

    return noErr;
}

@implementation OEGameAudio
{
    __weak OEGameCore  *_gameCore;
    OEGameAudioContext *_contexts;
    NSNumber           *_outputDeviceID; // nil if no output device has been set (use default)

    AUGraph   _graph;
    AUNode    _converterNode, _mixerNode, _outputNode;
    AudioUnit _converterUnit, _mixerUnit, _outputUnit;
}

// No default version for this class
- (id)init
{
    return nil;
}

// Designated Initializer
- (id)initWithCore:(OEGameCore *)core
{
    self = [super init];
    if(self != nil)
    {
        _gameCore = core;
    }
    
    return self;
}

- (void)dealloc
{
    free(_contexts);

    AUGraphUninitialize(_graph);
    //FIXME: added this line tonight.  do we need it?  Fuckety fuck fucking shitty Core Audio documentation... :X
    DisposeAUGraph(_graph);
}

- (void)startAudio
{
    [self createGraph];
}

- (void)stopAudio
{
    ExtAudioFileDispose(recordingFile);
    AUGraphStop(_graph);
    AUGraphClose(_graph);
    AUGraphUninitialize(_graph);
}

- (void)pauseAudio
{
    AUGraphStop(_graph);
}

- (void)resumeAudio
{
    AUGraphStart(_graph);
}

- (void)createGraph
{
    OSStatus err;
    
    AUGraphStop(_graph);
    AUGraphClose(_graph);
    AUGraphUninitialize(_graph);
    
    //Create the graph
    err = NewAUGraph(&_graph);
    if(err) NSLog(@"NewAUGraph failed");
    
    //Open the graph
    err = AUGraphOpen(_graph);
    if(err) NSLog(@"couldn't open graph");
    
    ComponentDescription desc;
    
    desc.componentType         = kAudioUnitType_Output;
    desc.componentSubType      = kAudioUnitSubType_DefaultOutput;
    desc.componentManufacturer = kAudioUnitManufacturer_Apple;
    desc.componentFlagsMask    = 0;
    desc.componentFlags        = 0;

    //Create the output node
    err = AUGraphAddNode(_graph, (const AudioComponentDescription *)&desc, &_outputNode);
    if(err) NSLog(@"couldn't create node for output unit");
    
    err = AUGraphNodeInfo(_graph, _outputNode, NULL, &_outputUnit);
    if(err) NSLog(@"couldn't get output from node");
    
    
    desc.componentType = kAudioUnitType_Mixer;
    desc.componentSubType = kAudioUnitSubType_StereoMixer;
    desc.componentManufacturer = kAudioUnitManufacturer_Apple;

    //Create the mixer node
    err = AUGraphAddNode(_graph, (const AudioComponentDescription *)&desc, &_mixerNode);
    if(err) NSLog(@"couldn't create node for file player");
    
    err = AUGraphNodeInfo(_graph, _mixerNode, NULL, &_mixerUnit);
    if(err) NSLog(@"couldn't get player unit from node");

    desc.componentType = kAudioUnitType_FormatConverter;
    desc.componentSubType = kAudioUnitSubType_AUConverter;
    desc.componentManufacturer = kAudioUnitManufacturer_Apple;
    
    NSUInteger bufferCount = [_gameCore audioBufferCount];

    _contexts = realloc(_contexts, sizeof(OEGameAudioContext) * bufferCount);
    for(UInt32 i = 0; i < bufferCount; ++i)
    {
        id<OEAudioBuffer> buffer = [_gameCore audioBufferAtIndex:i];
        _contexts[i] = (OEGameAudioContext){
            (__bridge void *)buffer,
            (UInt32)[_gameCore channelCountForBuffer:i],
            (UInt32)[_gameCore audioBitDepth] / 8};
        
        //Create the converter node
        err = AUGraphAddNode(_graph, (const AudioComponentDescription *)&desc, &_converterNode);
        if(err)  NSLog(@"couldn't create node for converter");
        
        err = AUGraphNodeInfo(_graph, _converterNode, NULL, &_converterUnit);
        if(err) NSLog(@"couldn't get player unit from converter");
        
        
        AURenderCallbackStruct renderStruct;
        renderStruct.inputProc = RenderCallback;
        renderStruct.inputProcRefCon = (void *)&_contexts[i];
        
        err = AudioUnitSetProperty(_converterUnit, kAudioUnitProperty_SetRenderCallback,
                                   kAudioUnitScope_Input, 0, &renderStruct, sizeof(AURenderCallbackStruct));
        if(err) DLog(@"Couldn't set the render callback");
        else DLog(@"Set the render callback");
        
        AudioStreamBasicDescription mDataFormat;
        UInt32 channelCount = _contexts[i].channelCount;
        UInt32 bytesPerSample = _contexts[i].bytesPerSample;
        int formatFlag = (bytesPerSample == 4) ? kLinearPCMFormatFlagIsFloat : kLinearPCMFormatFlagIsSignedInteger;
        mDataFormat.mSampleRate       = [_gameCore audioSampleRateForBuffer:i];
        mDataFormat.mFormatID         = kAudioFormatLinearPCM;
        mDataFormat.mFormatFlags      = formatFlag | kAudioFormatFlagsNativeEndian;
        mDataFormat.mBytesPerPacket   = bytesPerSample * channelCount;
        mDataFormat.mFramesPerPacket  = 1; // this means each packet in the AQ has two samples, one for each channel -> 4 bytes/frame/packet
        mDataFormat.mBytesPerFrame    = bytesPerSample * channelCount;
        mDataFormat.mChannelsPerFrame = channelCount;
        mDataFormat.mBitsPerChannel   = 8 * bytesPerSample;
        
        UInt32 bufSize = (UInt32)[buffer length] / mDataFormat.mBytesPerFrame;
        err = AudioUnitSetProperty(_converterUnit, kAudioUnitProperty_MaximumFramesPerSlice, kAudioUnitScope_Global, 0, &bufSize, sizeof(UInt32));
        if (err) NSLog(@"couldn't set max frames per slice");
        
        err = AudioUnitSetProperty(_converterUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 0, &mDataFormat, sizeof(AudioStreamBasicDescription));
        if(err) NSLog(@"couldn't set player's input stream format");
      
        err = AUGraphConnectNodeInput(_graph, _converterNode, 0, _mixerNode, i);
        if(err) NSLog(@"Couldn't connect the converter to the mixer");
    }
    // connect the player to the output unit (stream format will propagate)
         
    err = AUGraphConnectNodeInput(_graph, _mixerNode, 0, _outputNode, 0);
    if(err) NSLog(@"Could not connect the input of the output");
    
    
    //AudioUnitSetParameter(_outputUnit, kAudioUnitParameterUnit_LinearGain, kAudioUnitScope_Global, 0, [[[GameDocumentController sharedDocumentController] preferenceController] volume] ,0);
    AudioUnitSetParameter(_outputUnit, kAudioUnitParameterUnit_LinearGain, kAudioUnitScope_Global, 0, 1.0 ,0);

    AudioDeviceID outputDeviceID = [_outputDeviceID unsignedIntValue];
    if(outputDeviceID != 0)
        AudioUnitSetProperty(_outputUnit, kAudioOutputUnitProperty_CurrentDevice, kAudioUnitScope_Global, 0, &outputDeviceID, sizeof(outputDeviceID));

    err = AUGraphInitialize(_graph);
    if(err) NSLog(@"couldn't initialize graph");
    
    err = AUGraphStart(_graph);
    if(err) NSLog(@"couldn't start graph");
	
    //CFShow(_graph);
    [self setVolume:[self volume]];
}

- (AudioDeviceID)outputDeviceID
{
    return [_outputDeviceID unsignedIntValue];
}

- (void)setOutputDeviceID:(AudioDeviceID)outputDeviceID
{
    AudioDeviceID currentID = [self outputDeviceID];
    if(outputDeviceID != currentID)
    {
        _outputDeviceID = (outputDeviceID == 0 ? nil : @(outputDeviceID));

        if(_outputUnit != NULL)
            AudioUnitSetProperty(_outputUnit, kAudioOutputUnitProperty_CurrentDevice, kAudioUnitScope_Global, 0, &outputDeviceID, sizeof(outputDeviceID));
    }
}

- (void)setVolume:(CGFloat)aVolume
{
    _volume = aVolume;
    if(_outputUnit != NULL)
        AudioUnitSetParameter(_outputUnit, kAudioUnitParameterUnit_LinearGain, kAudioUnitScope_Global, 0, _volume, 0);
}

@end
