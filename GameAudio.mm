//
//  GameAudio.m
//  OpenEmu
//
//  Created by Josh Weinberg on 9/7/08.
//  Copyright 2008 __MyCompanyName__. All rights reserved.
//

#import "GameAudio.h"
#import "GameCore.h"
#import "GamePreferencesController.h"
//#import "GameDocumentController.h"
OSStatus RenderCallback(
								  void *							in,
								  AudioUnitRenderActionFlags *		ioActionFlags,
								  const AudioTimeStamp *			inTimeStamp,
								  UInt32							inBusNumber,
								  UInt32							inNumberFrames,
								  AudioBufferList *					ioData)
{
	RenderCallbackData * wrapper = (RenderCallbackData *)in;
	
	UInt16 *coreAudioBuffer = (UInt16*) ioData->mBuffers[0].mData;
	memset(coreAudioBuffer, 0, ioData->mBuffers[0].mDataByteSize);
	
	if(!wrapper.useRingBuffer )
	{
		[[wrapper core] requestAudio: inNumberFrames inBuffer: ioData->mBuffers[0].mData];
	}
	else
	{
		[wrapper lock];
		
		//NSLog(@"Frames: %i", inNumberFrames);
		if (inNumberFrames > 0 && ![wrapper paused]) {
			
			inTimeStamp = 0;
			if (wrapper.bufUsed < inNumberFrames * wrapper.channels)
			{
				wrapper.bufUsed = inNumberFrames * wrapper.channels;
				wrapper.bufOutPos = (wrapper.bufInPos + (wrapper.sizeSoundBuffer) - inNumberFrames * wrapper.channels) % (wrapper.sizeSoundBuffer);
			}
			// For each frame/packet (the same in our example)
			for(int i=0; i<inNumberFrames * wrapper.channels; ++i) {
				
				*coreAudioBuffer++ = wrapper.sndBuf[wrapper.bufOutPos];
				wrapper.bufOutPos = (wrapper.bufOutPos + 1) % (wrapper.sizeSoundBuffer);	
			}
			wrapper.bufUsed -= inNumberFrames * wrapper.channels;
		}
		
		[wrapper unlock];
	}
	return 0;
}

@implementation RenderCallbackData

@synthesize bufUsed, bufOutPos, bufInPos, samplesFrame, sizeSoundBuffer, channels, sampleRate, sndBuf, paused, core = _core, useRingBuffer;

// No default version for this class
- (id)init
{
    [self release];
    return nil;
}

// Designated Initializer
- (id) initWithCore:(GameCore*) core{
	
	self = [super init];
	
	if(self)
	{
		_core = core;
		sampleRate = [core frameSampleRate];
		samplesFrame = [core frameSampleCount];
		sizeSoundBuffer = [core soundBufferSize] * 8;
		channels = [core channelCount];
		
		soundLock = [NSLock new];
				
		sndBuf = new UInt16[sizeSoundBuffer];
		memset(sndBuf, 0, sizeSoundBuffer*sizeof(UInt16));

		bufInPos     = 0;
		bufOutPos    = 0;
		bufUsed      = 0;
		useRingBuffer = ![_core respondsToSelector:@selector(requestAudio:inBuffer:)];
		
	}
	
	return self;
}

- (void) lock
{
	[soundLock lock];
}

-(void) unlock
{
	[soundLock unlock];
}

- (void) dealloc
{
	delete [] sndBuf;
	[soundLock release];
	[super dealloc];
}

@end


@implementation GameAudio

- (void) pauseAudio
{
	NSLog(@"Stopped audio");
	[self stopAudio];
}

- (void) startAudio
{
//	AUGraphStart(mGraph);
	wrapper.paused = false;
	[self createGraph];
}

- (void) stopAudio
{	
	AUGraphStop(mGraph);
	AUGraphClose(mGraph);
	AUGraphUninitialize(mGraph);
}

- (void) advanceBuffer
{	
	if([wrapper useRingBuffer])
	{
		if(gameCore != NULL)
		{
			memcpy(&wrapper.sndBuf[wrapper.bufInPos], [gameCore soundBuffer], wrapper.samplesFrame * wrapper.channels * sizeof(UInt16));
		}
		
		wrapper.bufInPos = (wrapper.bufInPos + wrapper.samplesFrame * wrapper.channels) % (wrapper.sizeSoundBuffer);
		wrapper.bufUsed += wrapper.channels * wrapper.samplesFrame;
		
		if (wrapper.bufUsed > wrapper.sizeSoundBuffer)
		{
			wrapper.bufUsed   = wrapper.sizeSoundBuffer;
			wrapper.bufOutPos = wrapper.bufInPos;
		}
	}
}

