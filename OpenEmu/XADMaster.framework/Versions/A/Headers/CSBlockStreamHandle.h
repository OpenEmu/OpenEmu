#import "CSStreamHandle.h"

#define CSBlockStreamHandle XADBlockStreamHandle

@interface CSBlockStreamHandle:CSStreamHandle
{
	uint8_t *_currblock;
	off_t _blockstartpos;
	int _blocklength;
	BOOL _endofblocks;
}

// Intializers
-(id)initWithName:(NSString *)descname length:(off_t)length;
-(id)initWithHandle:(CSHandle *)handle length:(off_t)length bufferSize:(int)buffersize;
-(id)initAsCopyOf:(CSBlockStreamHandle *)other;

// Implemented by this class
-(void)seekToFileOffset:(off_t)offs;
-(void)resetStream;
-(int)streamAtMost:(int)num toBuffer:(void *)buffer;

// Internal methods
-(void)_readNextBlock;

// Implemented by subclasses
-(void)resetBlockStream;
-(int)produceBlockAtOffset:(off_t)pos;

// Called by subclasses
-(void)setBlockPointer:(uint8_t *)blockpointer;
-(void)endBlockStream;

@end
