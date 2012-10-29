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

#import "OELibrarySplitView.h"
#import "OELibraryController.h"

extern NSString * const OESidebarMinWidth;
extern NSString * const OESidebarMaxWidth;
extern NSString * const OEMainViewMinWidth;

extern NSString * const OESidebarWidthKey;
extern NSString * const OESidebarVisibleKey;


@interface OELibrarySplitView ()
- (void)OE_commonLibrarySplitViewInit;
- (void)OE_replaceView:(NSView *)aView withView:(NSView *)anotherView animated:(BOOL)flag;
@end

@implementation OELibrarySplitView
@synthesize resizesLeftView, drawsWindowResizer;
@synthesize minWidth, sidebarMaxWidth, mainViewMinWidth;

#pragma mark - Lifecycle

+ (void)initialize
{
    if(self == [OELibrarySplitView class])
    {
        NSDictionary *defaults = @{ OESidebarMinWidth : @105.0f, OESidebarMaxWidth : @450.0f, OEMainViewMinWidth : @495.0f, OESidebarWidthKey : @186.0f };
        [[NSUserDefaults standardUserDefaults] registerDefaults:defaults];
    }
}

- (id)init
{
    if((self = [super init]))
    {
        [self OE_commonLibrarySplitViewInit];
    }
    
    return self;
}

- (id)initWithCoder:(NSCoder *)coder
{
    if((self = [super initWithCoder:coder]))
    {
        [self OE_commonLibrarySplitViewInit];
    }
    
    return self;
}

- (id)initWithFrame:(NSRect)frame
{
    if((self = [super initWithFrame:frame]))
    {
        [self OE_commonLibrarySplitViewInit];
    }
    
    return self;
}

- (void)OE_commonLibrarySplitViewInit
{
    [self setDrawsWindowResizer:YES];
    [self setResizesLeftView:NO];
    [self setDelegate:self];
}

#pragma mark - Main methods

- (NSView *)rightContentView
{
    return [[[[self subviews] objectAtIndex:1] subviews] objectAtIndex:0];
}

- (NSView *)leftContentView
{
    return [[[[self subviews] objectAtIndex:0] subviews] objectAtIndex:0];
}

- (void)OE_replaceView:(NSView *)aView withView:(NSView *)anotherView animated:(BOOL)animated;
{
    if(animated) NSLog(@"OELibrarySplitView hasn't implemented animation yet. Sorry about that; we'll proceed without animation");

    NSRect frame = [aView frame];
    NSView *superView = [aView superview];

    [superView replaceSubview:aView with:anotherView];
    [anotherView setFrame:frame];
}

- (void)replaceLeftContentViewWithView:(NSView*)contentView animated:(BOOL)animationFlag
{
    [self OE_replaceView:[self leftContentView] withView:contentView animated:animationFlag];
}

- (void)replaceRightContentViewWithView:(NSView*)contentView animated:(BOOL)animationFlag
{
    [self OE_replaceView:[self rightContentView] withView:contentView animated:animationFlag];
}

- (void)setSplitterPosition:(CGFloat)newPosition animated:(BOOL)animatedFlag
{
    // Note that newPosition may be 0, in which case we are being asked to hide the left view

    NSView *leftView  = [[self subviews] objectAtIndex:0];
    NSView *rightView = [[self subviews] objectAtIndex:1];

    NSRect newLeftFrame, newRightFrame;

    newLeftFrame            = [leftView frame];
    newLeftFrame.size.width = newPosition;

    newRightFrame            = [rightView frame];
    newRightFrame.origin.x   = newPosition + [self dividerThickness];
    newRightFrame.size.width = NSMaxX([rightView frame]) - newRightFrame.origin.x;

    if(animatedFlag)
    {
        [NSAnimationContext beginGrouping];
        {{
            [[NSAnimationContext currentContext] setDuration:0.2];
            [[leftView animator] setFrame:newLeftFrame];
            [[rightView animator] setFrame:newRightFrame];
        }}
        [NSAnimationContext endGrouping];
    }
    else
    {
        [leftView setFrame:newLeftFrame];
        [rightView setFrame:newRightFrame];
    }
}

- (CGFloat)splitterPosition
{
    NSView *leftView = [self.subviews objectAtIndex:0];
    return NSWidth([leftView frame]);
}

#pragma mark - NSSplitView overridden methods

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
    if([self splitView:self shouldHideDividerAtIndex:0]) return;

    [[NSColor blackColor] setFill];
    NSRectFill(rect);
}

#pragma mark - NSSplitViewDelegate protocol methods

- (CGFloat)splitView:(NSSplitView *)splitView constrainMinCoordinate:(CGFloat)proposedMin ofSubviewAt:(NSInteger)dividerIndex
{
    return [self minWidth];
}

- (CGFloat)splitView:(NSSplitView *)splitView constrainMaxCoordinate:(CGFloat)proposedMaximumPosition ofSubviewAt:(NSInteger)dividerIndex
{
    NSAssert(NSWidth([self frame]) - [self mainViewMinWidth] > [self sidebarMaxWidth], @"We should never get to the point where the library right view is this small");
    return [self sidebarMaxWidth];
}

- (BOOL)splitView:(NSSplitView *)splitView shouldHideDividerAtIndex:(NSInteger)dividerIndex
{
    return dividerIndex == 0 && [self splitterPosition] == 0;
}

- (BOOL)splitView:(NSSplitView *)splitView shouldAdjustSizeOfSubview:(NSView *)view
{
    return view != [[self subviews] objectAtIndex:0] || [self resizesLeftView];
}

@end
