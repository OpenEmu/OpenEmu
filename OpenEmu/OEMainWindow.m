//
//  OELibraryWindow.m
//  OpenEmuMockup
//
//  Created by Christoph Leimbrock on 08.04.11.
//  Copyright 2011 Christoph Leimbrock. All rights reserved.
//

#import "OEMainWindow.h"
#import <Quartz/Quartz.h>
#define titleBarHeight 21.0
@implementation OEMainWindow
- (void)awakeFromNib
{
    [super awakeFromNib];
    
    NSView *contentView = [self contentView];
    NSView *windowBorderView = [contentView superview];
    NSRect windowBorderFrame = [windowBorderView frame];
    
    NSRect titlebarRect = NSMakeRect(0, windowBorderFrame.size.height-22, windowBorderFrame.size.width, 22);
    OEMainWindowTitleBarView *titlebarView = [[[OEMainWindowTitleBarView alloc] initWithFrame:titlebarRect] autorelease];
    [titlebarView setAutoresizingMask:(NSViewMinYMargin | NSViewWidthSizable)];
    [windowBorderView addSubview:titlebarView positioned:NSWindowAbove relativeTo:[[windowBorderView subviews] objectAtIndex:0]];
    
    [contentView setWantsLayer:YES];
    
    CATransition *cvTransition = [CATransition animation];
    cvTransition.type = kCATransitionFade;
    cvTransition.timingFunction = [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionDefault];
    cvTransition.duration = 0.8;
    [contentView setAnimations:[NSDictionary dictionaryWithObject:cvTransition forKey:@"subviews"]];

    [self setOpaque:NO];
    [self setBackgroundColor:[NSColor clearColor]]; 
}

- (void)setMainContentView:(NSView*)view
{
    NSView *contentView = [self contentView];
    if(mainContentView)
    {
        [view setFrame:[mainContentView frame]];
        [view setAutoresizingMask:[mainContentView autoresizingMask]];
        [[contentView animator] replaceSubview:mainContentView with:view];   
    }
    else 
    {
        [view setAutoresizingMask:NSViewWidthSizable|NSViewHeightSizable];
        [view setFrame:(NSRect){{0,45},{contentView.frame.size.width, contentView.frame.size.height-45}}];
        [contentView addSubview:view];       
    }
    
    [view retain];
    [mainContentView release];
    mainContentView = view;
}

- (NSView*)mainContentView
{
    return mainContentView;
}
#pragma mark -
#pragma mark Custom Theme Drawing

+ (void)initialize
{
    // Make sure not to reinitialize for subclassed objects
    if (self != [OEMainWindow class])
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
        float newHeight = [self frame].origin.y + [self frame].size.height-dirtyRect.origin.y - titleBarHeight;
        if(newHeight <= 0.0)
        {
            return;
        }
        dirtyRect.size.height = newHeight;
    }
    
    [NSColor blackColor];
    NSRectFill(dirtyRect);
    
}
@end
@implementation OEMainWindowTitleBarView
- (void)drawRect:(NSRect)dirtyRect
{
    if(dirtyRect.origin.y!=0) return;
    
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
    
    NSColor *topLineColor = [NSColor colorWithDeviceWhite:0.32 alpha:1];
    NSColor *gradientTop = [NSColor colorWithDeviceWhite:0.2 alpha:1];
    NSColor *gradientBottom = [NSColor colorWithDeviceWhite:0.15 alpha:1];
    
    // Draw top line
    NSRect lineRect = NSMakeRect(0, 43, viewRect.size.width, 1);
    [topLineColor setFill];
    NSRectFill(lineRect);
    
    // Draw Gradient
    viewRect.origin.y = 0;
    viewRect.size.height -= 1;
    NSGradient *backgroundGradient = [[NSGradient alloc] initWithStartingColor:gradientTop endingColor:gradientBottom];
    [backgroundGradient drawInRect:viewRect angle:-90];
    [backgroundGradient release];
}
- (BOOL)isOpaque
{
    return NO;
}

@end