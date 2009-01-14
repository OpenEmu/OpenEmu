#import <Foundation/Foundation.h>
#import <regex.h>

@interface XADRegex:NSObject
{
	regex_t preg;
}

-(id)initWithPattern:(NSString *)pattern options:(int)options;
-(void)dealloc;

-(BOOL)matchesString:(NSString *)string;
-(NSString *)matchedSubstringOfString:(NSString *)string;
-(NSArray *)capturedSubstringsOfString:(NSString *)string;

+(XADRegex *)regexWithPattern:(NSString *)pattern options:(int)options;
+(XADRegex *)regexWithPattern:(NSString *)pattern;

+(NSString *)null;

+(void)initialize;

@end

@interface NSString (XADRegex)

-(BOOL)matchedByPattern:(NSString *)pattern options:(int)options;
-(BOOL)matchedByPattern:(NSString *)pattern;

-(NSString *)substringMatchedByPattern:(NSString *)pattern options:(int)options;
-(NSString *)substringMatchedByPattern:(NSString *)pattern;

-(NSArray *)substringsCapturedByPattern:(NSString *)pattern options:(int)options;
-(NSArray *)substringsCapturedByPattern:(NSString *)pattern;

-(NSString *)escapedPattern;

@end
