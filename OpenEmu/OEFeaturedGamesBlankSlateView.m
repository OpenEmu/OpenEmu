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
- (void)commonBlankSlateInit
{
    [super commonBlankSlateInit];
}

- (void)viewDidMoveToWindow
{
    [super viewDidMoveToWindow];
}

- (void)setProgressAction:(NSString *)progressAction
{
    OEBlankSlateSpinnerView *spinner = [[OEBlankSlateSpinnerView alloc] initWithFrame:NSZeroRect];
    NSString   *text     = OELocalizedString(@"Check out some excellent homebrew games.", @"");
    [self setupBoxWithText:OELocalizedString(@"Fetching Games…", @"Featured Games Blank Slate View Updating Info") andImageView:spinner];
    [self addLeftHeadlineWithText:OELocalizedString(@"Featured Games", @"")];
    [self addInformationalText:text];
}

- (void)setError:(NSString *)error
{
    NSImageView *warningImageView = [[NSImageView alloc] initWithFrame:NSZeroRect];
    [warningImageView setImage:[NSImage imageNamed:@"blank_slate_warning"]];
    [warningImageView setImageScaling:NSImageScaleNone];
    [warningImageView setImageAlignment:NSImageAlignTop];
    [warningImageView unregisterDraggedTypes];
    NSString   *text = OELocalizedString(@"Check out some excellent homebrew games.", @"");
    [self setupBoxWithText:error andImageView:warningImageView];
    [self addLeftHeadlineWithText:OELocalizedString(@"Featured Games", @"")];
    [self addInformationalText:text];
}
@end
