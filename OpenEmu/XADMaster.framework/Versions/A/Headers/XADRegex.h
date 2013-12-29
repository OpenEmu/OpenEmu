#import <Foundation/Foundation.h>

#ifdef _WIN32
#import "regex.h"
#else
#import <regex.h>
#endif

@interface XADRegex:NSObject
{
	NSString *patternstring;
	regex_t preg;
	regmatch_t *matches;
	NSRange matchrange;
	NSData *currdata;
}

+(XADRegex *)regexWithPattern:(NSString *)pattern options:(int)options;
+(XADRegex *)regexWithPattern:(NSString *)pattern;

+(NSString *)patternForLiteralString:(NSString *)string;
+(NSString *)patternForGlob:(NSString *)glob;

+(NSString *)null;

-(id)initWithPattern:(NSString *)pattern options:(int)options;
-(void)dealloc;

-(void)beginMatchingString:(NSString *)string;
//-(void)beginMatchingString:(NSString *)string range:(NSRange)range;
-(void)beginMatchingData:(NSData *)data;
-(void)beginMatchingData:(NSData *)data range:(NSRange)range;
-(void)finishMatching;
-(BOOL)matchNext;
-(NSString *)stringForMatch:(int)n;
-(NSArray *)allMatches;

-(BOOL)matchesString:(NSString *)string;
-(NSString *)matchedSubstringOfString:(NSString *)string;
-(NSArray *)capturedSubstringsOfString:(NSString *)string;
-(NSArray *)allMatchedSubstringsOfString:(NSString *)string;
-(NSArray *)allCapturedSubstringsOfString:(NSString *)string;
-(NSArray *)componentsOfSeparatedString:(NSString *)string;

/*
-(NSString *)expandReplacementString:(NSString *)replacement;
*/

-(NSString *)pattern;
-(NSString *)description;

@end

@interface NSString (XADRegex)

-(BOOL)matchedByPattern:(NSString *)pattern;
-(BOOL)matchedByPattern:(NSString *)pattern options:(int)options;

-(NSString *)substringMatchedByPattern:(NSString *)pattern;
-(NSString *)substringMatchedByPattern:(NSString *)pattern options:(int)options;

-(NSArray *)substringsCapturedByPattern:(NSString *)pattern;
-(NSArray *)substringsCapturedByPattern:(NSString *)pattern options:(int)options;

-(NSArray *)allSubstringsMatchedByPattern:(NSString *)pattern;
-(NSArray *)allSubstringsMatchedByPattern:(NSString *)pattern options:(int)options;

-(NSArray *)allSubstringsCapturedByPattern:(NSString *)pattern;
-(NSArray *)allSubstringsCapturedByPattern:(NSString *)pattern options:(int)options;

-(NSArray *)componentsSeparatedByPattern:(NSString *)pattern;
-(NSArray *)componentsSeparatedByPattern:(NSString *)pattern options:(int)options;

-(NSString *)escapedPattern;

@end

/*@interface NSMutableString (XADRegex)

-(void)replacePattern:(NSString *)pattern with:(NSString *)replacement;
-(void)replacePattern:(NSString *)pattern with:(NSString *)replacement options:(int)options;
-(void)replacePattern:(NSString *)pattern usingSelector:(SEL)selector onObject:(id)object;
-(void)replacePattern:(NSString *)pattern usingSelector:(SEL)selector onObject:(id)object options:(int)options;
-(void)replaceEveryPattern:(NSString *)pattern with:(NSString *)replacement;
-(void)replaceEveryPattern:(NSString *)pattern with:(NSString *)replacement options:(int)options;
-(void)replaceEveryPattern:(NSString *)pattern usingSelector:(SEL)selector onObject:(id)object;
-(void)replaceEveryPattern:(NSString *)pattern usingSelector:(SEL)selector onObject:(id)object options:(int)options;

@end*/