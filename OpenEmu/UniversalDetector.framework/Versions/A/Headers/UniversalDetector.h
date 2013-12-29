#import <Foundation/Foundation.h>

@interface UniversalDetector:NSObject
{
	void *detector;
	NSString *charset;
	float confidence;
	const char *lastcstring;
}

+(UniversalDetector *)detector;
+(NSArray *)possibleMIMECharsets;

-(id)init;
-(void)dealloc;

-(void)analyzeData:(NSData *)data;
-(void)analyzeBytes:(const char *)data length:(int)len;
-(void)reset;

-(BOOL)done;
-(NSString *)MIMECharset;
-(float)confidence;

#ifdef __APPLE__
-(NSStringEncoding)encoding;
#endif

@end
