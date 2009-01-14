//
//  GameAudio.m
//  OpenEmu
//
//  Created by Josh Weinberg on 9/7/08.
//  Copyright 2008 __MyCompanyName__. All rights reserved.
//

#import "GameAudio.h"


@implementation GameAudio
int sampleRate;
NSLock* soundLock;
UInt32 bufInPos, bufOutPos, bufUsed;
UInt16* sndBuf;
int samplesFrame;
int sizeSoundBuffer;
int channels;

id <GameCore> gameCore;

// Callback info struct -- holds the queue and the buffers and some other stuff for the callback
typedef struct AQCallbackStruct {
	AudioQueueRef					queue;
	UInt32							frameCount;
	AudioQueueBufferRef				mBuffers[AUDIOBUFFERS];
	AudioStreamBasicDescription		mDataFormat;
} AQCallbackStruct;

AQCallbackStruct in;

void AQBufferCallback(void *	in,	AudioQueueRef inQ, AudioQueueBufferRef	outQB)
{
	int i;	
	UInt32 err;
	// Get the info struct and a pointer to our output data
	AQCallbackStruct * inData = (AQCallbackStruct *)in;
	
	memset(outQB->mAudioData, 0, sizeof(outQB->mAudioData));
	UInt16 *coreAudioBuffer = (UInt16*) outQB->mAudioData;
	
	[soundLock lock];
	// if we're being asked to render
	if (inData->frameCount > 0) {
		
		if (bufUsed < inData->frameCount*channels)
		{
			bufUsed = inData->frameCount*channels;
			bufOutPos = (bufInPos + (sizeSoundBuffer) - inData->frameCount*channels) % (sizeSoundBuffer);
		}
		
		
		// Need to set this
		outQB->mAudioDataByteSize = 2*channels*inData->frameCount; // two shorts per frame, one frame per packet
		// For each frame/packet (the same in our example)
		for(i=0; i<inData->frameCount * channels; ++i) {
			
			*coreAudioBuffer++ = sndBuf[bufOutPos];
			//*coreAudioBuffer++ = sndBuf[bufOutPos+1];
			bufOutPos = (bufOutPos + 1) % (sizeSoundBuffer);
			
		}
		
		bufUsed -= inData->frameCount*channels;
		
		// "Enqueue" the buffer
		err = AudioQueueEnqueueBuffer(inQ, outQB, 0, NULL);
		if(err) fprintf(stderr, "AudioQueueEnqueueBuffer err %d\n", err);
		//		else fprintf(stdout, "Queued buffer\n");
	}
	else {
		err = AudioQueueStop(inData->queue, false);
		if(err) fprintf(stderr, "AudioQueueStop err %d\n", err);
		//		else fprintf(stdout, "Stoped queue\n");
	}
	[soundLock unlock];	
}


- (void) setupAudio
{
	UInt32 err;
	
	
	sndBuf = new UInt16[sizeSoundBuffer];
	memset(sndBuf, 0, sizeSoundBuffer*sizeof(UInt16));
	
	bufInPos     = 0;
	bufOutPos    = 0;
	bufUsed      = 0;
	
	in.mDataFormat.mSampleRate = sampleRate;
	in.mDataFormat.mFormatID = kAudioFormatLinearPCM;
	in.mDataFormat.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger | kAudioFormatFlagsNativeEndian;
	in.mDataFormat.mBytesPerPacket = 2*channels;
	in.mDataFormat.mFramesPerPacket = 1; // this means each packet in the AQ has two samples, one for each channel -> 4 bytes/frame/packet
	in.mDataFormat.mBytesPerFrame = 2*channels;
	in.mDataFormat.mChannelsPerFrame = channels;
	in.mDataFormat.mBitsPerChannel = 16;
	
	
	if(in.queue)
		err = AudioQueueDispose(in.queue,true);
	// Set up the output buffer callback on the current run loop
	err = AudioQueueNewOutput(&in.mDataFormat, AQBufferCallback, &in, NULL, kCFRunLoopCommonModes, 0, &in.queue);
	if(err) fprintf(stderr, "AudioQueueNewOutput err %d\n", err);
	
	// Set the size and packet count of each buffer read. (e.g. "frameCount")
	in.frameCount = 400;
	
	// Byte size is 4*frames (see above)
	UInt32 bufferBytes  = in.frameCount*2*channels;
	
	// alloc 3 buffers.
	for (int i=0; i<AUDIOBUFFERS; i++) {
		err = AudioQueueAllocateBuffer(in.queue, bufferBytes, &in.mBuffers[i]);
		if(err) fprintf(stderr, "AudioQueueAllocateBuffer [%d] err %d\n",i, err);
		// "Prime" by calling the callback once per buffer
		AQBufferCallback (&in, in.queue, in.mBuffers[i]);
	}	
	
//	err = AudioQueueSetParameter(in.queue, kAudioQueueParam_Volume, 1.0);
//	if(err) fprintf(stderr, "AudioQueueSetParameter err %d\n", err);
}

- (void) pauseAudio
{
	AudioQueuePause(in.queue);
}

- (void) startAudio
{
	AudioQueueStart(in.queue, NULL);	
}

- (void) stopAudio
{
	AudioQueueStop(in.queue, YES);
}

- (void) advanceBuffer
{	
	//for(int i = 0; i < SAMPLEFRAME; ++i)
	//	sndBuf[bufInPos+i] = [gameCore sndBuf][i];
	memcpy(&sndBuf[bufInPos], [gameCore sndBuf], samplesFrame*channels*sizeof(UInt16));
//	NSLog(@"Advanced");
	bufInPos = (bufInPos + samplesFrame*channels) % (sizeSoundBuffer);
	bufUsed += channels*samplesFrame;
	if (bufUsed > sizeSoundBuffer)
	{
		bufUsed   = sizeSoundBuffer;
		bufOutPos = bufInPos;
	}
}

- (id) initWithCore:(id <GameCore>) core
{
	self = [super init];
	
	if(self)
	{
		gameCore = core;
		
		sampleRate = [gameCore sampleRate];
		samplesFrame = [gameCore samplesFrame];
		sizeSoundBuffer = [gameCore sizeSoundBuffer];
		channels = [gameCore channels];
		[self setupAudio];
		
	}
	
	return self;
}

- (void) setVolume: (float) volume
{
	AudioQueueSetParameter(in.queue, kAudioQueueParam_Volume, volume);
}


@end
