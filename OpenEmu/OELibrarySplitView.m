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

#import "OELibrarySplitView.h"
#import "NSWindow+OEFullScreenAdditions.h"

#pragma mark - Public variables

NSString *const OELibrarySplitViewDidToggleSidebarNotification = @"OELibrarySplitViewDidToggleSidebarNotification";

#pragma mark - Private variables

static const CGFloat _OEToggleSidebarAnimationDuration = 0.2;
static const CGFloat _OESidebarMinWidth                = 105;
static const CGFloat _OESidebarMaxWidth                = 450;
static const CGFloat _OEMainViewMinWidth               = 495;

static NSString * const _OESidebarVisibleKey = @"isSidebarVisible";
static NSString * const _OESidebarWidthKey   = @"lastSidebarWidth";

#pragma mark - OELibrarySplitViewDelegateProxy

@interface OELibrarySplitViewDelegateProxy : NSObject <OELibrarySplitViewDelegate>
@property(nonatomic, weak) id<NSSplitViewDelegate>                     superDelegate;
@property(nonatomic, unsafe_unretained) id<OELibrarySplitViewDelegate> localDelegate;
// can't use weak for localDelegate yet because OELibraryController, the delegate, inherits from NSViewController,
// which is weak-unavailable in 10.7. See issue #345.
@end

#pragma mark - OELibrarySplitView

@interface OELibrarySplitView () <NSSplitViewDelegate>
{
    BOOL    _animating;
    BOOL    _togglingSidebarAndChangingWindowSize;
    CGFloat _previousWidth;

    OELibrarySplitViewDelegateProxy *_delegateProxy;
}

- (instancetype)OE_commonLibrarySplitViewInit __attribute__((objc_method_family(init)));
- (NSView *)OE_sidebarContainerView;
- (NSView *)OE_mainContainerView;
- (CGFloat)OE_visibleSidebarSplitterPosition;
- (void)OE_getSidebarFrame:(NSRect *)sidebarFrame mainFrame:(NSRect *)mainFrame forSplitterAtPosition:(CGFloat)splitterPosition;

@end

@implementation OELibrarySplitView

#pragma mark - Lifecycle

+ (void)initialize
{
    if(self != [OELibrarySplitView class]) return;

    [[NSUserDefaults standardUserDefaults] registerDefaults:(@{
                                                             _OESidebarVisibleKey : @YES,
                                                             _OESidebarWidthKey   : @186.0f,
                                                             })];

}

- (id)init
{
    return [[super init] OE_commonLibrarySplitViewInit];
}

- (id)initWithCoder:(NSCoder *)coder
{
    return [[super initWithCoder:coder] OE_commonLibrarySplitViewInit];
}

- (id)initWithFrame:(NSRect)frame
{
    return [[super initWithFrame:frame] OE_commonLibrarySplitViewInit];
}

- (instancetype)OE_commonLibrarySplitViewInit
{
    [super setDelegate:self];
    return self;
}

#pragma mark - Main methods

- (void)toggleSidebar
{
    if(_animating) return;
    _animating = YES;

    const BOOL hidingSidebar = [self isSidebarVisible];
    [[NSUserDefaults standardUserDefaults] setBool:!hidingSidebar forKey:_OESidebarVisibleKey];

    const BOOL shouldChangeWindowSize = ![[self window] OE_isFullScreen];

    void (^animationCompletionHandler)(void) = ^{
        _animating = NO;
        if(shouldChangeWindowSize) _togglingSidebarAndChangingWindowSize = NO;

        NSNotification *notification = [NSNotification notificationWithName:OELibrarySplitViewDidToggleSidebarNotification object:self];
        [[NSNotificationCenter defaultCenter] postNotification:notification];

        if([[_delegateProxy localDelegate] respondsToSelector:@selector(librarySplitViewDidToggleSidebar:)])
        {
            [[_delegateProxy localDelegate] librarySplitViewDidToggleSidebar:notification];
        }

    };

    const CGFloat splitterPosition = [self OE_visibleSidebarSplitterPosition];

    // If toggling the sidebar enlarges or reduces the window width, we change the window frame
    // accordingly and let -adjustSubviews compute the visual splitter position whilst the window
    // size is changing
    if(shouldChangeWindowSize)
    {
        const CGFloat widthCorrection = (hidingSidebar ? -splitterPosition : splitterPosition);
        NSRect windowFrame = [[self window] frame];
        windowFrame.origin.x -= widthCorrection;
        windowFrame.size.width += widthCorrection;

        _togglingSidebarAndChangingWindowSize = YES;
        [NSAnimationContext beginGrouping];
        {{
            [[NSAnimationContext currentContext] setCompletionHandler:animationCompletionHandler];

            [[[self window] animator] setFrame:windowFrame display:YES animate:YES];
        }};
        [NSAnimationContext endGrouping];
    }
    else
    {
        NSRect newSidebarFrame, newMainFrame;
        [self OE_getSidebarFrame:&newSidebarFrame mainFrame:&newMainFrame forSplitterAtPosition:(hidingSidebar ? 0 : splitterPosition)];

        [NSAnimationContext beginGrouping];
        {{
            [[NSAnimationContext currentContext] setDuration:_OEToggleSidebarAnimationDuration];
            [[NSAnimationContext currentContext] setCompletionHandler:animationCompletionHandler];

            [[[self OE_sidebarContainerView] animator] setFrame:newSidebarFrame];
            [[[self OE_mainContainerView] animator] setFrame:newMainFrame];
        }}
        [NSAnimationContext endGrouping];
    }
}

