#import "CSHandle.h"

#define CSMemoryHandle XADMemoryHandle

@interface CSMemoryHandle:CSHandle
{
	NSData *backingdata;
	off_t memorypos;
}

+(CSMemoryHandle *)memoryHandleForReadingData:(NSData *)data;
+(CSMemoryHandle *)memoryHandleForReadingBuffer:(const void *)buf length:(unsigned)len;
+(CSMemoryHandle *)memoryHandleForReadingMappedFile:(NSString *)filename;
+(CSMemoryHandle *)memoryHandleForWriting;

// Initializers
-(id)initWithData:(NSData *)dataobj;
-(id)initAsCopyOf:(CSMemoryHandle *)other;
-(void)dealloc;

// Public methods
-(NSData *)data;
-(NSMutableData *)mutableData;

// Implemented by this class
-(off_t)fileSize;
-(off_t)offsetInFile;
-(BOOL)atEndOfFile;

-(void)seekToFileOffset:(off_t)offs;
-(void)seekToEndOfFile;
//-(void)pushBackByte:(int)byte;
-(int)readAtMost:(int)num toBuffer:(void *)buffer;
-(void)writeBytes:(int)num fromBuffer:(const void *)buffer;

-(NSData *)fileContents;
-(NSData *)remainingFileContents;
-(NSData *)readDataOfLength:(int)length;
-(NSData *)readDataOfLengthAtMost:(int)length;
-(NSData *)copyDataOfLength:(int)length;
-(NSData *)copyDataOfLengthAtMost:(int)length;

@end