- (void)createGraph
{	
	OSStatus err;
	
	
	AUGraphStop(mGraph);
	AUGraphClose(mGraph);
	AUGraphUninitialize(mGraph);
	//Create the graph
	err = NewAUGraph(&mGraph);
	if(err)
		NSLog(@"NewAUGraph failed");
	
	
	//Open the graph
	err = AUGraphOpen(mGraph);
	if(err)
		NSLog(@"couldn't open graph");

	
	
	ComponentDescription desc;
	
	desc.componentType = kAudioUnitType_Output;
	desc.componentSubType = kAudioUnitSubType_DefaultOutput;
	desc.componentManufacturer = kAudioUnitManufacturer_Apple;
    desc.componentFlagsMask = 0;
	desc.componentFlags  =0; 

	//Create the output node
	err = AUGraphAddNode(mGraph, &desc, &mOutputNode);
	if(err)
		NSLog(@"couldn't create node for output unit");
	
	err = AUGraphNodeInfo(mGraph, mOutputNode, NULL, &mOutputUnit);
	if(err)
		NSLog(@"couldn't get output from node");
	
	
	desc.componentType = kAudioUnitType_Mixer;
	desc.componentSubType = kAudioUnitSubType_StereoMixer;
	desc.componentManufacturer = kAudioUnitManufacturer_Apple;

	//Create the mixer node
	err = AUGraphAddNode(mGraph, &desc, &mMixerNode);
	if(err)
		NSLog(@"couldn't create node for file player");
	
	err = AUGraphNodeInfo(mGraph, mMixerNode, NULL, &mMixerUnit);
	if(err)
		NSLog(@"couldn't get player unit from node");

	desc.componentType = kAudioUnitType_FormatConverter;
	desc.componentSubType = kAudioUnitSubType_AUConverter;
	desc.componentManufacturer = kAudioUnitManufacturer_Apple;
	
	//Create the converter node
	err = AUGraphAddNode(mGraph, &desc, &mConverterNode);
	if(err)
		NSLog(@"couldn't create node for converter");
	
	err = AUGraphNodeInfo(mGraph, mConverterNode, NULL, &mConverterUnit);
	if(err)
		NSLog(@"couldn't get player unit from converter");
	
	
	AURenderCallbackStruct renderStruct;
	renderStruct.inputProc = RenderCallback;
	renderStruct.inputProcRefCon = wrapper;
	
	
	err = AudioUnitSetProperty(mConverterUnit, kAudioUnitProperty_SetRenderCallback,
							   kAudioUnitScope_Input, 0, &renderStruct, sizeof(AURenderCallbackStruct));
	if(err)
		NSLog(@"Couldn't set the render callback");
	else
		NSLog(@"Set the render callback");
	
	
	
	AudioStreamBasicDescription mDataFormat;
	
	mDataFormat.mSampleRate = [gameCore frameSampleRate];
	mDataFormat.mFormatID = kAudioFormatLinearPCM;
	mDataFormat.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger | kAudioFormatFlagsNativeEndian;
	mDataFormat.mBytesPerPacket = 2*[gameCore channelCount];
	mDataFormat.mFramesPerPacket = 1; // this means each packet in the AQ has two samples, one for each channel -> 4 bytes/frame/packet
	mDataFormat.mBytesPerFrame = 2*[gameCore channelCount];
	mDataFormat.mChannelsPerFrame = [gameCore channelCount];
	mDataFormat.mBitsPerChannel = 16;
	

	err = AudioUnitSetProperty(mConverterUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 0, &mDataFormat, sizeof(AudioStreamBasicDescription));//,
	if(err)
		NSLog(@"couldn't set player's input stream format");
	
	// connect the player to the output unit (stream format will propagate)
	 
	err = AUGraphConnectNodeInput(mGraph, mMixerNode, 0, mOutputNode, 0);
	if(err)
		NSLog(@"Could not connect the input of the output");
	
	err = AUGraphConnectNodeInput(mGraph, mConverterNode, 0, mMixerNode, 0);
	if(err)
		NSLog(@"Couldn't connect the converter to the mixer");
	
	//AudioUnitSetParameter(mOutputUnit, kAudioUnitParameterUnit_LinearGain, kAudioUnitScope_Global, 0, [[[GameDocumentController sharedDocumentController] preferenceController] volume] ,0);
	AudioUnitSetParameter(mOutputUnit, kAudioUnitParameterUnit_LinearGain, kAudioUnitScope_Global, 0, 1.0 ,0);
	
	err = AUGraphInitialize(mGraph);
	if(err)
		NSLog(@"couldn't initialize graph");
	
	err = AUGraphStart(mGraph);
	if(err)
		NSLog(@"couldn't start graph");
}

// No default version for this class
- (id)init
{
    [self release];
    return nil;
}

// Designated Initializer
- (id) initWithCore:(GameCore*) core
{
	self = [super init];
	
	if(self)
	{
		gameCore = core;
		wrapper = [[RenderCallbackData alloc] initWithCore: gameCore];
		[self createGraph];
	}
	
	return self;
}

- (void) dealloc
{
	[wrapper release];
	AUGraphUninitialize(mGraph);
	[super dealloc];
}


- (void) setVolume: (float) volume
{
	AudioUnitSetParameter(mOutputUnit, kAudioUnitParameterUnit_LinearGain, kAudioUnitScope_Global, 0, volume,0);
//	AUGraphUpdate(mGraph, NULL);
}


@end
