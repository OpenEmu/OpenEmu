#import <Foundation/Foundation.h>
#import <stdint.h>



#define CSHandleMaxLength 0x7fffffffffffffffll
#define CSHandle XADHandle



extern NSString *CSOutOfMemoryException;
extern NSString *CSEndOfFileException;
extern NSString *CSNotImplementedException;
extern NSString *CSNotSupportedException;



@interface CSHandle:NSObject <NSCopying>
{
	NSString *name;
	off_t bitoffs;
	uint8_t readbyte,readbitsleft;
	uint8_t writebyte,writebitsleft;
}

-(id)initWithName:(NSString *)descname;
-(id)initAsCopyOf:(CSHandle *)other;
-(void)dealloc;


// Methods implemented by subclasses

-(off_t)fileSize;
-(off_t)offsetInFile;
-(BOOL)atEndOfFile;
-(void)seekToFileOffset:(off_t)offs;
-(void)seekToEndOfFile;
-(void)pushBackByte:(int)byte;
-(int)readAtMost:(int)num toBuffer:(void *)buffer;
-(void)writeBytes:(int)num fromBuffer:(const void *)buffer;



// Utility methods

-(void)skipBytes:(off_t)bytes;

-(int8_t)readInt8;
-(uint8_t)readUInt8;

-(int16_t)readInt16BE;
-(int32_t)readInt32BE;
-(int64_t)readInt64BE;
-(uint16_t)readUInt16BE;
-(uint32_t)readUInt32BE;
-(uint64_t)readUInt64BE;

-(int16_t)readInt16LE;
-(int32_t)readInt32LE;
-(int64_t)readInt64LE;
-(uint16_t)readUInt16LE;
-(uint32_t)readUInt32LE;
-(uint64_t)readUInt64LE;

-(uint32_t)readID;

-(uint32_t)readBits:(int)bits;
-(uint32_t)readBitsLE:(int)bits;
-(int32_t)readSignedBits:(int)bits;
-(int32_t)readSignedBitsLE:(int)bits;
-(void)flushReadBits;

-(NSData *)readLine;
-(NSString *)readLineWithEncoding:(NSStringEncoding)encoding;
-(NSString *)readUTF8Line;

-(NSData *)fileContents;
-(NSData *)remainingFileContents;
-(NSData *)readDataOfLength:(int)length;
-(NSData *)readDataOfLengthAtMost:(int)length;
-(NSData *)copyDataOfLength:(int)length;
-(NSData *)copyDataOfLengthAtMost:(int)length;
-(void)readBytes:(int)num toBuffer:(void *)buffer;

-(off_t)readAndDiscardAtMost:(off_t)num;
-(void)readAndDiscardBytes:(off_t)num;

-(CSHandle *)subHandleOfLength:(off_t)length;
-(CSHandle *)subHandleFrom:(off_t)start length:(off_t)length;
-(CSHandle *)nonCopiedSubHandleOfLength:(off_t)length;
-(CSHandle *)nonCopiedSubHandleFrom:(off_t)start length:(off_t)length;

-(void)writeInt8:(int8_t)val;
-(void)writeUInt8:(uint8_t)val;

-(void)writeInt16BE:(int16_t)val;
-(void)writeInt32BE:(int32_t)val;
//-(void)writeInt64BE:(int64_t)val;
-(void)writeUInt16BE:(uint16_t)val;
-(void)writeUInt32BE:(uint32_t)val;
//-(void)writeUInt64BE:(uint64_t)val;

-(void)writeInt16LE:(int16_t)val;
-(void)writeInt32LE:(int32_t)val;
//-(void)writeInt64LE:(int64_t)val;
-(void)writeUInt16LE:(uint16_t)val;
-(void)writeUInt32LE:(uint32_t)val;
//-(void)writeUInt64LE:(uint64_t)val;

-(void)writeID:(uint32_t)val;

-(void)writeBits:(int)bits value:(uint32_t)val;
-(void)writeSignedBits:(int)bits value:(int32_t)val;
-(void)flushWriteBits;

-(void)writeData:(NSData *)data;

//-(void)_raiseClosed;
-(void)_raiseMemory;
-(void)_raiseEOF;
-(void)_raiseNotImplemented:(SEL)selector;
-(void)_raiseNotSupported:(SEL)selector;

-(NSString *)name;
-(NSString *)description;

-(id)copyWithZone:(NSZone *)zone;

@end

