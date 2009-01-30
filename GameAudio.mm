//
//  GameAudio.m
//  OpenEmu
//
//  Created by Josh Weinberg on 9/7/08.
//  Copyright 2008 __MyCompanyName__. All rights reserved.
//

#import "GameAudio.h"

void AQBufferCallback(void *	in,	AudioQueueRef inQ, AudioQueueBufferRef	outQB)
{
	int i;	
	UInt32 err;
	// Get the info struct and a pointer to our output data
	AQCallbackWrapper * wrapper = (AQCallbackWrapper *)in;
	
	memset(outQB->mAudioData, 0, sizeof(outQB->mAudioData));
	UInt16 *coreAudioBuffer = (UInt16*) outQB->mAudioData;
	
	[wrapper lock];
	// if we're being asked to render
	if (wrapper.frameCount > 0) {
		
		if (wrapper.bufUsed < wrapper.frameCount * wrapper.channels)
		{
			wrapper.bufUsed = wrapper.frameCount*wrapper.channels;
			wrapper.bufOutPos = (wrapper.bufInPos + (wrapper.sizeSoundBuffer) - wrapper.frameCount*wrapper.channels) % (wrapper.sizeSoundBuffer);
		}
		
		
		// Need to set this
		outQB->mAudioDataByteSize = 2*wrapper.channels*wrapper.frameCount; // two shorts per frame, one frame per packet
		// For each frame/packet (the same in our example)
		for(i=0; i<wrapper.frameCount * wrapper.channels; ++i) {
			
			*coreAudioBuffer++ = wrapper.sndBuf[wrapper.bufOutPos];
			//*coreAudioBuffer++ = sndBuf[bufOutPos+1];
			wrapper.bufOutPos = (wrapper.bufOutPos + 1) % (wrapper.sizeSoundBuffer);
			
		}
		
		wrapper.bufUsed -= wrapper.frameCount*wrapper.channels;
		
		// "Enqueue" the buffer
		err = AudioQueueEnqueueBuffer(inQ, outQB, 0, NULL);
		if(err) fprintf(stderr, "AudioQueueEnqueueBuffer err %d\n", err);
		//		else fprintf(stdout, "Queued buffer\n");
	}
	else {
		err = AudioQueueStop(wrapper.queue, false);
		if(err) fprintf(stderr, "AudioQueueStop err %d\n", err);
		//		else fprintf(stdout, "Stoped queue\n");
	}
	@try {
		
		[wrapper unlock];
	}
	@catch (NSException * e) {
	NSLog(@"Lock weird?");
	}		
}

@implementation AQCallbackWrapper

@synthesize bufUsed, bufOutPos, bufInPos, samplesFrame, sizeSoundBuffer, channels, sampleRate, frameCount, sndBuf, queue;

- (id) initWithCore:(id <GameCore>) core{
	
	self = [super init];
	
	if(self)
	{
		sampleRate = [core sampleRate];
		samplesFrame = [core samplesFrame];
		sizeSoundBuffer = [core sizeSoundBuffer];
		channels = [core channels];
		
		soundLock = [NSLock new];
		
		UInt32 err;
		
		sndBuf = new UInt16[sizeSoundBuffer];
		memset(sndBuf, 0, sizeSoundBuffer*sizeof(UInt16));
		
		bufInPos     = 0;
		bufOutPos    = 0;
		bufUsed      = 0;
		
		mDataFormat.mSampleRate = sampleRate;
		mDataFormat.mFormatID = kAudioFormatLinearPCM;
		mDataFormat.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger | kAudioFormatFlagsNativeEndian;
		mDataFormat.mBytesPerPacket = 2*channels;
		mDataFormat.mFramesPerPacket = 1; // this means each packet in the AQ has two samples, one for each channel -> 4 bytes/frame/packet
		mDataFormat.mBytesPerFrame = 2*channels;
		mDataFormat.mChannelsPerFrame = channels;
		mDataFormat.mBitsPerChannel = 16;
		
		queue = 0;
		
		if(queue)
			err = AudioQueueDispose(queue,true);
		
		// Set up the output buffer callback on the current run loop
		err = AudioQueueNewOutput(&mDataFormat, AQBufferCallback, self, NULL, NULL, 0, &queue);

		if(err) fprintf(stderr, "AudioQueueNewOutput err %d\n", err);
		
		// Set the size and packet count of each buffer read. (e.g. "frameCount")
		frameCount = 400;
		
		// Byte size is 4*frames (see above)
		UInt32 bufferBytes  = frameCount * 2 * channels;
		
		// alloc 3 buffers.
		for (int i=0; i<AUDIOBUFFERS; i++) {
			err = AudioQueueAllocateBuffer(queue, bufferBytes, &mBuffers[i]);
			if(err) fprintf(stderr, "AudioQueueAllocateBuffer [%d] err %d\n",i, err);
			// "Prime" by calling the callback once per buffer
			AQBufferCallback (self, queue, mBuffers[i]);
		}	
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
	[soundLock dealloc];
	AudioQueueDispose(queue, YES);
	[super dealloc];
}

@end



@implementation GameAudio

- (void) pauseAudio
{
	AudioQueuePause(wrapper.queue);
}

- (void) startAudio
{
	AudioQueueStart(wrapper.queue, NULL);	
}

- (void) stopAudio
{
	
	AudioQueueStop(wrapper.queue, YES);
	//AudioQueueDispose(in.queue, YES);
}

- (void) advanceBuffer
{	
	if(gameCore != NULL)
	{
		memcpy(&wrapper.sndBuf[wrapper.bufInPos], [gameCore sndBuf], wrapper.samplesFrame * wrapper.channels * sizeof(UInt16));
	}
	wrapper.bufInPos = (wrapper.bufInPos + wrapper.samplesFrame * wrapper.channels) % (wrapper.sizeSoundBuffer);
	wrapper.bufUsed += wrapper.channels * wrapper.samplesFrame;
	if (wrapper.bufUsed > wrapper.sizeSoundBuffer)
	{
		wrapper.bufUsed   = wrapper.sizeSoundBuffer;
		wrapper.bufOutPos = wrapper.bufInPos;
	}
}

- (id) initWithCore:(id <GameCore>) core
{
	self = [super init];
	
	if(self)
	{
		gameCore = core;
		wrapper = [[AQCallbackWrapper alloc] initWithCore: gameCore];
	}
	
	return self;
}

- (void) dealloc
{
	[wrapper dealloc];
	[super dealloc];
}


- (void) setVolume: (float) volume
{
	AudioQueueSetParameter(wrapper.queue, kAudioQueueParam_Volume, volume);
}


@end
