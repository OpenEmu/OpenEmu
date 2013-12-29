#import "CSHandle.h"
#import "CSInputBuffer.h"

#define CSStreamHandle XADStreamHandle

@interface CSStreamHandle:CSHandle
{
	off_t streampos,streamlength;
	BOOL needsreset,endofstream;
	int nextstreambyte;

	@public
	CSInputBuffer *input;
}

// Initializers
-(id)initWithName:(NSString *)descname;
-(id)initWithName:(NSString *)descname length:(off_t)length;
-(id)initWithHandle:(CSHandle *)handle;
-(id)initWithHandle:(CSHandle *)handle length:(off_t)length;
-(id)initWithHandle:(CSHandle *)handle bufferSize:(int)buffersize;
-(id)initWithHandle:(CSHandle *)handle length:(off_t)length bufferSize:(int)buffersize;
-(id)initAsCopyOf:(CSStreamHandle *)other;
-(void)dealloc;

// Implemented by this class
-(off_t)fileSize;
-(off_t)offsetInFile;
-(BOOL)atEndOfFile;
-(void)seekToFileOffset:(off_t)offs;
-(void)seekToEndOfFile;
-(int)readAtMost:(int)num toBuffer:(void *)buffer;

// Implemented by subclasses
-(void)resetStream;
-(int)streamAtMost:(int)num toBuffer:(void *)buffer;

// Called by subclasses
-(void)endStream;
-(BOOL)_prepareStreamSeekTo:(off_t)offs;
-(void)setStreamLength:(off_t)length;
-(void)setInputBuffer:(CSInputBuffer *)inputbuffer;

@end
