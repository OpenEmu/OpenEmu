#import "CSStreamHandle.h"

#import <setjmp.h>

#define CSByteStreamHandle XADByteStreamHandle

@interface CSByteStreamHandle:CSStreamHandle
{
	uint8_t (*bytestreamproducebyte_ptr)(id,SEL,off_t);
	int bytesproduced;
	@public
	jmp_buf eofenv;
}

// Intializers
-(id)initWithName:(NSString *)descname length:(off_t)length;
-(id)initWithHandle:(CSHandle *)handle length:(off_t)length bufferSize:(int)buffersize;
-(id)initAsCopyOf:(CSByteStreamHandle *)other;

// Implemented by this class
-(int)streamAtMost:(int)num toBuffer:(void *)buffer;
-(void)resetStream;

// Implemented by subclasses
-(void)resetByteStream;
-(uint8_t)produceByteAtOffset:(off_t)pos;

// Called by subclasses
-(void)endByteStream;

@end



extern NSString *CSByteStreamEOFReachedException;

static inline void CSByteStreamEOF(CSByteStreamHandle *self) __attribute__((noreturn));
static inline void CSByteStreamEOF(CSByteStreamHandle *self) { longjmp(self->eofenv,1); }
