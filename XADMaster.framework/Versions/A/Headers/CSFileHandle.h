#import "CSHandle.h"

#import <stdio.h>

#define CSFileHandle XADFileHandle

extern NSString *CSFileErrorException;

@interface CSFileHandle:CSHandle
{
	FILE *fh;
	BOOL close;

	NSLock *multilock;
	CSFileHandle *parent;
	off_t pos;
}

+(CSFileHandle *)fileHandleForReadingAtPath:(NSString *)path;
+(CSFileHandle *)fileHandleForWritingAtPath:(NSString *)path;
+(CSFileHandle *)fileHandleForPath:(NSString *)path modes:(NSString *)modes;

-(id)initWithFilePointer:(FILE *)file closeOnDealloc:(BOOL)closeondealloc name:(NSString *)descname;
-(id)initAsCopyOf:(CSFileHandle *)other;
-(void)dealloc;

-(off_t)fileSize;
-(off_t)offsetInFile;
-(BOOL)atEndOfFile;

-(void)seekToFileOffset:(off_t)offs;
-(void)seekToEndOfFile;
-(void)pushBackByte:(int)byte;
-(int)readAtMost:(int)num toBuffer:(void *)buffer;
-(void)writeBytes:(int)num fromBuffer:(const void *)buffer;

-(void)_raiseError;
-(void)_setMultiMode;

-(FILE *)filePointer;

@end
