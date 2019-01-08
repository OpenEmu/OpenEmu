/*
 Copyright (c) 2013, OpenEmu Team

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

#import "OETextFieldCell.h"
#import "OEControl.h"
#import "OETheme.h"
#import "OEThemeTextAttributes.h"
#import "OEThemeImage.h"

@interface OETextFieldCell ()
{
    NSMutableParagraphStyle *_style;  // Cached paragraph style used to render text
}
- (NSDictionary*)_textAttributes; // Apple Private Override
@property (nonatomic)  BOOL isEditing;
@end

@implementation OETextFieldCell

- (id)initTextCell:(NSString *)aString
{
    self = [super initTextCell:aString];
    if(self)
    {
        [self OE_commonInit];
    }
    return self;
}

- (id)initWithCoder:(NSCoder *)aDecoder
{
    self = [super initWithCoder:aDecoder];
    if(self)
    {
        [self OE_commonInit];
    }
    return self;
}

- (void)OE_commonInit
{
    [self setFocusRingType:NSFocusRingTypeNone];
    [self setDrawsBackground:NO];

    [self setIsEditing:NO];
}
#pragma mark -
- (NSDictionary*)_textAttributes
{
    return [[self themeTextAttributes] textAttributesForState:[self OE_currentState]];
}

#pragma mark - Drawing
- (void)drawInteriorWithFrame:(NSRect)cellFrame inView:(NSView *)controlView
{
    NSRect titleRect = [self titleRectForBounds:cellFrame];
    [super drawInteriorWithFrame:titleRect inView:controlView];
}

- (void)drawWithFrame:(NSRect)cellFrame inView:(NSView *)controlView
{
    if(_themed && _backgroundThemeImage)
    {
        [[_backgroundThemeImage imageForState:[self OE_currentState]] drawInRect:cellFrame fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:nil];
    }
    [self drawInteriorWithFrame:cellFrame inView:controlView];
}

#pragma mark -
- (void)editWithFrame:(NSRect)aRect inView:(NSView *)controlView editor:(NSText *)textObj delegate:(id)anObject event:(NSEvent *)theEvent
{
    NSRect titleRect = [self titleRectForBounds:aRect];
    [super editWithFrame:titleRect inView:controlView editor:textObj delegate:anObject event:theEvent];
    self.isEditing = YES;
}

- (void)selectWithFrame:(NSRect)aRect inView:(NSView *)controlView editor:(NSText *)textObj delegate:(id)anObject start:(NSInteger)selStart length:(NSInteger)selLength
{
    NSRect titleRect = [self titleRectForBounds:aRect];
    [super selectWithFrame:titleRect inView:controlView editor:textObj delegate:anObject start:selStart length:selLength];
    self.isEditing = YES;
}

- (void)endEditing:(NSText *)textObj
{
    [super endEditing:textObj];
    self.isEditing = NO;
}

- (void)setIsEditing:(BOOL)flag
{
    if(flag != _isEditing)
    {
        _isEditing = flag;
        [[self controlView] setNeedsDisplay:YES];
    }
}

- (NSTextView *)fieldEditorForView:(NSView *)aControlView
{
    return nil;
}

- (NSText*)setUpFieldEditorAttributes:(NSText *)textObj
{
    if(_themed)
    {
        NSTextView *fieldEditor = (NSTextView*)textObj;
        
        [fieldEditor setDrawsBackground:NO];
        
        OEThemeState currentState = [self OE_currentState] | OEThemeInputStateFocused;
        NSDictionary *typingAttribtues = [[self themeTextAttributes] textAttributesForState:currentState];
        [fieldEditor setTypingAttributes:typingAttribtues];

        NSDictionary *selectionAttributes = [[self selectedThemeTextAttributes] textAttributesForState:currentState];
        [fieldEditor setSelectedTextAttributes:selectionAttributes];
        [fieldEditor setInsertionPointColor:[typingAttribtues objectForKey:@"NSColor"]];
    }
    return textObj;
}
#pragma mark - Theming
@synthesize themed = _themed;
@synthesize hovering = _hovering;
@synthesize stateMask = _stateMask;
@synthesize backgroundThemeImage = _backgroundThemeImage;
@synthesize themeImage = _themeImage;
@synthesize themeTextAttributes = _themeTextAttributes, selectedThemeTextAttributes = _selectedThemeTextAttributes;

- (OEThemeState)OE_currentState
{
    // This is a convenience method that retrieves the current state of the text field
    BOOL focused      = NO;
    BOOL windowActive = NO;
    
    if(((_stateMask & OEThemeStateAnyFocus) != 0) || ((_stateMask & OEThemeStateAnyWindowActivity) != 0))
    {
        // Set the focused, windowActive, and hover properties only if the state mask is tracking the button's focus, mouse hover, and window activity properties
        NSWindow *window = [[self controlView] window];
        
        focused      = self.isEditing || [window firstResponder] == [self controlView];
        windowActive = ((_stateMask & OEThemeStateAnyWindowActivity) != 0) && ([window isMainWindow] || ([window parentWindow] && [[window parentWindow] isMainWindow]));
    }
    
    return [OEThemeObject themeStateWithWindowActive:windowActive buttonState:[self state] selected:NO enabled:[self isEnabled] focused:focused houseHover:[self isHovering] modifierMask:[NSEvent modifierFlags]] & _stateMask;
}

- (BOOL)startTrackingAt:(NSPoint)startPoint inView:(NSView *)controlView
{
    id<OEControl> control = (id<OEControl>)controlView;
    if(![control conformsToProtocol:@protocol(OEControl)] || ![control respondsToSelector:@selector(updateHoverFlagWithMousePoint:)]) return NO;
    
    [control updateHoverFlagWithMousePoint:startPoint];
    return YES;
}

- (BOOL)continueTracking:(NSPoint)lastPoint at:(NSPoint)currentPoint inView:(NSView *)controlView
{
    id<OEControl> control = (id<OEControl>)controlView;
    if(![control conformsToProtocol:@protocol(OEControl)] || ![control respondsToSelector:@selector(updateHoverFlagWithMousePoint:)]) return NO;
    
    [control updateHoverFlagWithMousePoint:currentPoint];
    return YES;
}

- (void)stopTracking:(NSPoint)lastPoint at:(NSPoint)stopPoint inView:(NSView *)controlView mouseIsUp:(BOOL)flag
{
    id<OEControl> control = (id<OEControl>)controlView;
    if(![control conformsToProtocol:@protocol(OEControl)] || ![control respondsToSelector:@selector(updateHoverFlagWithMousePoint:)]) return;
    
    [control updateHoverFlagWithMousePoint:stopPoint];
}

- (NSDictionary *)OE_attributesForState:(OEThemeState)state
{
    // This is a convenience method for creating the attributes for an NSAttributedString
    if(!_themeTextAttributes) return nil;
    if(!_style) _style = [[NSParagraphStyle defaultParagraphStyle] mutableCopy];
    
    NSDictionary *attributes = [_themeTextAttributes textAttributesForState:state];
    if(![attributes objectForKey:NSParagraphStyleAttributeName])
    {
        [_style setLineBreakMode:([self wraps] ? NSLineBreakByWordWrapping : NSLineBreakByClipping)];
        [_style setAlignment:[self alignment]];
        
        NSMutableDictionary *newAttributes = [attributes mutableCopy];
        [newAttributes setValue:_style forKey:NSParagraphStyleAttributeName];
        attributes = [newAttributes copy];
    }
    
    return attributes;
}

- (NSSize)cellSize
{
    NSSize size = [super cellSize];
    if(_themed && _themeImage)
        size.width += [self image].size.width;
    return size;
}

- (NSRect)imageRectForBounds:(NSRect)theRect
{
    NSRect result = [super imageRectForBounds:theRect];
    if(_themed && _themeImage)
    {
    }
    return result;
}

- (NSRect)titleRectForBounds:(NSRect)theRect
{
    NSRect result = [super titleRectForBounds:theRect];
    if(_themed)
    {
        NSSize titleSize = [[self attributedStringValue] size];
        result.origin.y = roundf(theRect.origin.y + (theRect.size.height - titleSize.height) / 2.0);
        
        result.origin.x += 2.0;
        result.size.width -= 4.0;
    }
    return result;
}

- (NSImage *)image
{
    return (!_themed || _themeImage == nil ? [super image] : [_themeImage imageForState:[self OE_currentState]]);
}


- (void)OE_recomputeStateMask
{
    _themed    = (_backgroundThemeImage != nil || _themeImage != nil || _themeTextAttributes != nil);
    _stateMask = [_backgroundThemeImage stateMask] | [_themeImage stateMask] | [_themeTextAttributes stateMask];
}

- (void)setThemeKey:(NSString *)key
{
    NSString *backgroundKey = key;
    if(![key hasSuffix:@"_background"])
    {
        [self setThemeImageKey:key];
        backgroundKey = [key stringByAppendingString:@"_background"];
    }
    [self setBackgroundThemeImageKey:backgroundKey];
    
    
    NSString *selectionKey = key;
    if(![key hasSuffix:@"_selection"])
    {
        [self setThemeTextAttributesKey:key];
        selectionKey = [key stringByAppendingString:@"_selection"];
    }
    [self setSelectedThemeTextAttributesKey:selectionKey];
}

- (void)setBackgroundThemeImageKey:(NSString *)key
{
    [self setBackgroundThemeImage:[[OETheme sharedTheme] themeImageForKey:key]];
}

- (void)setThemeImageKey:(NSString *)key
{
    [self setThemeImage:[[OETheme sharedTheme] themeImageForKey:key]];
}

- (void)setThemeTextAttributesKey:(NSString *)key
{
    [self setThemeTextAttributes:[[OETheme sharedTheme] themeTextAttributesForKey:key]];
}

- (void)setSelectedThemeTextAttributesKey:(NSString*)key
{
    [self setSelectedThemeTextAttributes:[[OETheme sharedTheme] themeTextAttributesForKey:key]];
}

- (void)setBackgroundThemeImage:(OEThemeImage *)backgroundThemeImage
{
    if(_backgroundThemeImage != backgroundThemeImage)
    {
        // TODO: Only invalidate area of the control view
        _backgroundThemeImage = backgroundThemeImage;
        [[self controlView] setNeedsDisplay:YES];
        [self OE_recomputeStateMask];
    }
}

- (void)setThemeImage:(OEThemeImage *)themeImage
{
    if(_themeImage != themeImage)
    {
        // TODO: Only invalidate area of the control view
        _themeImage = themeImage;
        [[self controlView] setNeedsDisplay:YES];
        [self OE_recomputeStateMask];
    }
}

- (void)setThemeTextAttributes:(OEThemeTextAttributes *)themeTextAttributes
{
    if(_themeTextAttributes != themeTextAttributes)
    {
        // TODO: Only invalidate area of the control view
        _themeTextAttributes = themeTextAttributes;
        [[self controlView] setNeedsDisplay:YES];
        [self OE_recomputeStateMask];
    }
}

@end
