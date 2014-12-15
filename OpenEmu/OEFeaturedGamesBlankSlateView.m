//
//  OEFeaturedGamesBlankSlateView.m
//  OpenEmu
//
//  Created by Christoph Leimbrock on 12/12/14.
//
//

#import "OEFeaturedGamesBlankSlateView.h"
#import "OEBlankSlateSpinnerView.h"

@implementation OEFeaturedGamesBlankSlateView
- (void)OE_commonBlankSlateInit
{
    [super OE_commonBlankSlateInit];
}

- (void)viewDidMoveToWindow
{
    [super viewDidMoveToWindow];

}

- (void)setProgressAction:(NSString *)progressAction
{
    [[[self subviews] copy] makeObjectsPerformSelector:@selector(removeFromSuperview)];

    NSView *view = [[NSView alloc] initWithFrame:(NSRect){ .size = { ViewWidth, ViewHeight },.origin ={0,0}}];

    OEBlankSlateSpinnerView *spinner = [[OEBlankSlateSpinnerView alloc] initWithFrame:NSZeroRect];

    [self OE_setupBoxInView:view withText:OELocalizedString(@"Fetching Games…", @"Featured Games Blank Slate View Updating Info") andImageView:spinner];
    [self OE_addLeftHeadlineWithText:OELocalizedString(@"Featured Games", @"") toView:view];

    NSRect rect = (NSRect){ .size = { [view frame].size.width, bottomTextViewHeight }};
    rect.origin.y = NSMaxY([view bounds])-NSHeight(rect)-instructionsTopToViewTop;

    NSTextView *textView = [[NSTextView alloc] initWithFrame:NSInsetRect(rect, -4, 0)];
    NSString   *text     = OELocalizedString(@"Check out some excellent homebrew games.", @"");
    [textView setString:text];
    [textView setDrawsBackground:NO];
    [textView setEditable:NO];
    [textView setSelectable:NO];
    [textView setFont:[[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:0 weight:0 size:11.0]];
    [textView setTextColor:[NSColor colorWithDeviceWhite:0.86 alpha:1.0]];
    [textView setTextContainerInset:NSMakeSize(0, 0)];

    NSShadow *shadow = [[NSShadow alloc] init];
    [shadow setShadowColor:[NSColor blackColor]];
    [shadow setShadowBlurRadius:0];
    [shadow setShadowOffset:(NSSize){0, -1}];
    [textView setShadow:shadow];

    [view addSubview:textView];

    [self OE_showView:view];
}

- (void)setError:(NSString *)error
{
    [[[self subviews] copy] makeObjectsPerformSelector:@selector(removeFromSuperview)];

    NSView *view = [[NSView alloc] initWithFrame:(NSRect){ .size = { ViewWidth, ViewHeight },.origin ={0,0}}];

    NSImageView *warningImageView = [[NSImageView alloc] initWithFrame:NSZeroRect];
    [warningImageView setImage:[NSImage imageNamed:@"blank_slate_warning"]];
    [warningImageView setImageScaling:NSImageScaleNone];
    [warningImageView setImageAlignment:NSImageAlignTop];
    [warningImageView unregisterDraggedTypes];

    [self OE_setupBoxInView:view withText:error andImageView:warningImageView];
    [self OE_addLeftHeadlineWithText:OELocalizedString(@"Featured Games", @"") toView:view];

    NSRect rect = (NSRect){ .size = { [view frame].size.width, bottomTextViewHeight }};
    rect.origin.y = NSMaxY([view bounds])-NSHeight(rect)-instructionsTopToViewTop;

    NSTextView *textView = [[NSTextView alloc] initWithFrame:NSInsetRect(rect, -4, 0)];
    NSString   *text     = OELocalizedString(@"Check out some excellent homebrew games.", @"");
    [textView setString:text];
    [textView setDrawsBackground:NO];
    [textView setEditable:NO];
    [textView setSelectable:NO];
    [textView setFont:[[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:0 weight:0 size:11.0]];
    [textView setTextColor:[NSColor colorWithDeviceWhite:0.86 alpha:1.0]];
    [textView setTextContainerInset:NSMakeSize(0, 0)];

    NSShadow *shadow = [[NSShadow alloc] init];
    [shadow setShadowColor:[NSColor blackColor]];
    [shadow setShadowBlurRadius:0];
    [shadow setShadowOffset:(NSSize){0, -1}];
    [textView setShadow:shadow];

    [view addSubview:textView];

    [self OE_showView:view];
}
@end
