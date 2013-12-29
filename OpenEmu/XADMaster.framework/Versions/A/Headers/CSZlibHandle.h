#import "CSStreamHandle.h"

#ifndef __MACTYPES__
#define Byte zlibByte
#include <zlib.h>
#undef Byte
#else
#include <zlib.h>
#endif

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

// Intializers
-(id)initWithHandle:(CSHandle *)handle length:(off_t)length header:(BOOL)header name:(NSString *)descname;
-(id)initAsCopyOf:(CSZlibHandle *)other;
-(void)dealloc;

// Public methods
-(void)setSeekBackAtEOF:(BOOL)seekateof;
-(void)setEndStreamAtInputEOF:(BOOL)endateof;

// Implemented by this class
-(void)resetStream;
-(int)streamAtMost:(int)num toBuffer:(void *)buffer;

// Internal methods
-(void)_raiseZlib;

@end