static inline int16_t CSInt16BE(const uint8_t *b) { return ((int16_t)b[0]<<8)|(int16_t)b[1]; }
static inline int32_t CSInt32BE(const uint8_t *b) { return ((int32_t)b[0]<<24)|((int32_t)b[1]<<16)|((int32_t)b[2]<<8)|(int32_t)b[3]; }
static inline int64_t CSInt64BE(const uint8_t *b) { return ((int64_t)b[0]<<56)|((int64_t)b[1]<<48)|((int64_t)b[2]<<40)|((int64_t)b[3]<<32)|((int64_t)b[4]<<24)|((int64_t)b[5]<<16)|((int64_t)b[6]<<8)|(int64_t)b[7]; }
static inline uint16_t CSUInt16BE(const uint8_t *b) { return ((uint16_t)b[0]<<8)|(uint16_t)b[1]; }
static inline uint32_t CSUInt32BE(const uint8_t *b) { return ((uint32_t)b[0]<<24)|((uint32_t)b[1]<<16)|((uint32_t)b[2]<<8)|(uint32_t)b[3]; }
static inline uint64_t CSUInt64BE(const uint8_t *b) { return ((uint64_t)b[0]<<56)|((uint64_t)b[1]<<48)|((uint64_t)b[2]<<40)|((uint64_t)b[3]<<32)|((uint64_t)b[4]<<24)|((uint64_t)b[5]<<16)|((uint64_t)b[6]<<8)|(uint64_t)b[7]; }
static inline int16_t CSInt16LE(const uint8_t *b) { return ((int16_t)b[1]<<8)|(int16_t)b[0]; }
static inline int32_t CSInt32LE(const uint8_t *b) { return ((int32_t)b[3]<<24)|((int32_t)b[2]<<16)|((int32_t)b[1]<<8)|(int32_t)b[0]; }
static inline int64_t CSInt64LE(const uint8_t *b) { return ((int64_t)b[7]<<56)|((int64_t)b[6]<<48)|((int64_t)b[5]<<40)|((int64_t)b[4]<<32)|((int64_t)b[3]<<24)|((int64_t)b[2]<<16)|((int64_t)b[1]<<8)|(int64_t)b[0]; }
static inline uint16_t CSUInt16LE(const uint8_t *b) { return ((uint16_t)b[1]<<8)|(uint16_t)b[0]; }
static inline uint32_t CSUInt32LE(const uint8_t *b) { return ((uint32_t)b[3]<<24)|((uint32_t)b[2]<<16)|((uint32_t)b[1]<<8)|(uint32_t)b[0]; }
static inline uint64_t CSUInt64LE(const uint8_t *b) { return ((uint64_t)b[7]<<56)|((uint64_t)b[6]<<48)|((uint64_t)b[5]<<40)|((uint64_t)b[4]<<32)|((uint64_t)b[3]<<24)|((uint64_t)b[2]<<16)|((uint64_t)b[1]<<8)|(uint64_t)b[0]; }

static inline void CSSetInt16BE(uint8_t *b,int16_t n) { b[0]=(n>>8)&0xff; b[1]=n&0xff; }
static inline void CSSetInt32BE(uint8_t *b,int32_t n) { b[0]=(n>>24)&0xff; b[1]=(n>>16)&0xff; b[2]=(n>>8)&0xff; b[3]=n&0xff; }
static inline void CSSetUInt16BE(uint8_t *b,uint16_t n) { b[0]=(n>>8)&0xff; b[1]=n&0xff; }
static inline void CSSetUInt32BE(uint8_t *b,uint32_t n) { b[0]=(n>>24)&0xff; b[1]=(n>>16)&0xff; b[2]=(n>>8)&0xff; b[3]=n&0xff; }
static inline void CSSetInt16LE(uint8_t *b,int16_t n) { b[1]=(n>>8)&0xff; b[0]=n&0xff; }
static inline void CSSetInt32LE(uint8_t *b,int32_t n) { b[3]=(n>>24)&0xff; b[2]=(n>>16)&0xff; b[1]=(n>>8)&0xff; b[0]=n&0xff; }
static inline void CSSetUInt16LE(uint8_t *b,uint16_t n) { b[1]=(n>>8)&0xff; b[0]=n&0xff; }
static inline void CSSetUInt32LE(uint8_t *b,uint32_t n) { b[3]=(n>>24)&0xff; b[2]=(n>>16)&0xff; b[1]=(n>>8)&0xff; b[0]=n&0xff; }

