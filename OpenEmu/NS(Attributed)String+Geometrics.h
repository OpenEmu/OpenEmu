/*
 
 * SUMMARY
 
 The methods in these @interfaces are typically used to dynamically size
 an NSTextView or NSTextField to fit their strings.  They return the
 ^used^ size, width or height of the given string/attributes, constrained
 by the maximum dimensions passed in the 'width' and 'height' arguments. 
 
 * RENDERING IN NSTextView VS. NSTextField
 
 Text rendered in a multiline line-wrapped NSTextField leaves much more
 space between lines than text rendered in an NSTextView.  The total points
 per line is typically 10-20% higher.
 
 Because most apps use NSTextView to render multiline text, using a 
 line-wrapped NSTextField looks funny, and obviously it wastes useful
 screen area.  But there are more subtle disadavantages if you wish to 
 estimate the size of the rendered text, typically done in 
 order to size the view.
 
 First of all, you cannot get a perfect estimate of the height.
 Although using the proper typesetterBehavior in the NSLayoutManager
 providing the estimate fixes the severe 10-20% underestimation which
 you'd get from using the default NSTypesetterLatestBehavior,
 it still usually gives results that are a little inaccurate.
 The error depends on the font and the size.
 For Arial and Helvetica, the calculated height is usually underestimated
 by the measure of one glyph descender; i.e. the measurement extends
 only to the baseline of the last line.
 For Lucida Grande smaller-sized fonts (9-10 pt), and for most sizes of
 Goudy Old Style, the calculated height is overestimated, by about one line.
 For Stencil, the calculated height is accurate.
 For Zapfino, the calculated height is usually underrestimated by 1-3 pixels.
 These are the only fonts that I looked at the results for.
 
 Finally, although the typesetterBehavior seems to be, at this time,
 equal to NSTypesetterBehavior_10_2_WithCompatibility, I suppose that
 this could change in the future.  This will change the vertical
 size of the rendered text.
 
 For these reasons, using a wrapped NSTextField to render multiline text is
 therefore discouraged in favor of using an NSTextView.
 
 * THE GLOBAL VARIABLE gNSStringGeometricsTypesetterBehavior
 
 The estimate of line spacing is controlled by the NSTypesetterBehavior
 setting in NSLayoutManager used in these methods.  Therefore, you must
 specify the NSTypsetterBehavior you desire when using one of these methods
 to get a measurement.
 
 Rather than providing a 'typsetterBehavior' argument in each of the methods in
 this category, which would make them really messy just to support a
 discouraged usage, a global variable, gNSStringGeometricsTypesetterBehavior, is
 initialized with the value NSTypesetterLatestBehavior.  This value is 
 appropriate to estimating height of text to be rendered in an NSTextView.
 This is also the default behavior in NSLayoutManager.
 
 Therefore, if you want to get measurements for text to be rendered in an
 NSTextView, these methods will "just work".
 
 Also, if you want to get the dimensions for text which will render in a
 single line, even in NSTextField, these methods will "just work".
 
 However, if you want to get dimensions of a string as rendered in the
 discouraged NSTextField with line wrapping, set the global variable
 gNSStringGeometricsTypesetterBehavior to 
 NSTypesetterBehavior_10_2_WithCompatibility before invoking these methods.
 Invoking any of these methods will automatically set it back to the
 default value of NSTypesetterLatestBehavior.
 
 * ARGUMENTS width and height
 
 In the sizeFor... methods, pass either a width or height which is known to 
 be larger than the width or height that is required.  Usually, one of these
 should be the "unlimited" value of CGFLOAT_MAX.
 If text will be drawn on one line, you may pass CGFLOAT_MAX for width.
 
 * ARGUMENT attributes, NSAttributedString attributes
 
 The dictionary 'attributes', or for NSAttributedString (Geometrics), the
 attributes of the receiver, must contain at least one key:
 NSFontAttributeName, with value an NSFont object.
 Other keys in 'attributes' are ignored.
 
 * DEGENERATE ARGUMENT CASES
 
 If the receiver has 0 -length, all of these methods will return 0.0.
 If 'font' argument is nil, will log error to console and return 0.0 x 0.0.
 
 It is sometimes useful to know that, according to Douglas Davidson,
 http://www.cocoabuilder.com/archive/message/cocoa/2002/2/13/66379,
 "The default font for text that has no font attribute set is 12-pt Helvetica."
 Can't find any official documentation on this, but it seems to be still
 true today, as of Mac OS 10.5.2, for NSTextView.  For NSTextField, however,
 the default font is 12-pt Lucida Grande.  
 
 If you pass a nil 'font' argument, these methods will log an error and
 return 0.0.  But if you pass an NSAttributedString with no font attribute
 for a run, these methods will calculate assuming 12-pt regular Helvetica.

 * INTERNAL DESIGN
 
 -[NSAttributedString sizeForWidth:height:] is the primitive workhorse method.
 All other methods in these @interfaces invoke this method under the hood.
 Basically, it stuffs your string into an NSTextContainer, stuffs this into
 an NSLayout Manager, and then gets the answer by invoking
 -[NSLayoutManager usedRectForTextContainer:].  The idea is copied from here:
 http://developer.apple.com/documentation/Cocoa/Conceptual/TextLayout/Tasks/StringHeight.html 
 
 * AUTHOR
 
 Please send bug reports or other comments to Jerry Krinock, jerry@ieee.org
 Updates may be available at http://sheepsystems.com/sourceCode
 
 * ACKNOWLEDGEMENTS
 
 Thanks very much to Steve Nygard for taking the project one night,
 recognizing the importance of the line fragment padding and hyphenation
 factor, and the idea of generalizing to support NSAttributedString.
 
 */

@import Cocoa;

extern NSInteger gNSStringGeometricsTypesetterBehavior ;

@interface NSAttributedString (Geometrics) 

// Measuring Attributed Strings
- (NSSize)sizeForWidth:(CGFloat)width
				height:(CGFloat)height ;
- (CGFloat)heightForWidth:(CGFloat)width ;
- (CGFloat)widthForHeight:(CGFloat)height ;

@end

@interface NSString (Geometrics)

// Measuring a String With Attributes
- (NSSize)sizeForWidth:(CGFloat)width
				height:(CGFloat)height
			attributes:(NSDictionary*)attributes ;
- (CGFloat)heightForWidth:(CGFloat)width
               attributes:(NSDictionary*)attributes ;
- (CGFloat)widthForHeight:(CGFloat)height
               attributes:(NSDictionary*)attributes ;

// Measuring a String with a constant Font
- (NSSize)sizeForWidth:(CGFloat)width
				height:(CGFloat)height
				  font:(NSFont*)font ;
- (CGFloat)heightForWidth:(CGFloat)width
                     font:(NSFont*)font ;
- (CGFloat)widthForHeight:(CGFloat)height
                     font:(NSFont*)font ;

@end
