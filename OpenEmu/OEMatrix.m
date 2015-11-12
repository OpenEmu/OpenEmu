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

#import "OEMatrix.h"
#import "OETheme.h"
#import "OECell.h"

@interface OEMatrix ()

- (void)OE_windowKeyChanged:(NSNotification *)notification;

@end

@implementation OEMatrix
@synthesize backgroundThemeImage = _backgroundThemeImage;
@synthesize themeImage = _themeImage;
@synthesize themeTextAttributes = _themeTextAttributes;

- (void)viewWillMoveToWindow:(NSWindow *)newWindow
{
    [super viewWillMoveToWindow:newWindow];

    if([self window])
    {
        [[NSNotificationCenter defaultCenter] removeObserver:self name:NSWindowDidBecomeMainNotification object:[self window]];
        [[NSNotificationCenter defaultCenter] removeObserver:self name:NSWindowDidResignMainNotification object:[self window]];
    }

    if(newWindow)
    {
        // Register with the default notification center for changes in the window's keyedness only if one of the themed elements (the state mask) is influenced by the window's activity
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(OE_windowKeyChanged:) name:NSWindowDidBecomeMainNotification object:newWindow];
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(OE_windowKeyChanged:) name:NSWindowDidResignMainNotification object:newWindow];
    }
}

- (void)viewDidMoveToWindow
{
    [super viewDidMoveToWindow];
    [self updateTrackingAreas];
}

- (void)updateTrackingAreas
{
    if(_trackingArea) [self removeTrackingArea:_trackingArea];
    // Track mouse enter and exit (hover and off) events only if the one of the themed elements (the state mask) is influenced by the mouse
    _trackingArea = [[NSTrackingArea alloc] initWithRect:[self bounds] options:NSTrackingActiveInActiveApp | NSTrackingMouseEnteredAndExited | NSTrackingMouseMoved owner:self userInfo:nil];
    [self addTrackingArea:_trackingArea];
}

- (void)OE_setHoverCell:(id<OECell>)hoverCell
{
    if (_hoverCell != hoverCell)
    {
        [_hoverCell setHovering:NO];
        if([hoverCell conformsToProtocol:@protocol(OECell)])
        {
            _hoverCell = hoverCell;
            [_hoverCell setHovering:YES];
        }
        else
        {
            _hoverCell = nil;
        }
        [self setNeedsDisplay:YES];
    }
}

- (void)updateHoverFlagWithMousePoint:(NSPoint)point
{
    NSArray *cells = [self cells];
    if([cells count] == 0)
    {
        [self OE_setHoverCell:nil];
        return;
    }

    const NSSize cellSize = [self cellSize];
    NSUInteger row =  point.y / cellSize.height;
    NSUInteger col =  point.x / cellSize.width;
    [self OE_setHoverCell:[self cellAtRow:row column:col]];
}

- (void)OE_updateHoverFlag:(NSEvent *)theEvent
{
    const NSPoint locationInView = [self convertPoint:[theEvent locationInWindow] fromView:nil];
    [self updateHoverFlagWithMousePoint:locationInView];
}

- (void)mouseEntered:(NSEvent *)theEvent
{
    [self OE_updateHoverFlag:theEvent];
}

- (void)mouseExited:(NSEvent *)theEvent
{
    [self OE_updateHoverFlag:theEvent];
}

- (void)mouseMoved:(NSEvent *)theEvent
{
    [self OE_updateHoverFlag:theEvent];
}

- (void)OE_windowKeyChanged:(NSNotification *)notification
{
    // The keyedness of the window has changed, we want to redisplay the button with the new state, this is only fired when NSWindowDidBecomeMainNotification and NSWindowDidResignMainNotification is registered.
    [self setNeedsDisplay];
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
        _backgroundThemeImage = backgroundThemeImage;
        NSArray *cells = [self cells];
        for(id<OECell> cell in cells)
        {
            if([cell conformsToProtocol:@protocol(OECell)]) [cell setBackgroundThemeImage:_backgroundThemeImage];
        }
        [self setNeedsDisplay];
    }
}

- (void)setThemeImage:(OEThemeImage *)themeImage
{
    if(_themeImage != themeImage)
    {
        _themeImage = themeImage;
        NSArray *cells = [self cells];
        for(id<OECell> cell in cells)
        {
            if([cell conformsToProtocol:@protocol(OECell)]) [cell setThemeImage:_themeImage];
        }
        [self setNeedsDisplay];
    }
}

- (void)setThemeTextAttributes:(OEThemeTextAttributes *)themeTextAttributes
{
    if(_themeTextAttributes != themeTextAttributes)
    {
        _themeTextAttributes = themeTextAttributes;
        NSArray *cells = [self cells];
        for(id<OECell> cell in cells)
        {
            if([cell conformsToProtocol:@protocol(OECell)]) [cell setThemeTextAttributes:_themeTextAttributes];
        }
        [self setNeedsDisplay];
    }
}

- (BOOL)isTrackingMouseActivity
{
    return YES;
}

- (BOOL)isTrackingWindowActivity
{
    return YES;
}

@end
