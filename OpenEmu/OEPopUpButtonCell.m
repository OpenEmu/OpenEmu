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

#import "OEPopUpButtonCell.h"

@implementation OEPopUpButtonCell
@synthesize stateMask = _stateMask;
@synthesize backgroundThemeImage = _backgroundThemeImage;
@synthesize themeImage = _themeImage;
@synthesize themeTextAttributes = _themeTextAttributes;

- (OEThemeState)OE_currentState
{
    // This is a convenience method that retrieves the current state of the button
    BOOL focused      = NO;
    BOOL windowActive = NO;
    BOOL hover        = NO;

    if((_stateMask & OEThemeStateAnyFocus) || (_stateMask & OEThemeStateAnyMouse) || (_stateMask & OEThemeStateAnyWindowActivity))
    {
        // Set the focused, windowActive, and hover properties only if the state mask is tracking the button's focus, mouse hover, and window activity properties
        NSWindow *window = [[self controlView] window];

        focused      = [window firstResponder] == [self controlView];
        windowActive = (_stateMask & OEThemeStateAnyWindowActivity) && ([window isMainWindow] || ([window parentWindow] && [[window parentWindow] isMainWindow]));

        if(_stateMask & OEThemeStateAnyMouse)
        {
            const NSPoint p = [[self controlView] convertPointFromBase:[window convertScreenToBase:[NSEvent mouseLocation]]];
            hover           = NSPointInRect(p, [[self controlView] bounds]);
        }
    }

    return [OEThemeObject themeStateWithWindowActive:windowActive buttonState:[self state] selected:[self isHighlighted] enabled:[self isEnabled] focused:focused houseHover:hover] & _stateMask;
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

- (void)drawBorderAndBackgroundWithFrame:(NSRect)cellFrame inView:(NSView *)controlView
{
    if(_themed)
    {
        if(_backgroundThemeImage == nil) return;
        [[_backgroundThemeImage imageForState:[self OE_currentState]] drawInRect:cellFrame fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:nil];
    }
    else
    {
        [super drawBorderAndBackgroundWithFrame:cellFrame inView:controlView];
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
        NSRect textRect  = [self titleRectForBounds:cellFrame];
        NSRect imageRect = [self imageRectForBounds:cellFrame];

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

- (void)setBackgroundThemeImage:(OEThemeImage *)backgroundThemeImage
{
    if(_backgroundThemeImage != backgroundThemeImage)
    {
        _backgroundThemeImage = backgroundThemeImage;
        [[self controlView] setNeedsDisplay:YES];
        [self OE_recomputeStateMask];
    }
}

- (void)setThemeImage:(OEThemeImage *)themeImage
{
    if(_themeImage != themeImage)
    {
        _themeImage = themeImage;
        [[self controlView] setNeedsDisplay:YES];
        [self OE_recomputeStateMask];
    }
}

- (void)setThemeTextAttributes:(OEThemeTextAttributes *)themeTextAttributes
{
    if(_themeTextAttributes != themeTextAttributes)
    {
        _themeTextAttributes = themeTextAttributes;
        [[self controlView] setNeedsDisplay:YES];
        [self OE_recomputeStateMask];
    }
}

@end