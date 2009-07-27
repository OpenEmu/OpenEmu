#import <Foundation/Foundation.h>

@class XADStringSource,UniversalDetector;



@protocol XADString <NSObject>

-(NSString *)string;
-(NSString *)stringWithEncoding:(NSStringEncoding)encoding;
-(NSData *)data;

-(BOOL)encodingIsKnown;
-(NSStringEncoding)encoding;
-(float)confidence;

-(XADStringSource *)source;

@end



@interface XADString:NSObject <XADString>
{
	NSData *data;
	NSString *string;
	XADStringSource *source;
}

+(XADString *)XADStringWithString:(NSString *)knownstring;

-(id)initWithData:(NSData *)bytedata source:(XADStringSource *)stringsource;
-(id)initWithString:(NSString *)knownstring;
-(void)dealloc;

-(NSString *)string;
-(NSString *)stringWithEncoding:(NSStringEncoding)encoding;
-(NSData *)data;

-(BOOL)encodingIsKnown;
-(NSStringEncoding)encoding;
-(float)confidence;

-(XADStringSource *)source;

-(BOOL)isEqual:(XADString *)other;
-(unsigned)hash;
-(id)copyWithZone:(NSZone *)zone;

@end



@interface XADStringSource:NSObject
{
	UniversalDetector *detector;
	NSStringEncoding fixedencoding;
	BOOL mac;
}

-(id)init;
-(void)dealloc;

-(BOOL)analyzeDataAndCheckForASCII:(NSData *)data;

-(NSStringEncoding)encoding;
-(float)confidence;
-(UniversalDetector *)detector;

-(void)setFixedEncoding:(NSStringEncoding)encoding;
-(BOOL)hasFixedEncoding;
-(void)setPrefersMacEncodings:(BOOL)prefermac;

@end
