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

#import "OEBackgroundImageView.h"
#import "OETheme.h"
#import "OEThemeImage.h"
#import "OEThemeTextAttributes.h"

@interface OEBackgroundImageView ()
@property(nonatomic, assign, getter = isHovering) BOOL         hovering;
@property(nonatomic, readonly, getter = isThemed) BOOL         themed;
@property (nonatomic, readonly)                   OEThemeState stateMask;
@end

@implementation OEBackgroundImageView
- (id)initWithThemeKey:(NSString*)themeKey
{
    self = [super initWithFrame:NSZeroRect];
    if(self)
    {
        [self setThemeKey:themeKey];
    }
    return self;
}

- (void)drawRect:(NSRect)dirtyRect
{
    OEThemeState currentState = [self OE_currentState];
    
    [[[self backgroundThemeImage] imageForState:currentState] drawInRect:dirtyRect fromRect:dirtyRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:[self isFlipped] hints:nil];
    [[[self themeImage] imageForState:currentState] drawInRect:dirtyRect fromRect:dirtyRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:[self isFlipped] hints:nil];
    [[self image] drawInRect:dirtyRect fromRect:dirtyRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:[self isFlipped] hints:nil];
}

- (BOOL)isFlipped
{
    return _shouldFlipCoordinates;
}
#pragma mark - Theme
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
        [self setNeedsDisplay:YES];
        [self OE_recomputeStateMask];
    }
}

- (void)setThemeImage:(OEThemeImage *)themeImage
{
    if(_themeImage != themeImage)
    {
        // TODO: Only invalidate area of the control view
        _themeImage = themeImage;
        [self setNeedsDisplay:YES];
        [self OE_recomputeStateMask];
    }
}

- (void)setThemeTextAttributes:(OEThemeTextAttributes *)themeTextAttributes
{
    if(_themeTextAttributes != themeTextAttributes)
    {
        _themeTextAttributes = themeTextAttributes;
        [self setNeedsDisplay:YES];
        [self OE_recomputeStateMask];
    }
}

- (OEThemeState)OE_currentState
{
    // This is a convenience method that retrieves the current state of the button
    BOOL focused      = NO;
    BOOL windowActive = NO;

    if(((_stateMask & OEThemeStateAnyFocus) != 0) || ((_stateMask & OEThemeStateAnyWindowActivity) != 0))
    {
        // Set the focused, windowActive, and hover properties only if the state mask is tracking the button's focus, mouse hover, and window activity properties
        NSWindow *window = [self window];

        focused      = [window firstResponder] == self;
        windowActive = ((_stateMask & OEThemeStateAnyWindowActivity) != 0) && ([window isMainWindow] || ([window parentWindow] && [[window parentWindow] isMainWindow]));
    }

    return [OEThemeObject themeStateWithWindowActive:windowActive buttonState:NSMixedState selected:NO enabled:NO focused:focused houseHover:[self isHovering] modifierMask:[NSEvent modifierFlags]] & _stateMask;
}

- (void)OE_recomputeStateMask
{
    _themed    = (_backgroundThemeImage != nil || _themeImage != nil || _themeTextAttributes != nil);
    _stateMask = [_backgroundThemeImage stateMask] | [_themeImage stateMask] | [_themeTextAttributes stateMask];
}

@end
