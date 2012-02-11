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

#import "OEMainWindow.h"
#import <Quartz/Quartz.h>
#define titleBarHeight 21.0

@implementation OEMainWindow

- (void)awakeFromNib
{
    [super awakeFromNib];
    
    NSView *contentView       = [self contentView];
    NSView *windowBorderView  = [contentView superview];
    NSRect  windowBorderFrame = [windowBorderView frame];
    
    NSRect titlebarRect = NSMakeRect(0, windowBorderFrame.size.height - 22.0, windowBorderFrame.size.width, 22.0);
    OEMainWindowTitleBarView *titlebarView = [[OEMainWindowTitleBarView alloc] initWithFrame:titlebarRect];
    [titlebarView setAutoresizingMask:(NSViewMinYMargin | NSViewWidthSizable)];
    [windowBorderView addSubview:titlebarView positioned:NSWindowAbove relativeTo:[[windowBorderView subviews] objectAtIndex:0]];
    
    [contentView setWantsLayer:YES];
    
    CATransition *cvTransition = [CATransition animation];
    cvTransition.type = kCATransitionFade;
    cvTransition.timingFunction = [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionDefault];
    cvTransition.duration = 0.8;
    [contentView setAnimations:[NSDictionary dictionaryWithObject:cvTransition forKey:@"subviews"]];
    
    [self setOpaque:NO];
    [self setBackgroundColor:[NSColor blackColor]];
}

#pragma mark -
#pragma mark Custom Theme Drawing

+ (void)initialize
{
    // Make sure not to reinitialize for subclassed objects
    if(self != [OEMainWindow class])
        return;
    
    [NSWindow registerWindowClassForCustomThemeFrameDrawing:[OEMainWindow class]];
}

- (BOOL)drawsAboveDefaultThemeFrame
{
    return YES;
}

- (void)drawThemeFrame:(NSValue*)dirtyRectValue
{
    NSRect dirtyRect = [dirtyRectValue rectValue];
    float maxY = NSMaxY(dirtyRect);
    
    if(maxY > NSMaxY([self frame]) - titleBarHeight)
    {
        float newHeight = [self frame].origin.y + [self frame].size.height - dirtyRect.origin.y - titleBarHeight;
        if(newHeight <= 0.0) return;
        dirtyRect.size.height = newHeight;
    }
    
    [NSColor blackColor];
    NSRectFill(dirtyRect);
}

@end

@implementation OEMainWindowTitleBarView

- (void)drawRect:(NSRect)dirtyRect
{
    if(dirtyRect.origin.y != 0) return;
    
    dirtyRect.size.height = 1;
    [[NSColor blackColor] setFill];
    NSRectFill(dirtyRect);
}

@end

@implementation OEMainWindowContentView

- (void)drawRect:(NSRect)dirtyRect
{
    [[NSColor blackColor] setFill];
    NSRectFill(dirtyRect);
    
    if(NSMinY(dirtyRect) > 44) return;
    
    NSRect viewRect = [self bounds];
    viewRect.origin.y = NSMinY(viewRect);
    viewRect.size.height = 44.0;
    
    NSColor *topLineColor   = [NSColor colorWithDeviceWhite:0.32 alpha:1];
    NSColor *gradientTop    = [NSColor colorWithDeviceWhite:0.20 alpha:1];
    NSColor *gradientBottom = [NSColor colorWithDeviceWhite:0.15 alpha:1];
    
    // Draw top line
    NSRect lineRect = NSMakeRect(0, 43, viewRect.size.width, 1);
    [topLineColor setFill];
    NSRectFill(lineRect);
    
    // Draw Gradient
    viewRect.origin.y = 0;
    viewRect.size.height -= 1;
    NSGradient *backgroundGradient = [[NSGradient alloc] initWithStartingColor:gradientTop endingColor:gradientBottom];
    [backgroundGradient drawInRect:viewRect angle:-90.0];
}

- (BOOL)isOpaque
{
    return NO;
}

@end