- (void)OE_getSidebarFrame:(NSRect *)sidebarFrame mainFrame:(NSRect *)mainFrame forSplitterAtPosition:(CGFloat)splitterPosition
{
    splitterPosition = MAX(splitterPosition, 0);

    if(sidebarFrame)
    {
        *sidebarFrame = (NSRect)
        {
            .origin      = NSZeroPoint,
            .size.width  = splitterPosition,
            .size.height = NSHeight([self frame])
        };

    }

    if(mainFrame)
    {
        const CGFloat x = splitterPosition;
        *mainFrame = (NSRect)
        {
            .origin.x    = x,
            .origin.y    = 0,
            .size.width  = NSWidth([self frame]) - x,
            .size.height = NSHeight([self frame])
        };
    }
}

- (CGFloat)OE_visibleSidebarSplitterPosition
{
    return [[NSUserDefaults standardUserDefaults] doubleForKey:_OESidebarWidthKey];
}

- (CGFloat)splitterPosition
{
    return NSWidth([[self OE_sidebarContainerView] frame]);
}

- (NSView *)OE_sidebarContainerView
{
    return ([[self subviews] count] > 0 ? [[self subviews] objectAtIndex:0] : nil);
}

- (NSView *)OE_mainContainerView
{
    return ([[self subviews] count] > 1 ? [[self subviews] objectAtIndex:1] : nil);
}

- (BOOL)isSidebarVisible
{
    return [[NSUserDefaults standardUserDefaults] boolForKey:_OESidebarVisibleKey];
}

#pragma mark - Overridden methods

- (void)adjustSubviews
{
    const CGFloat currentWidth = NSWidth([self frame]);
    const CGFloat mainWidth    = NSWidth([[self OE_mainContainerView] frame]);
    const CGFloat sidebarWidth = NSWidth([[self OE_sidebarContainerView] frame]);

    CGFloat visualSplitterPosition;

    // If we are changing the window size because the sidebar is being revealed or hidden, we:
    // - keep the width and the screen x origin of the main frame intact
    // - increase the sidebar width if the sidebar is being revealed
    // - decrease the sidebar width if the sidebar is being hidden
    // Note that whilst we are doing this, we are violating min/max width restrictions
    if(_togglingSidebarAndChangingWindowSize)
    {
        visualSplitterPosition = currentWidth - mainWidth;
    }
    else
    {
        // We cannot always let NSSplitView adjust the subviews with help from -splitView:shouldAdjustSizeOfSubview:
        // because of the following scenario:
        // - sidebar width > _OESidebarMinWidth
        // - main width > _OEMainViewMinWidth
        // - user aggressively shrinks the window width in one mouse swipe
        // When this happens, -splitView:shouldAdjustSizeOfSubview: initially returns YES for the main view since
        // it can be shrunk, and we prefer not to touch the sidebar width. However, since
        // -splitView:shouldAdjustSizeOfSubview: is not necessarily sent in a granular enough fashion, it is well
        /// possible that in the second time that it is sent the window has already been shrunk to a size that
        // violates _OEMainViewMinWidth. In this case, we force the splitter position to respect _OEMainViewMinWidth

        if(currentWidth <  _previousWidth      && // user is shrinking the window
           mainWidth    <= _OEMainViewMinWidth && // main view cannot be shrunk
           sidebarWidth >  _OESidebarMinWidth)    // sidebar can be shrunk
        {
            visualSplitterPosition = currentWidth - _OEMainViewMinWidth;
        }
        else
        {
            visualSplitterPosition = sidebarWidth;
        }
    }

    NSRect sidebarFrame, mainFrame;
    [self OE_getSidebarFrame:&sidebarFrame mainFrame:&mainFrame forSplitterAtPosition:visualSplitterPosition];
    [[self OE_sidebarContainerView] setFrame:sidebarFrame];
    [[self OE_mainContainerView] setFrame:mainFrame];

    _previousWidth = NSWidth([self frame]);
}

