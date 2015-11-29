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

#import "OESliderCell.h"
#import "OEControl.h"
#import "OETheme.h"
#import "OEThemeImage.h"

#import "OpenEmu-Swift.h"

@implementation OESliderCell

- (CGFloat)trackThickness
{
    NSImage *trackImage = [[self backgroundThemeImage] imageForState:OEThemeStateDefault];
    return [trackImage size].height;
}

#pragma mark - Drawing
- (void)drawBarInside:(NSRect)aRect flipped:(BOOL)flipped
{
    OEThemeState state  = [self OE_currentState];
    NSImage *trackImage = [[self backgroundThemeImage] imageForState:state];
    NSRect barRect = NSInsetRect(aRect, 1, (NSHeight(aRect)-[self trackThickness])/2.0);
    barRect = [[self controlView] backingAlignedRect:barRect options:NSAlignAllEdgesNearest];
    [trackImage drawInRect:barRect fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:@{NSImageHintInterpolation:@(NSImageInterpolationNone)}];

    NSRect knobRect = [self knobRectFlipped:flipped];
    NSImage *levelImage = [[self levelThemeImage] imageForState:state];
    if(levelImage != nil)
    {
        NSRect levelRect = barRect;
        levelRect.size.width = MAX(NSMidX(knobRect)-NSMinX(barRect), [levelImage size].width);
        levelRect = [[self controlView] backingAlignedRect:levelRect options:NSAlignAllEdgesNearest];
        [levelImage drawInRect:levelRect fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:@{NSImageHintInterpolation:@(NSImageInterpolationNone)}];
    }
}

- (void)drawKnob:(NSRect)knobRect
{
    OEThemeState state  = [self OE_currentState];
    NSImage *knobImage = [[self themeImage] imageForState:state];
    knobRect.origin.y   += NSHeight(knobRect) - [knobImage size].height;
    knobRect.origin.x   += (NSWidth(knobRect) - [knobImage size].width)/2;
    knobRect.size.width  = [knobImage size].width;
    knobRect.size.height = [knobImage size].height;
    knobRect = [[self controlView] backingAlignedRect:knobRect options:NSAlignAllEdgesNearest];
    [knobImage drawInRect:knobRect fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:@{NSImageHintInterpolation:@(NSImageInterpolationNone)}];
}

#pragma mark - State
@synthesize themed = _themed;
@synthesize hovering = _hovering;
@synthesize stateMask = _stateMask;
@synthesize backgroundThemeImage = _backgroundThemeImage;
@synthesize themeImage = _themeImage;
@synthesize levelThemeImage = _levelThemeImage;

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

#pragma mark - Tracking
- (BOOL)startTrackingAt:(NSPoint)startPoint inView:(NSView *)controlView
{
    id<OEControl> control = (id<OEControl>)controlView;
    if(![control conformsToProtocol:@protocol(OEControl)] || ![control respondsToSelector:@selector(updateHoverFlagWithMousePoint:)]) return NO;

    [control updateHoverFlagWithMousePoint:startPoint];
    return [super startTrackingAt:startPoint inView:controlView];
}

- (BOOL)continueTracking:(NSPoint)lastPoint at:(NSPoint)currentPoint inView:(NSView *)controlView
{
    id<OEControl> control = (id<OEControl>)controlView;
    if(![control conformsToProtocol:@protocol(OEControl)] || ![control respondsToSelector:@selector(updateHoverFlagWithMousePoint:)]) return NO;

    [control updateHoverFlagWithMousePoint:currentPoint];
    BOOL ret = [super continueTracking:lastPoint at:currentPoint inView:controlView];
    if([self levelThemeImage] != nil)
        [[self controlView] setNeedsDisplayInRect:[[self controlView] bounds]];
    return ret;
}

- (void)stopTracking:(NSPoint)lastPoint at:(NSPoint)stopPoint inView:(NSView *)controlView mouseIsUp:(BOOL)flag
{
    id<OEControl> control = (id<OEControl>)controlView;
    if(![control conformsToProtocol:@protocol(OEControl)] || ![control respondsToSelector:@selector(updateHoverFlagWithMousePoint:)]) return;

    [control updateHoverFlagWithMousePoint:stopPoint];
    [super stopTracking:lastPoint at:stopPoint inView:controlView mouseIsUp:flag];
}

- (void)OE_recomputeStateMask
{
    _themed    = (_backgroundThemeImage != nil || _themeImage != nil || _levelThemeImage != nil);
    _stateMask = [_backgroundThemeImage stateMask] | [_themeImage stateMask] | [_levelThemeImage stateMask] ;
}

#pragma mark - Theming
- (void)setThemeKey:(NSString *)key
{
    NSString *backgroundKey = [key stringByAppendingString:@"_background"];
    NSString *levelKey      = [key stringByAppendingString:@"_level"];

    [self setThemeImageKey:key];
    [self setBackgroundThemeImageKey:backgroundKey];
    [self setLevelThemeImageKey:levelKey];
}

- (void)setBackgroundThemeImageKey:(NSString *)key
{
    [self setBackgroundThemeImage:[[OETheme sharedTheme] themeImageForKey:key]];
}

- (void)setThemeImageKey:(NSString *)key
{
    OEThemeImage *image = [[OETheme sharedTheme] themeImageForKey:key];
    [self setThemeImage:image];
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

- (void)setLevelThemeImageKey:(NSString *)key
{
    [self setLevelThemeImage:[[OETheme sharedTheme] themeImageForKey:key]];
}

- (void)setLevelThemeImage:(OEThemeImage *)levelThemeImage
{
    if(_levelThemeImage != levelThemeImage)
    {
        _levelThemeImage = levelThemeImage;
        [[self controlView] setNeedsDisplay:YES];
        [self OE_recomputeStateMask];
    }
}
#pragma mark - Apple Private
// Apple private method that we override
- (BOOL)_usesCustomTrackImage
{
    return YES;
}

#pragma mark - unused
- (void)setThemeTextAttributesKey:(NSString *)key{}
@synthesize themeTextAttributes=_themeTextAttributes;
@end
