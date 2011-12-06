//
//  LibrarySplitView.m
//  OpenEmu
//
//  Created by Christoph Leimbrock on 30.03.11.
//  Copyright 2011 none. All rights reserved.
//

#import "OELibrarySplitView.h"
#import "OELibraryController.h"
@interface OELibrarySplitView (Private)
- (void)_setup;
@end
@implementation OELibrarySplitView
@synthesize resizesLeftView, drawsWindowResizer;
@synthesize minWidth, sidebarMaxWidth, mainViewMinWidth;
@synthesize libraryConroller;

- (id)init 
{
    self = [super init];
    if (self) 
    {
        [self _setup];
    }
    return self;
}

- (id)initWithCoder:(NSCoder *)coder 
{
    self = [super initWithCoder:coder];
    if (self) {
        [self _setup];
    }
    return self;
}

- (id)initWithFrame:(NSRect)frame 
{
    self = [super initWithFrame:frame];
    if (self) 
    {
        [self _setup];
    }
    return self;
}

- (void)_setup
{
    [self setDrawsWindowResizer:YES];
    [self setResizesLeftView:NO];
    [self setDelegate:self];
}

- (void)dealloc {
    [self setLibraryConroller:nil];
    
    [super dealloc];
}

#pragma mark -
- (CGFloat)splitView:(NSSplitView *)splitView constrainMinCoordinate:(CGFloat)proposedMin ofSubviewAt:(NSInteger)dividerIndex
{
    return [self minWidth];
}

- (CGFloat)splitView:(NSSplitView *)splitView constrainMaxCoordinate:(CGFloat)proposedMaximumPosition ofSubviewAt:(NSInteger)dividerIndex
{   
    return  ([self frame].size.width-[self mainViewMinWidth])>[self sidebarMaxWidth]?[self sidebarMaxWidth]:([self frame].size.width-[self mainViewMinWidth]);
}

- (BOOL)splitView:(NSSplitView *)splitView canCollapseSubview:(NSView *)subview
{    
    if(subview == [[self subviews] objectAtIndex:0]) return NO;
    return NO;
}

- (BOOL)isSubviewCollapsed:(NSView*)view
{
    return NO;
}

- (BOOL)splitView:(NSSplitView *)splitView shouldHideDividerAtIndex:(NSInteger)dividerIndex
{
    return dividerIndex==0 && [self splitterPosition]==0;
}

- (void)splitView:(NSSplitView *)aSplitView resizeSubviewsWithOldSize:(NSSize)oldSize
{    
    NSView *view0 = [self.subviews objectAtIndex: 0];
    NSView *view1 = [self.subviews objectAtIndex: 1];
    
    NSSize newSize = [aSplitView frame].size;
    NSRect leftFrame = [view0 frame];
    NSRect rightFrame = [view1 frame];
    
    if([self resizesLeftView])
    {
        leftFrame.size = newSize;
        rightFrame.size.height = newSize.height;
        
        leftFrame.size.width = newSize.width-rightFrame.size.width-[self dividerThickness];
        leftFrame.size.width = leftFrame.size.width<0?0:leftFrame.size.width;
        
        rightFrame.origin.x = leftFrame.size.width+[self dividerThickness];
    }
    else 
    {
        leftFrame.size.height = newSize.height;
        
        rightFrame.size = newSize;
        rightFrame.size.width = newSize.width-leftFrame.size.width-[self dividerThickness];
        rightFrame.size.width = rightFrame.size.width<0?0:rightFrame.size.width;
    }
    
    [view0 setFrame:leftFrame];
    [view1 setFrame:rightFrame];
    
    [[self libraryConroller] layoutToolbarItems];
}

- (void)splitViewDidResizeSubviews:(NSNotification *)notification
{
    [[self libraryConroller] layoutToolbarItems];
}

- (NSRect)splitView:(NSSplitView *)splitView effectiveRect:(NSRect)proposedEffectiveRect forDrawnRect:(NSRect)drawnRect ofDividerAtIndex:(NSInteger)dividerIndex
{   
    return proposedEffectiveRect;
}

- (CGFloat)dividerThickness
{   
    return 0.0;
}

- (BOOL)isFlipped
{
    return YES;
}


- (void)drawDividerInRect:(NSRect)rect
{
    if([self splitView:self shouldHideDividerAtIndex:0])
    {
        return;
    }
    
    [[NSColor blackColor] setFill];
    NSRectFill(rect);
}

- (void)setSplitterPosition:(CGFloat)newPosition animated:(BOOL)animatedFlag
{
    NSView *view0 = [self.subviews objectAtIndex: 0];
    NSView *view1 = [self.subviews objectAtIndex: 1];
    
    NSRect view0TargetFrame, view1TargetFrame;
    
    view0TargetFrame = NSMakeRect( view0.frame.origin.x, view0.frame.origin.y, newPosition, view0.frame.size.height);
    view1TargetFrame = NSMakeRect( newPosition + self.dividerThickness, view1.frame.origin.y, NSMaxX(view1.frame) - newPosition - self.dividerThickness, view1.frame.size.height);   
    
    if(animatedFlag)
    {
        [NSAnimationContext beginGrouping];
        [[NSAnimationContext currentContext] setDuration:0.2];
        [[view0 animator] setFrame: view0TargetFrame];
        [[view1 animator] setFrame: view1TargetFrame];
        [NSAnimationContext endGrouping];
    } 
    else
    {
        [view0 setFrame: view0TargetFrame];
        [view1 setFrame: view1TargetFrame];
    }
}

- (float)splitterPosition
{
    NSView *view0 = [self.subviews objectAtIndex:0];
    return [view0 frame].size.width;
}


- (NSView*)rightContentView
{
    return [[[[self subviews] objectAtIndex:1] subviews] objectAtIndex:0];
}

- (NSView*)leftContentView
{
    
    return [[[[self subviews] objectAtIndex:0] subviews] objectAtIndex:0];
}

- (void)_replaceView:(NSView*)aView withView:(NSView*)anotherView animated:(BOOL)flag
{
    if(!flag)
    {
        NSRect frame = [aView frame];
        NSView* superView = [aView superview];
        
        [superView replaceSubview:aView with:anotherView]; 
        [anotherView setFrame:frame];
    } 
    else 
    {
        NSLog(@"animation not implemented yet");
    }
    
}
- (void)replaceLeftContentViewWithView:(NSView*)contentView animated:(BOOL)animationFlag
{
    [self _replaceView:[self leftContentView] withView:contentView animated:animationFlag];
}
- (void)replaceRightContentViewWithView:(NSView*)contentView animated:(BOOL)animationFlag
{
    [self _replaceView:[self rightContentView] withView:contentView animated:animationFlag];
}
@end