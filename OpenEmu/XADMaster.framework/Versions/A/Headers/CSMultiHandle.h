#import "CSHandle.h"

#define CSMultiHandle XADMultiHandle

extern NSString *CSSizeOfSegmentUnknownException;

@interface CSMultiHandle:CSHandle
{
	NSArray *handles;
	long currhandle;
}

+(CSMultiHandle *)multiHandleWithHandleArray:(NSArray *)handlearray;
+(CSMultiHandle *)multiHandleWithHandles:(CSHandle *)firsthandle,...;

// Initializers
-(id)initWithHandles:(NSArray *)handlearray;
-(id)initAsCopyOf:(CSMultiHandle *)other;
-(void)dealloc;

// Public methods
-(NSArray *)handles;
-(CSHandle *)currentHandle;

// Implemented by this class
-(off_t)fileSize;
-(off_t)offsetInFile;
-(BOOL)atEndOfFile;

-(void)seekToFileOffset:(off_t)offs;
-(void)seekToEndOfFile;
-(int)readAtMost:(int)num toBuffer:(void *)buffer;

// Internal methods
-(void)_raiseSizeUnknownForSegment:(long)i;

@end
