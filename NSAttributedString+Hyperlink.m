/*
 Taken from apple example: http://developer.apple.com/qa/qa2006/qa1487.html
 */

#import "NSAttributedString+Hyperlink.h"

@implementation NSAttributedString (Hyperlink)
+ (id)hyperlinkFromString:(NSString *)inString withURL:(NSURL *)aURL
{
    return [[[NSAttributedString alloc] initWithString:inString
                                            attributes:[NSDictionary dictionaryWithObjectsAndKeys:
                                                        [aURL absoluteString], NSLinkAttributeName,
                                                        [NSColor blueColor], NSForegroundColorAttributeName,
                                                        [NSNumber numberWithInt:NSSingleUnderlineStyle], NSUnderlineStyleAttributeName,
                                                        nil]] autorelease];
}
@end