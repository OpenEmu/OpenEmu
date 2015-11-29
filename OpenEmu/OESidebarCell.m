/*
 Copyright (c) 2011, OpenEmu Team
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
     * Neither the name of the OpenEmu Team nor the
       names of its contributors may be used to endorse or promote products
       derived from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY OpenEmu Team ''AS IS'' AND ANY
 EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL OpenEmu Team BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#import "OESidebarCell.h"
#import "OESidebarFieldEditor.h"
#import "OESidebarOutlineView.h"
#import "OETheme.h"
#import "OEThemeTextAttributes.h"
#import "OEThemeGradient.h"

const CGFloat BadgeSpacing = 2.0;

@interface NSTextFieldCell (ApplePrivate)
- (NSDictionary *)_textAttributes;
@end

@interface OESidebarCell ()
@property (nonatomic, strong) NSString *themeKey;
@property OEThemeTextAttributes *groupAttributes;
@property OEThemeTextAttributes *itemAttributes;
@property OEThemeTextAttributes *badgeAttributes;
@property OEThemeGradient *highlightGradient;
@end

@implementation OESidebarCell
@synthesize isGroup = _isGroup, isEditing=_isEditing, image=_image, badge=_badge;
@synthesize themed = _themed;
@synthesize hovering = _hovering;
@synthesize stateMask = _stateMask;
@synthesize backgroundThemeImage = _backgroundThemeImage;
@synthesize themeImage = _themeImage;
@synthesize themeTextAttributes = _themeTextAttributes;

- (id)init 
{
    if((self = [super init]))
    {
        [self setLineBreakMode:NSLineBreakByTruncatingTail];
        [self setSelectable:YES];
		
    }
    return self;
}

- (id)copyWithZone:(NSZone *)zone 
{
    OESidebarCell *cell = (OESidebarCell *)[super copyWithZone:zone];

    [cell setImage:[self image]];
    [cell setBadge:[self badge]];
    [cell setThemeKey:[self themeKey]];

    return cell;
}
#pragma mark - Theming
- (void)setThemeKey:(NSString *)key
{
    _themeKey = key;

    NSString *itemKey  = [key stringByAppendingFormat:@"_item"];
    NSString *itemBadgeKey = [key stringByAppendingString:@"_item_badge"];
    NSString *groupKey = [key stringByAppendingFormat:@"_group"];
    NSString *highlightKey = [key stringByAppendingFormat:@"_highlight"];

    OETheme *theme = [OETheme sharedTheme];
    [self setItemAttributes:[theme themeTextAttributesForKey:itemKey]];
    [self setBadgeAttributes:[theme themeTextAttributesForKey:itemBadgeKey]];
    [self setGroupAttributes:[theme themeTextAttributesForKey:groupKey]];

    [self setHighlightGradient:[theme themeGradientForKey:highlightKey]];
}

- (void)setBackgroundThemeImageKey:(NSString *)key
{
}

- (void)setThemeImageKey:(NSString *)key
{
}

- (void)setThemeTextAttributesKey:(NSString *)key
{
}

- (void)setBackgroundThemeImage:(OEThemeImage *)backgroundThemeImage
{
}

- (void)setThemeImage:(OEThemeImage *)themeImage
{
}

- (void)setThemeTextAttributes:(OEThemeTextAttributes *)themeTextAttributes
{
}

#pragma mark - Frames
- (NSRect)imageRectForBounds:(NSRect)cellFrame 
{
    NSRect result = NSZeroRect;

    if(_image != nil)
    {
		NSSize iconSize = [_image size];
		result = NSMakeRect(cellFrame.origin.x, cellFrame.origin.y+ (cellFrame.size.height-iconSize.height)/2, iconSize.width, iconSize.height);
        result.origin.y = ceil(result.origin.y);
    }

    return result;
}

- (NSRect)titleRectForBounds:(NSRect)cellFrame 
{
    NSRect result;
    if(_image != nil)
    {
        CGFloat imageWidth = [_image size].width;
        result = cellFrame;
        result.origin.x += (6.0 + imageWidth);
        result.size.width -= (6.0 + imageWidth);
		
		result.size.height = 14.0;
		result.origin.y += (cellFrame.size.height-result.size.height)/2;
	} 
    else 
    {
        result = [super titleRectForBounds:cellFrame];
    }

    NSRect badgeRect = [self badgeRectForBounds:cellFrame];
    if(!NSEqualRects(badgeRect, NSZeroRect))
    {
        result.size.width -= NSMaxX(result)-NSMinX(badgeRect);
    }

    return result;
}

- (NSRect)badgeRectForBounds:(NSRect)bounds
{
    if([self badge] == nil || [[self badge] length] == 0) return NSZeroRect;

    NSRect rect = bounds;

    CGFloat width = [[self badge] sizeWithAttributes:@{}].width;
    rect.origin.x += rect.size.width -width -2*BadgeSpacing;
    rect.size.width = width+2*BadgeSpacing;

    return rect;
}
#pragma mark - Drawing
- (void)editWithFrame:(NSRect)aRect inView:(NSView *)controlView editor:(NSText *)textObj delegate:(id)anObject event:(NSEvent *)theEvent 
{
	NSRect textFrame = [self titleRectForBounds:NSInsetRect(aRect, 0, 1)];
	textFrame.size.width -= 6.0;
	
	OESidebarFieldEditor *fieldEditor = [OESidebarFieldEditor fieldEditor];
	[fieldEditor setFrame:[textObj frame]];
    
    [super editWithFrame:textFrame inView:controlView editor:fieldEditor delegate:anObject event: theEvent];
}

- (void)selectWithFrame:(NSRect)aRect inView:(NSView *)controlView editor:(NSText *)textObj delegate:(id)anObject start:(NSInteger)selStart length:(NSInteger)selLength 
{
	NSRect textFrame = [self titleRectForBounds:aRect];
	textFrame.size.width -= 6.0;
	
	textFrame.size.height += 2;
	textFrame.origin.y -= 1;
	
	OESidebarFieldEditor *fieldEditor = [OESidebarFieldEditor fieldEditor];
	[fieldEditor setFrame:[textObj frame]];
	
    [super selectWithFrame:textFrame inView:controlView editor:fieldEditor delegate:anObject start:selStart length:selLength];
}

- (NSText *)setUpFieldEditorAttributes:(NSText *)textObj
{	
	textObj = [super setUpFieldEditorAttributes:textObj];
	
    NSFont *font = [NSFont boldSystemFontOfSize:11];
	NSColor *textColor = [NSColor blackColor];
	
	NSMutableParagraphStyle *paragraphStyle = [[NSMutableParagraphStyle alloc] init];
	[paragraphStyle setLineBreakMode:NSLineBreakByTruncatingTail];
	NSDictionary *typeAttributes = [NSDictionary dictionaryWithObjectsAndKeys:
                                    textColor, NSForegroundColorAttributeName,
                                    paragraphStyle, NSParagraphStyleAttributeName,
                                    font, NSFontAttributeName,
                                    nil];
	
	NSColor *backgroundColor = [NSColor colorWithDeviceRed:0.788 green:0.871 blue:0.992 alpha:1.0];
	NSDictionary *selectAttributes = [NSDictionary dictionaryWithObjectsAndKeys:
									  textColor, NSForegroundColorAttributeName,
									  paragraphStyle, NSParagraphStyleAttributeName,
									  font, NSFontAttributeName,
									  backgroundColor, NSBackgroundColorAttributeName,
									  nil];
	
	if([textObj isKindOfClass:[NSTextView class]])
    {
		[(NSTextView*)textObj setFocusRingType:NSFocusRingTypeNone];
		[(NSTextView*)textObj setInsertionPointColor:textColor];
        
		[(NSTextView*)textObj setTypingAttributes:typeAttributes];
		[(NSTextView*)textObj setSelectedTextAttributes:selectAttributes];
	}
	
	NSColor *color = [NSColor colorWithDeviceWhite:0.72 alpha:1.0];
	[(NSTextView *)textObj setInsertionPointColor:color];
    
	self.isEditing = YES;
	
	return textObj;
}

- (void)drawWithFrame:(NSRect)cellFrame inView:(NSView *)controlView 
{
	NSRect imageFrame = [self imageRectForBounds:cellFrame];
    if([self image] != nil)
    {
		[[self image] drawInRect:imageFrame fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:nil];
    }
	
	NSRect titleFrame = [self titleRectForBounds:cellFrame];
    NSWindow *win = [controlView window];
	BOOL isSelected = [self isHighlighted];
	BOOL isActive = [win isMainWindow] && [win firstResponder]==controlView;

    OEThemeState state = (isActive   ? OEThemeInputStateWindowActive : OEThemeInputStateWindowInactive) |
                         (isSelected ? OEThemeInputStateFocused : OEThemeInputStateUnfocused);
    NSDictionary *attributes = nil;
	if([self isGroup])
    {
        attributes = [[self groupAttributes] textAttributesForState:state];
		
		titleFrame = cellFrame;
		titleFrame.size.height -= 9;
		titleFrame.origin.y += 9;
		titleFrame.origin.x -= 8;
		titleFrame.size.width += 8;
    }
    else
    {
        attributes = [[self itemAttributes] textAttributesForState:state];
        
        // Adjust the title frame to fit the system typeface.
        titleFrame.size.height += 3;
        titleFrame.origin.y -= 2;
    }

	NSAttributedString *strVal = [[NSAttributedString alloc] initWithString:[self stringValue] attributes:attributes];
	[self setAttributedStringValue:strVal];

    [super drawWithFrame:titleFrame inView:controlView];

    NSRect badgeFrame = [self badgeRectForBounds:cellFrame];
    [self drawBadgeInFrame:badgeFrame highlighted:isSelected active:isActive];
}

- (void)drawBadgeInFrame:(NSRect)frame highlighted:(BOOL)highlighted active:(BOOL)isActive
{
    OEThemeState state = (isActive   ? OEThemeInputStateWindowActive : OEThemeInputStateWindowInactive) | (highlighted ? OEThemeInputStateFocused : OEThemeInputStateUnfocused);

    frame = NSInsetRect(frame, BadgeSpacing, BadgeSpacing);

    NSBezierPath *path = [NSBezierPath bezierPathWithRoundedRect:frame xRadius:3.0 yRadius:3.0];

    [[NSColor lightGrayColor] setFill];
    [[NSColor yellowColor] setStroke];

    [path fill];
    [path stroke];

    NSString *badge = [self badge];
    NSDictionary *attributes = [[self badgeAttributes] textAttributesForState:state];
    [badge drawInRect:frame withAttributes:attributes];
}

- (NSCellHitResult)hitTestForEvent:(NSEvent *)event inRect:(NSRect)cellFrame ofView:(NSView *)controlView
{
    NSPoint point = [controlView convertPoint:[event locationInWindow] fromView:nil];
    // If we have an image, we need to see if the user clicked on the image portion.
    if(_image != nil)
    {
        // This code closely mimics drawWithFrame:inView:
        NSRect imageFrame = [self imageRectForBounds:cellFrame];
		
		// If the point is in the image rect, then it is a content hit
        if(NSMouseInRect(point, imageFrame, [controlView isFlipped]))
        {
            // We consider this just a content area. It is not trackable, nor it it editable text. If it was, we would or in the additional items.
            // By returning the correct parts, we allow NSTableView to correctly begin an edit when the text portion is clicked on.
            return NSCellHitNone;
		}
    }
	
	if(NSMouseInRect(point, cellFrame, [controlView isFlipped]))
		return NSCellHitEditableTextArea;
	
    // At this point, the cellFrame has been modified to exclude the portion for the image. Let the superclass handle the hit testing at this point.
    return [super hitTestForEvent:event inRect:cellFrame ofView:controlView];    
}

#pragma mark - Apple Private Overrides
- (NSDictionary *)_textAttributes
{
	NSDictionary *typeAttributes = [super _textAttributes];

	if([self isEditing])
    {
        NSFont *font = [NSFont boldSystemFontOfSize:11];
        NSColor *textColor = [NSColor blackColor];
        NSMutableParagraphStyle *paragraphStyle = [[NSMutableParagraphStyle alloc] init];
        [paragraphStyle setLineBreakMode:NSLineBreakByTruncatingTail];
        typeAttributes = [NSDictionary dictionaryWithObjectsAndKeys:
                          textColor, NSForegroundColorAttributeName,
                          paragraphStyle, NSParagraphStyleAttributeName,
                          font, NSFontAttributeName,
                          nil];
	}

	return typeAttributes;
}

@end
