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

#import "OEHorizontalLine.h"
#import "OETheme.h"
#import "OEThemeObject.h"
#import "OEThemeGradient.h"
#import "OEThemeImage.h"

@interface OEHorizontalLine ()
@property (strong) OEThemeGradient *lineGradient;
@end
@implementation OEHorizontalLine
@synthesize backgroundThemeImage;
@synthesize themeImage;
@synthesize themeTextAttributes;

- (void)viewWillMoveToWindow:(NSWindow *)newWindow
{
    [super viewWillMoveToWindow:newWindow];
    
    if([self window] != nil)
    {
        [[NSNotificationCenter defaultCenter] removeObserver:self name:NSWindowDidBecomeMainNotification object:[self window]];
        [[NSNotificationCenter defaultCenter] removeObserver:self name:NSWindowDidResignMainNotification object:[self window]];
    }
    
    if(newWindow != nil && [self isTrackingWindowActivity])
    {
        // Register with the default notification center for changes in the window's keyedness only if one of the themed elements (the state mask) is influenced by the window's activity
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(OE_windowKeyChanged:) name:NSWindowDidBecomeMainNotification object:newWindow];
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(OE_windowKeyChanged:) name:NSWindowDidResignMainNotification object:newWindow];
    }
}

- (void)OE_windowKeyChanged:(NSNotification *)notification
{
    // The keyedness of the window has changed, we want to redisplay the button with the new state, this is only fired when NSWindowDidBecomeMainNotification and NSWindowDidResignMainNotification is registered.
    [self setNeedsDisplay:YES];
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
    
    OEThemeGradient *gradient = [[OETheme sharedTheme] themeGradientForKey:key];
    [self setLineGradient:gradient];
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

- (BOOL)isTrackingWindowActivity
{
    return YES;
}

- (BOOL)isTrackingMouseActivity
{
    return NO;
}

- (void)drawRect:(NSRect)dirtyRect
{
    [[NSColor clearColor] setFill];
    NSRectFill(dirtyRect);
    
    if([self backgroundThemeImage] != nil)
    {
        [[[self backgroundThemeImage] imageForState:OEThemeStateDefault] drawInRect:[self bounds] fromRect:NSZeroRect operation:NSCompositeCopy fraction:1.0];
    }
    
    if([self lineGradient] != nil)
    {
        NSGradient *gradient = [[self lineGradient] gradientForState:OEThemeStateDefault];
        NSColor *bottomColor, *topColor;
        [gradient getColor:&bottomColor location:NULL atIndex:0];
        [gradient getColor:&topColor location:NULL atIndex:1];

        NSRect singleLineRect = [self bounds];
        singleLineRect.size.height = 1;

        NSRect dirtyLineRect = NSIntersectionRect(singleLineRect, dirtyRect);
        [bottomColor setFill];
        NSRectFill(dirtyLineRect);
        
        singleLineRect.origin.y += 1.0;
        dirtyLineRect = NSIntersectionRect(singleLineRect, dirtyRect);
        [topColor setFill];
        NSRectFill(dirtyLineRect);
    }

    if([self themeImage] != nil)
    {
        [[[self themeImage] imageForState:OEThemeStateDefault] drawInRect:[self bounds] fromRect:NSZeroRect operation:NSCompositeCopy fraction:1.0];
    }
}
@end
