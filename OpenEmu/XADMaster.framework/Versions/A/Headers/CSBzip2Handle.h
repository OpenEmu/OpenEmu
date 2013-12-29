#import "CSStreamHandle.h"

#include <bzlib.h>

#define CSBzip2Handle XADBzip2Handle

extern NSString *CSBzip2Exception;

@interface CSBzip2Handle:CSStreamHandle
{
	CSHandle *parent;
	off_t startoffs;
	bz_stream bzs;
	BOOL inited,checksumcorrect;

	uint8_t inbuffer[16*1024];
}

+(CSBzip2Handle *)bzip2HandleWithHandle:(CSHandle *)handle;
+(CSBzip2Handle *)bzip2HandleWithHandle:(CSHandle *)handle length:(off_t)length;

// Initializers.
-(id)initWithHandle:(CSHandle *)handle length:(off_t)length name:(NSString *)descname;
-(void)dealloc;

// Implemented by this class.
-(void)resetStream;
-(int)streamAtMost:(int)num toBuffer:(void *)buffer;

// Checksum functions for XADMaster.
-(BOOL)hasChecksum;
-(BOOL)isChecksumCorrect;

// Internal methods.
-(void)_raiseBzip2:(int)error;

@end
