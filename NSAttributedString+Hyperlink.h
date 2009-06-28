/*
 Taken from apple example: http://developer.apple.com/qa/qa2006/qa1487.html
 */

@interface NSAttributedString (Hyperlink)
+(id)hyperlinkFromString:(NSString*)inString withURL:(NSURL*)aURL;
@end