- (void)setDelegate:(id<OELibrarySplitViewDelegate>)delegate
{
    if(!_delegateProxy)
    {
        _delegateProxy = [OELibrarySplitViewDelegateProxy new];
        [_delegateProxy setSuperDelegate:self];
        [super setDelegate:_delegateProxy];
    }

    [_delegateProxy setLocalDelegate:delegate];
}

// We don't really draw the divider, so its thickness is 0 from an NSSplitView perspective.
// Eventually we should make this class consider non-zero thickness.
- (CGFloat)dividerThickness
{
    return 0.0;
}

- (BOOL)isFlipped
{
    return YES;
}

- (void)viewDidMoveToWindow
{
    if(![self window]) return;

    NSView *contentView = [[self window] contentView];

    const CGFloat splitViewMinWidth = _OESidebarMinWidth + [self dividerThickness] + _OEMainViewMinWidth;
    const CGFloat leftMargin        = [self convertPoint:NSZeroPoint toView:contentView].x;
    const CGFloat rightMargin       = NSMaxX([contentView frame]) - [self convertPoint:(NSPoint){NSMaxX([self frame]), 0} toView:contentView].x;
    const CGFloat contentMinWidth   = leftMargin + splitViewMinWidth + rightMargin;

    NSSize contentMinSize = [[self window] contentMinSize];
#ifdef DEBUG_PRINT
    if(contentMinWidth > contentMinSize.width) DLog(@"Content mininum width had to be adjusted from %f to %f", contentMinSize.width, contentMinWidth);
#endif
    contentMinSize.width = MAX(contentMinSize.width, contentMinWidth);
    [[self window] setContentMinSize:contentMinSize];
}

#pragma mark - NSSplitViewDelegate protocol methods

- (CGFloat)splitView:(NSSplitView *)splitView constrainMinCoordinate:(CGFloat)proposedMin ofSubviewAt:(NSInteger)dividerIndex
{
    return _OESidebarMinWidth;
}

- (CGFloat)splitView:(NSSplitView *)splitView constrainMaxCoordinate:(CGFloat)proposedMaximumPosition ofSubviewAt:(NSInteger)dividerIndex
{
    return MIN(_OESidebarMaxWidth, NSWidth([self frame]) - _OEMainViewMinWidth);
}

- (BOOL)splitView:(NSSplitView *)splitView shouldHideDividerAtIndex:(NSInteger)dividerIndex
{
    return ![self isSidebarVisible];
}

- (void)splitViewDidResizeSubviews:(NSNotification *)notification
{
    if(!_togglingSidebarAndChangingWindowSize && [self isSidebarVisible]) [[NSUserDefaults standardUserDefaults] setDouble:[self splitterPosition] forKey:_OESidebarWidthKey];

    if([[_delegateProxy localDelegate] respondsToSelector:_cmd]) [[_delegateProxy localDelegate] splitViewDidResizeSubviews:notification];
}

@end

#pragma mark - OELibrarySplitViewDelegateProxy

@implementation OELibrarySplitViewDelegateProxy

- (id)forwardingTargetForSelector:(SEL)selector
{
    // OELibrarySplitView takes precedence over its (local) delegate
    if([_superDelegate respondsToSelector:selector]) return _superDelegate;
    if([_localDelegate respondsToSelector:selector]) return _localDelegate;
    return nil;
}

@end
