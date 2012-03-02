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

@interface OEMainWindow ()

@property (strong) NSTimer * mouseIdleTimer;

@end

@implementation OEMainWindow
@synthesize mouseIdleTimer;

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
    
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(windowDidEnterFullScreen:) name:NSWindowDidEnterFullScreenNotification object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(windowDidExitFullScreen:) name:NSWindowDidExitFullScreenNotification object:nil];
}

- (void)windowDidEnterFullScreen:(NSNotification *)aNotification
{
    mouseIdleTimer = [NSTimer scheduledTimerWithTimeInterval:2 target:self selector:@selector(checkMouseIdleTime:) userInfo:nil repeats:YES];
    [mouseIdleTimer fire];
}

- (void)windowDidExitFullScreen:(NSNotification *)aNotification
{
    [mouseIdleTimer invalidate];
    [NSCursor setHiddenUntilMouseMoves:NO];
}

- (void)checkMouseIdleTime:(NSTimer*)aNotification
{
    CFTimeInterval mouseIdleTime = CGEventSourceSecondsSinceLastEventType(kCGEventSourceStateCombinedSessionState, kCGEventMouseMoved);
    if (mouseIdleTime >= 2)
    {
        [NSCursor setHiddenUntilMouseMoves:YES];
    }
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

- (void)drawThemeFrame:(NSRect)dirtyRect
{
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
