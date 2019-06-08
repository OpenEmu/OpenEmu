/*
	<samplecode>
		<abstract>
			Utility classes to manage audio formats and buffers for an audio unit implementation's input and output audio busses.
		</abstract>
	</samplecode>
*/

#import <AudioToolbox/AudioToolbox.h>
#import <AudioUnit/AudioUnit.h>
#import <AVFoundation/AVFoundation.h>

#pragma mark BufferedAudioBus Utility Class
// Utility classes to manage audio formats and buffers for an audio unit implementation's input and output audio busses.

// Reusable non-ObjC class, accessible from render thread.
struct BufferedAudioBus {
	AUAudioUnitBus* bus = nullptr;
	AUAudioFrameCount maxFrames = 0;
    
	AVAudioPCMBuffer* pcmBuffer = nullptr;
    
	AudioBufferList const* originalAudioBufferList = nullptr;
	AudioBufferList* mutableAudioBufferList = nullptr;

	void init(AVAudioFormat* defaultFormat, AVAudioChannelCount maxChannels) {
		maxFrames = 0;
		pcmBuffer = nullptr;
		originalAudioBufferList = nullptr;
		mutableAudioBufferList = nullptr;
		
        bus = [[AUAudioUnitBus alloc] initWithFormat:defaultFormat error:nil];

        bus.maximumChannelCount = maxChannels;
	}
	
	void allocateRenderResources(AUAudioFrameCount inMaxFrames) {
		maxFrames = inMaxFrames;
		
		pcmBuffer = [[AVAudioPCMBuffer alloc] initWithPCMFormat:bus.format frameCapacity: maxFrames];
		
        originalAudioBufferList = pcmBuffer.audioBufferList;
        mutableAudioBufferList = pcmBuffer.mutableAudioBufferList;
	}
    
	void deallocateRenderResources() {
		pcmBuffer = nullptr;
		originalAudioBufferList = nullptr;
		mutableAudioBufferList = nullptr;
	}
};

#pragma mark - BufferedOutputBus: BufferedAudioBus
#pragma mark prepareOutputBufferList()
/*
    BufferedOutputBus
 
	This class provides a prepareOutputBufferList method to copy the internal buffer pointers
	to the output buffer list in case the client passed in null buffer pointers.
 */
struct BufferedOutputBus: BufferedAudioBus {
	void prepareOutputBufferList(AudioBufferList* outBufferList, AVAudioFrameCount frameCount, bool zeroFill) {
		UInt32 byteSize = frameCount * sizeof(float);
		for (UInt32 i = 0; i < outBufferList->mNumberBuffers; ++i) {
			outBufferList->mBuffers[i].mNumberChannels = originalAudioBufferList->mBuffers[i].mNumberChannels;
			outBufferList->mBuffers[i].mDataByteSize = byteSize;
			if (outBufferList->mBuffers[i].mData == nullptr) {
				outBufferList->mBuffers[i].mData = originalAudioBufferList->mBuffers[i].mData;
			}
			if (zeroFill) {
				memset(outBufferList->mBuffers[i].mData, 0, byteSize);
			}
		}
	}
};

#pragma mark - BufferedInputBus: BufferedAudioBus
#pragma mark pullInput()
#pragma mark prepareInputBufferList()
/*
	BufferedInputBus
 
	This class manages a buffer into which an audio unit with input busses can
    pull its input data.
 */
struct BufferedInputBus : BufferedAudioBus {
	/*
        Gets input data for this input by preparing the input buffer list and pulling
        the pullInputBlock.
    */
	AUAudioUnitStatus pullInput(AudioUnitRenderActionFlags *actionFlags,
								AudioTimeStamp const* timestamp,
								AVAudioFrameCount frameCount,
								NSInteger inputBusNumber,
								AURenderPullInputBlock pullInputBlock) {
        if (pullInputBlock == NULL) {
			return kAudioUnitErr_NoConnection;
		}
        
        /*
         Important:
             The Audio Unit must supply valid buffers in (inputData->mBuffers[x].mData) and mDataByteSize.
             mDataByteSize must be consistent with frameCount.
             
             The AURenderPullInputBlock may provide input in those specified buffers, or it may replace
             the mData pointers with pointers to memory which it owns and guarantees will remain valid
             until the next render cycle.
             
             See prepareInputBufferList()
        */
		
		prepareInputBufferList();
		
		return pullInputBlock(actionFlags, timestamp, frameCount, inputBusNumber, mutableAudioBufferList);
	}
    
    /*
        prepareInputBufferList populates the mutableAudioBufferList with the data
        pointers from the originalAudioBufferList.
     
        The upstream audio unit may overwrite these with its own pointers, so each
        render cycle this function needs to be called to reset them.
     */
    void prepareInputBufferList() {
        UInt32 byteSize = maxFrames * sizeof(float);
		
        mutableAudioBufferList->mNumberBuffers = originalAudioBufferList->mNumberBuffers;
		
        for (UInt32 i = 0; i < originalAudioBufferList->mNumberBuffers; ++i) {
            mutableAudioBufferList->mBuffers[i].mNumberChannels = originalAudioBufferList->mBuffers[i].mNumberChannels;
            mutableAudioBufferList->mBuffers[i].mData = originalAudioBufferList->mBuffers[i].mData;
            mutableAudioBufferList->mBuffers[i].mDataByteSize = byteSize;
        }
    }
};
