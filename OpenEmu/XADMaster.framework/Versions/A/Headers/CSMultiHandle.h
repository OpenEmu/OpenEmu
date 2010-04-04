#import "CSHandle.h"

#define CSMultiHandle XADMultiHandle

extern NSString *CSSizeOfSegmentUnknownException;

@interface CSMultiHandle:CSHandle
{
	NSArray *handles;
	int currhandle;
}

+(CSHandle *)multiHandleWithHandleArray:(NSArray *)handlearray;
+(CSHandle *)multiHandleWithHandles:(CSHandle *)firsthandle,...;

-(id)initWithHandles:(NSArray *)handlearray;
-(id)initAsCopyOf:(CSMultiHandle *)other;
-(void)dealloc;

-(NSArray *)handles;
-(CSHandle *)currentHandle;

-(off_t)fileSize;
-(off_t)offsetInFile;
-(BOOL)atEndOfFile;

-(void)seekToFileOffset:(off_t)offs;
-(void)seekToEndOfFile;
-(int)readAtMost:(int)num toBuffer:(void *)buffer;

-(void)_raiseSizeUnknownForSegment:(int)i;

@end
