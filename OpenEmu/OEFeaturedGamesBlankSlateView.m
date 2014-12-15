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

- (void)setupViewForRepresentedObject
{
    id representedObject = [self representedObject];

    NSString *text = OELocalizedString(@"Check out some excellent homebrew games.", @"");
    if([representedObject isKindOfClass:[NSString class]])
    {
        OEBlankSlateSpinnerView *spinner = [[OEBlankSlateSpinnerView alloc] initWithFrame:NSZeroRect];
        [self setupBoxWithText:representedObject andImageView:spinner];
        [self addLeftHeadlineWithText:OELocalizedString(@"Featured Games", @"")];
        [self addInformationalText:text];
    }
    else if([representedObject isKindOfClass:[NSError class]])
    {
        NSImageView *warningImageView = [[NSImageView alloc] initWithFrame:NSZeroRect];
        [warningImageView setImage:[NSImage imageNamed:@"blank_slate_warning"]];
        [warningImageView setImageScaling:NSImageScaleNone];
        [warningImageView setImageAlignment:NSImageAlignTop];
        [warningImageView unregisterDraggedTypes];

        // TODO: extract real error reason here. Make sure it's not much longer than this!
        NSString *reason = OELocalizedString(@"No Internet Connection", @"Featured Games Blank Slate View Error Info");
        [self setupBoxWithText:reason andImageView:warningImageView];
        [self addLeftHeadlineWithText:OELocalizedString(@"Featured Games", @"")];
        [self addInformationalText:text];
    }
    else
    {
        DLog(@"Unknown Represented Object!");
    }
}
@end
