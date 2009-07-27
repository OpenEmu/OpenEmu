#import "CSHandle.h"

#define CSSubHandle XADSubHandle

@interface CSSubHandle:CSHandle
{
	CSHandle *parent;
	off_t start,end;
}

-(id)initWithHandle:(CSHandle *)handle from:(off_t)from length:(off_t)length;
-(id)initAsCopyOf:(CSSubHandle *)other;
-(void)dealloc;

-(off_t)fileSize;
-(off_t)offsetInFile;
-(BOOL)atEndOfFile;

-(void)seekToFileOffset:(off_t)offs;
-(void)seekToEndOfFile;
-(int)readAtMost:(int)num toBuffer:(void *)buffer;

@end
