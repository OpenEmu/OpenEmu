#import "CSStreamHandle.h"

#define CSBlockStreamHandle XADBlockStreamHandle

@interface CSBlockStreamHandle:CSStreamHandle
{
	uint8_t *currblock;
	off_t blockstartpos;
	int blocklength;
	BOOL endofblocks;
}

-(id)initWithName:(NSString *)descname length:(off_t)length;
-(id)initWithHandle:(CSHandle *)handle length:(off_t)length bufferSize:(int)buffersize;
-(id)initAsCopyOf:(CSBlockStreamHandle *)other;

-(void)setBlockPointer:(uint8_t *)blockpointer;

-(void)seekToFileOffset:(off_t)offs;

-(void)resetStream;
-(int)streamAtMost:(int)num toBuffer:(void *)buffer;

-(void)resetBlockStream;
-(int)produceBlockAtOffset:(off_t)pos;

-(void)endBlockStream;

@end
