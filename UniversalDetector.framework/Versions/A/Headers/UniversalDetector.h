#import <Cocoa/Cocoa.h>


@interface UniversalDetector:NSObject
{
	void *detectorptr;
	NSString *charset;
	float confidence;
}

-(id)init;
-(void)dealloc;

-(void)analyzeData:(NSData *)data;
-(void)analyzeBytes:(const char *)data length:(int)len;
-(void)reset;

-(BOOL)done;
-(NSString *)MIMECharset;
-(NSStringEncoding)encoding;
-(float)confidence;

+(UniversalDetector *)detector;

@end
