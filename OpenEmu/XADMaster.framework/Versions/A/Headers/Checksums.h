#import "CSHandle.h"
#import "CSSubHandle.h"
#import "CSStreamHandle.h"

@interface CSHandle (Checksums)

-(BOOL)hasChecksum;
-(BOOL)isChecksumCorrect;

@end

@interface CSSubHandle (Checksums)

-(BOOL)hasChecksum;
-(BOOL)isChecksumCorrect;

@end

@interface CSStreamHandle (Checksums)

-(BOOL)hasChecksum;
-(BOOL)isChecksumCorrect;

@end

#define CSChecksumWrapperHandle XADChecksumWrapperHandle

@interface CSChecksumWrapperHandle:CSHandle
{
	CSHandle *parent,*checksum;
}

-(id)initWithHandle:(CSHandle *)handle checksumHandle:(CSHandle *)checksumhandle;
-(void)dealloc;

-(off_t)fileSize;
-(off_t)offsetInFile;
-(BOOL)atEndOfFile;
-(void)seekToFileOffset:(off_t)offs;
-(void)seekToEndOfFile;
-(void)pushBackByte:(int)byte;
-(int)readAtMost:(int)num toBuffer:(void *)buffer;
-(void)writeBytes:(int)num fromBuffer:(const void *)buffer;

-(BOOL)hasChecksum;
-(BOOL)isChecksumCorrect;

@end
