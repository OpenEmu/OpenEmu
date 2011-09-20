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
#import "OEGameCore.h"


ExtAudioFileRef recordingFile;

OSStatus RenderCallback(void                       *in,
                        AudioUnitRenderActionFlags *ioActionFlags,
                        const AudioTimeStamp       *inTimeStamp,
                        UInt32                      inBusNumber,
                        UInt32                      inNumberFrames,
                        AudioBufferList            *ioData)
{
    [((OEGameCore *)in) getAudioBuffer:ioData->mBuffers[0].mData frameCount:inNumberFrames bufferIndex:0];
    //ExtAudioFileWriteAsync( recordingFile, inNumberFrames, ioData );
    
    return 0;
}


@implementation OEGameAudio

// No default version for this class
- (id)init
{
    [self release];
    return nil;
}

// Designated Initializer
- (id)initWithCore:(OEGameCore *)core
{
    self = [super init];
    if(self != nil)
    {
        gameCore = core;
        [self createGraph];
    }
    
    return self;
}

- (void)dealloc
{
    AUGraphUninitialize(mGraph);
    //FIXME: added this line tonight.  do we need it?  Fuckety fuck fucking shitty Core Audio documentation... :X
    DisposeAUGraph(mGraph);
    [super dealloc];
}

- (void)pauseAudio
{
    DLog(@"Stopped audio");
    [self stopAudio];
}

- (void)startAudio
{
    [self createGraph];
}

- (void)stopAudio
{
    ExtAudioFileDispose(recordingFile);
    AUGraphStop(mGraph);
    AUGraphClose(mGraph);
    AUGraphUninitialize(mGraph);
}

- (void)createGraph
{
    OSStatus err;
    
    AUGraphStop(mGraph);
    AUGraphClose(mGraph);
    AUGraphUninitialize(mGraph);
    
    //Create the graph
    err = NewAUGraph(&mGraph);
    if(err) NSLog(@"NewAUGraph failed");
    
    //Open the graph
    err = AUGraphOpen(mGraph);
    if(err) NSLog(@"couldn't open graph");
    
    ComponentDescription desc;
    
    desc.componentType         = kAudioUnitType_Output;
    desc.componentSubType      = kAudioUnitSubType_DefaultOutput;
    desc.componentManufacturer = kAudioUnitManufacturer_Apple;
    desc.componentFlagsMask    = 0;
    desc.componentFlags        = 0;

    //Create the output node
    err = AUGraphAddNode(mGraph, (const AudioComponentDescription *)&desc, &mOutputNode);
    if(err) NSLog(@"couldn't create node for output unit");
    
    err = AUGraphNodeInfo(mGraph, mOutputNode, NULL, &mOutputUnit);
    if(err) NSLog(@"couldn't get output from node");
    
    
    desc.componentType = kAudioUnitType_Mixer;
    desc.componentSubType = kAudioUnitSubType_StereoMixer;
    desc.componentManufacturer = kAudioUnitManufacturer_Apple;

    //Create the mixer node
    err = AUGraphAddNode(mGraph, (const AudioComponentDescription *)&desc, &mMixerNode);
    if(err) NSLog(@"couldn't create node for file player");
    
    err = AUGraphNodeInfo(mGraph, mMixerNode, NULL, &mMixerUnit);
    if(err) NSLog(@"couldn't get player unit from node");

    desc.componentType = kAudioUnitType_FormatConverter;
    desc.componentSubType = kAudioUnitSubType_AUConverter;
    desc.componentManufacturer = kAudioUnitManufacturer_Apple;
    
    //Create the converter node
    err = AUGraphAddNode(mGraph, (const AudioComponentDescription *)&desc, &mConverterNode);
    if(err)  NSLog(@"couldn't create node for converter");
    
    err = AUGraphNodeInfo(mGraph, mConverterNode, NULL, &mConverterUnit);
    if(err) NSLog(@"couldn't get player unit from converter");
    
    
    AURenderCallbackStruct renderStruct;
    renderStruct.inputProc = RenderCallback;
    renderStruct.inputProcRefCon = gameCore;
    
    err = AudioUnitSetProperty(mConverterUnit, kAudioUnitProperty_SetRenderCallback,
                               kAudioUnitScope_Input, 0, &renderStruct, sizeof(AURenderCallbackStruct));
    if(err) DLog(@"Couldn't set the render callback");
    else DLog(@"Set the render callback");
    
    AudioStreamBasicDescription mDataFormat;
    
    mDataFormat.mSampleRate       = [gameCore frameSampleRate];
    mDataFormat.mFormatID         = kAudioFormatLinearPCM;
    mDataFormat.mFormatFlags      = kLinearPCMFormatFlagIsSignedInteger | kAudioFormatFlagsNativeEndian;
    mDataFormat.mBytesPerPacket   = 2 * [gameCore channelCount];
    mDataFormat.mFramesPerPacket  = 1; // this means each packet in the AQ has two samples, one for each channel -> 4 bytes/frame/packet
    mDataFormat.mBytesPerFrame    = 2 * [gameCore channelCount];
    mDataFormat.mChannelsPerFrame = [gameCore channelCount];
    mDataFormat.mBitsPerChannel   = 16;
    
    err = AudioUnitSetProperty(mConverterUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 0, &mDataFormat, sizeof(AudioStreamBasicDescription));
    if(err) NSLog(@"couldn't set player's input stream format");
    
    // connect the player to the output unit (stream format will propagate)
     
    err = AUGraphConnectNodeInput(mGraph, mMixerNode, 0, mOutputNode, 0);
    if(err) NSLog(@"Could not connect the input of the output");
    
    err = AUGraphConnectNodeInput(mGraph, mConverterNode, 0, mMixerNode, 0);
    if(err) NSLog(@"Couldn't connect the converter to the mixer");
    
    //AudioUnitSetParameter(mOutputUnit, kAudioUnitParameterUnit_LinearGain, kAudioUnitScope_Global, 0, [[[GameDocumentController sharedDocumentController] preferenceController] volume] ,0);
    AudioUnitSetParameter(mOutputUnit, kAudioUnitParameterUnit_LinearGain, kAudioUnitScope_Global, 0, 1.0 ,0);
    
    err = AUGraphInitialize(mGraph);
    if(err) NSLog(@"couldn't initialize graph");
    
    err = AUGraphStart(mGraph);
    if(err) NSLog(@"couldn't start graph");
	
    [self setVolume:[self volume]];
}

- (float)volume
{
    return volume;
}

- (void)setVolume:(float)aVolume
{
    volume = aVolume;
    AudioUnitSetParameter(mOutputUnit, kAudioUnitParameterUnit_LinearGain, kAudioUnitScope_Global, 0, volume, 0);
}

@end
