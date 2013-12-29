#import <Foundation/Foundation.h>
#import "CSStreamHandle.h"

@class XADRC4Engine;

@interface XADRC4Handle:CSStreamHandle
{
	CSHandle *parent;
	off_t startoffs;
	NSData *key;
	XADRC4Engine *rc4;
}

-(id)initWithHandle:(CSHandle *)handle key:(NSData *)keydata;
-(void)dealloc;

-(void)resetStream;
-(int)streamAtMost:(int)num toBuffer:(void *)buffer;

@end

@interface XADRC4Engine:NSObject
{
	uint8_t s[256];
	int i,j;
}

+(XADRC4Engine *)engineWithKey:(NSData *)key;

-(id)initWithKey:(NSData *)key;

-(NSData *)encryptedData:(NSData *)data;

-(void)encryptBytes:(unsigned char *)bytes length:(int)length;
-(void)skipBytes:(int)length;

@end

