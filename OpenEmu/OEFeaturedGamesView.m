//
//  OEFeaturedGamesView.m
//  OpenEmu
//
//  Created by Christoph Leimbrock on 10/07/14.
//
//

#import "OEFeaturedGamesView.h"

#import "IKImageBrowserView.h"
#import "IKImageBrowserLayoutManager.h"

@interface IKImageBrowserView ()
- (void)setImageBrowserFlavor:(int)arg1;
- (int)imageBrowserFlavor;
@end
@implementation OEFeaturedGamesView
static int f = 0;
- (void)awakeFromNib
{
    [super awakeFromNib];

    IKImageBrowserLayoutManager *manager = [self layoutManager];
    [manager setAlignLeftForSingleRow:YES];
    [manager setAlignment:1];

    [self setImageBrowserFlavor:f];
}


- (void)drawBackground{}
@end
