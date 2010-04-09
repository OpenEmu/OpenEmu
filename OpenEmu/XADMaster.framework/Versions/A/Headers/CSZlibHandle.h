#import "CSStreamHandle.h"

#include <zlib.h>

#define CSZlibHandle XADZlibHandle

extern NSString *CSZlibException;

@interface CSZlibHandle:CSStreamHandle
{
	CSHandle *parent;
	off_t startoffs;
	z_stream zs;
	BOOL inited,seekback,endstreamateof;

	uint8_t inbuffer[0x4000];
}

+(CSZlibHandle *)zlibHandleWithHandle:(CSHandle *)handle;
+(CSZlibHandle *)zlibHandleWithHandle:(CSHandle *)handle length:(off_t)length;
+(CSZlibHandle *)deflateHandleWithHandle:(CSHandle *)handle;
+(CSZlibHandle *)deflateHandleWithHandle:(CSHandle *)handle length:(off_t)length;

-(id)initWithHandle:(CSHandle *)handle length:(off_t)length header:(BOOL)header name:(NSString *)descname;
-(id)initAsCopyOf:(CSZlibHandle *)other;
-(void)dealloc;

-(void)setSeekBackAtEOF:(BOOL)seekateof;
-(void)setEndStreamAtInputEOF:(BOOL)endateof;

-(void)resetStream;
-(int)streamAtMost:(int)num toBuffer:(void *)buffer;

-(void)_raiseZlib;

@end
