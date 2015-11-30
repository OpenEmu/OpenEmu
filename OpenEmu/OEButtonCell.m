/*
 Copyright (c) 2012, OpenEmu Team

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

#import "OEButtonCell.h"
#import "OEControl.h"
#import "OETheme.h"
#import "OEThemeTextAttributes.h"
#import "OEThemeImage.h"

@implementation OEButtonCell
@synthesize themed = _themed;
@synthesize hovering = _hovering;
@synthesize stateMask = _stateMask;
@synthesize backgroundThemeImage = _backgroundThemeImage;
@synthesize themeImage = _themeImage;
@synthesize themeTextAttributes = _themeTextAttributes;

- (void)awakeFromNib
{
    [self setBezelStyle:0];
}

- (OEThemeState)OE_currentState
{
    // This is a convenience method that retrieves the current state of the button
    BOOL focused      = NO;
    BOOL windowActive = NO;

    if(((_stateMask & OEThemeStateAnyFocus) != 0) || ((_stateMask & OEThemeStateAnyWindowActivity) != 0))
    {
        // Set the focused, windowActive, and hover properties only if the state mask is tracking the button's focus, mouse hover, and window activity properties
        NSWindow *window = [[self controlView] window];

        focused      = [window firstResponder] == [self controlView] || ([window firstResponder] && [[self controlView] respondsToSelector:@selector(currentEditor)] && [window firstResponder]==[(NSControl*)[self controlView] currentEditor]);
        windowActive = ((_stateMask & OEThemeStateAnyWindowActivity) != 0) && ([window isMainWindow] || ([window parentWindow] && [[window parentWindow] isMainWindow]));
    }
    return [OEThemeObject themeStateWithWindowActive:windowActive buttonState:[self state] selected:[self isHighlighted] enabled:[self isEnabled] focused:focused houseHover:[self isHovering] modifierMask:[NSEvent modifierFlags]] & _stateMask;
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
        NSButtonType buttonType = [[self valueForKey:@"buttonType"] unsignedIntegerValue];
        switch(buttonType)
        {
            case NSSwitchButton:
            {
                NSSize imageSize = [self image].size;
                result.origin.y = NSMinY(result) + (NSHeight(result) - imageSize.height) / 2.0;
                result.size = imageSize;
                break;
            }
            default:
                if(NSIsEmptyRect(result))
                {
                    NSSize imageSize = [self image].size;
                    result.size = imageSize;
                    
                    switch ([self imagePosition]) {
                        // TODO: Take other imagePositions and imageScaling into account
                        case NSImageRight:
                            result.origin.x = NSMaxX(theRect) - imageSize.width;
                            result.origin.y = NSMinY(theRect) + (NSHeight(theRect)-imageSize.height) / 2.0;
                            break;
                        default:
                            result.origin.x = NSMinX(theRect) + (NSWidth(theRect)-imageSize.width) / 2.0;
                            result.origin.y = NSMinY(theRect) + (NSHeight(theRect)-imageSize.height) / 2.0;
                            break;
                    }
                }
                else
                    result.origin.x -= 2;
                break;
        }
    }


    if([self controlView])
        return [[self controlView] backingAlignedRect:result options:NSAlignAllEdgesNearest];
    else
        return result;
}

- (NSRect)titleRectForBounds:(NSRect)theRect
{
    NSRect result = [super titleRectForBounds:theRect];

    if(_themed)
    {
        NSButtonType buttonType = [[self valueForKey:@"buttonType"] unsignedIntegerValue];
        switch(buttonType)
        {
            case NSRadioButton:
            case NSSwitchButton:
                result = NSInsetRect(result, 3.0, 0.0);
                result.origin.y += 1.0;
                break;
            default:
                if([self isHighlighted] && [self isBordered])
                {
                    result.origin.x -= 1.0;
                    result.origin.y -= 1.0;
                }
                
                if(_themeImage)
                {
                    NSSize imageSize = [self image].size;
                    switch ([self imagePosition]) {
                    // TODO: Take other imagePositions and imageScaling into account
                        case NSImageRight:
                            result.size.width -= imageSize.width;
                            result.origin.y += 1;
                            break;
                        default:
                            break;
                    }
                }
                break;
        }
    }
    return result;
}

- (void)drawBezelWithFrame:(NSRect)frame inView:(NSView *)controlView
{
    if(_themed)
    {
        if(_backgroundThemeImage == nil) return;
        [[_backgroundThemeImage imageForState:[self OE_currentState]] drawInRect:frame fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:nil];
    }
    else
    {
        [super drawBezelWithFrame:frame inView:controlView];
    }
}

- (void)drawImage:(NSImage *)image withFrame:(NSRect)frame inView:(NSView *)controlView
{
    if(_themed)
    {
        [image drawInRect:frame fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:nil];
    }
    else
    {
        [super drawImage:image withFrame:frame inView:controlView];
    }
}

- (void)drawInteriorWithFrame:(NSRect)cellFrame inView:(NSView *)controlView
{
    if(_themed)
    {
        NSRect textRect  = NSIntegralRect([self titleRectForBounds:cellFrame]);
        NSRect imageRect = NSIntegralRect([self imageRectForBounds:cellFrame]);
        
        if(!NSIsEmptyRect(textRect))  [self drawTitle:[self attributedTitle] withFrame:textRect inView:controlView];
        if(!NSIsEmptyRect(imageRect)) [self drawImage:[self image] withFrame:imageRect inView:controlView];
    }
    else
    {
        [super drawInteriorWithFrame:cellFrame inView:controlView];
    }
}

- (NSImage *)image
{
    return (!_themed || _themeImage == nil ? [super image] : [_themeImage imageForState:[self OE_currentState]]);
}

- (NSAttributedString *)attributedTitle
{
    NSDictionary *attributes = (_themed ? [self OE_attributesForState:[self OE_currentState]] : nil);
    return (!attributes ? [super attributedTitle] : [[NSAttributedString alloc] initWithString:[self title] attributes:attributes]);
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
    [self setThemeTextAttributesKey:key];
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
