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

#import "OpenEmu-Swift.h"

#pragma mark - Public variables

NSString *const OELibrarySplitViewResetSidebarNotification = @"OELibrarySplitViewResetSidebarNotification";

#pragma mark - Private variables

static const CGFloat _OESidebarMinWidth  = 105;
static const CGFloat _OESidebarMaxWidth  = 450;
static const CGFloat _OEMainViewMinWidth = 495;

static NSString * const _OESidebarWidthKey = @"lastSidebarWidth";

#pragma mark - OELibrarySplitView

@interface OELibrarySplitView () <NSSplitViewDelegate>

@property CGFloat previousWidth;

- (NSView *)OE_sidebarContainerView;
- (NSView *)OE_mainContainerView;
- (CGFloat)OE_visibleSidebarSplitterPosition;
- (void)OE_getSidebarFrame:(NSRect *)sidebarFrame mainFrame:(NSRect *)mainFrame forSplitterAtPosition:(CGFloat)splitterPosition;

@end

@implementation OELibrarySplitView

#pragma mark - Lifecycle

+ (void)initialize
{
    if (self != [OELibrarySplitView class])
        return;

    [[NSUserDefaults standardUserDefaults] registerDefaults:(@{
                                                             _OESidebarWidthKey   : @186.0f,
                                                             })];
}

- (id)init
{
    self = [super init];
    if (self) {
        [self OE_commonLibrarySplitViewInit];
    }
    return self;
}

- (instancetype)initWithCoder:(NSCoder *)coder
{
    self = [super initWithCoder:coder];
    if (self) {
        [self OE_commonLibrarySplitViewInit];
    }
    return self;
}

- (instancetype)initWithFrame:(NSRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
        [self OE_commonLibrarySplitViewInit];
    }
    return self;
}

- (void)OE_commonLibrarySplitViewInit {
    super.delegate = self;
}

- (void)awakeFromNib
{
    [super awakeFromNib];

    NSRect newSidebarFrame, newMainFrame;
    const CGFloat splitterPosition = [self OE_visibleSidebarSplitterPosition];
    [self OE_getSidebarFrame:&newSidebarFrame mainFrame:&newMainFrame forSplitterAtPosition:splitterPosition];
    self.OE_sidebarContainerView.frame = newSidebarFrame;
    self.OE_mainContainerView.frame = newMainFrame;

    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(resetSidebar) name:OELibrarySplitViewResetSidebarNotification object:nil];

    [self adjustSubviews];
}

#pragma mark - Main methods

- (void)OE_getSidebarFrame:(NSRect *)sidebarFrame mainFrame:(NSRect *)mainFrame forSplitterAtPosition:(CGFloat)splitterPosition
{
    splitterPosition = MAX(splitterPosition, 0);

    if (sidebarFrame) {
        *sidebarFrame = NSMakeRect(0.0,
                                   0.0,
                                   splitterPosition,
                                   NSHeight(self.frame));
    }

    if (mainFrame) {
        const CGFloat x = splitterPosition;
        *mainFrame = NSMakeRect(x,
                                0.0,
                                NSWidth(self.frame) - x,
                                NSHeight(self.frame));
    }
}

- (CGFloat)OE_visibleSidebarSplitterPosition
{
    return [[NSUserDefaults standardUserDefaults] doubleForKey:_OESidebarWidthKey];
}

- (CGFloat)splitterPosition
{
    return NSWidth(self.OE_sidebarContainerView.frame);
}

- (NSView *)OE_sidebarContainerView
{
    return self.subviews.firstObject;
}

- (NSView *)OE_mainContainerView
{
    return (self.subviews.count > 1 ? self.subviews[1] : nil);
}

- (void)resetSidebar
{
    [self setPosition:186.0 ofDividerAtIndex:0];
}

#pragma mark - Overridden methods

- (void)adjustSubviews
{
    const CGFloat currentWidth = NSWidth(self.frame);
    const CGFloat mainWidth    = NSWidth(self.OE_mainContainerView.frame);
    const CGFloat sidebarWidth = NSWidth(self.OE_sidebarContainerView.frame);

    CGFloat visualSplitterPosition;

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
    
    const BOOL userIsShrinkingWindow = currentWidth < _previousWidth;
    const BOOL mainViewCannotBeShrunk = mainWidth <= _OEMainViewMinWidth;
    const BOOL sidebarCanBeShrunk = sidebarWidth > _OESidebarMinWidth;
    if (userIsShrinkingWindow && mainViewCannotBeShrunk && sidebarCanBeShrunk) {
        visualSplitterPosition = currentWidth - _OEMainViewMinWidth;
    } else {
        visualSplitterPosition = sidebarWidth;
    }

    NSRect sidebarFrame, mainFrame;
    [self OE_getSidebarFrame:&sidebarFrame mainFrame:&mainFrame forSplitterAtPosition:visualSplitterPosition];
    self.OE_sidebarContainerView.frame = sidebarFrame;
    self.OE_mainContainerView.frame = mainFrame;

    _previousWidth = NSWidth(self.frame);
}

- (NSColor *)dividerColor
{
    return [NSColor colorWithDeviceWhite:0 alpha:0.2];
}

- (BOOL)isFlipped
{
    return YES;
}

- (void)viewDidMoveToWindow
{
    if (!self.window)
        return;

    NSView *contentView = self.window.contentView;

    const CGFloat splitViewMinWidth = _OESidebarMinWidth + self.dividerThickness + _OEMainViewMinWidth;
    const CGFloat leftMargin = [self convertPoint:NSZeroPoint toView:contentView].x;
    const CGFloat rightMargin = NSMaxX(contentView.frame) - [self convertPoint:NSMakePoint(NSMaxX(self.frame), 0) toView:contentView].x;
    const CGFloat contentMinWidth   = leftMargin + splitViewMinWidth + rightMargin;

    NSSize contentMinSize = self.window.contentMinSize;
#ifdef DEBUG_PRINT
    if (contentMinWidth > contentMinSize.width) {
        DLog(@"Content mininum width had to be adjusted from %f to %f", contentMinSize.width, contentMinWidth);
    }
#endif
    contentMinSize.width = MAX(contentMinSize.width, contentMinWidth);
    self.window.contentMinSize = contentMinSize;
}

#pragma mark - NSSplitViewDelegate protocol methods

- (CGFloat)splitView:(NSSplitView *)splitView constrainMinCoordinate:(CGFloat)proposedMin ofSubviewAt:(NSInteger)dividerIndex
{
    return _OESidebarMinWidth;
}

- (CGFloat)splitView:(NSSplitView *)splitView constrainMaxCoordinate:(CGFloat)proposedMaximumPosition ofSubviewAt:(NSInteger)dividerIndex
{
    return MIN(_OESidebarMaxWidth, NSWidth(self.frame) - _OEMainViewMinWidth);
}

@end
