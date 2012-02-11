//
//  OECoverGridItemView.h
//  OEKitDemo
//
//  Created by Faustino Osuna on 1/31/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import "OEGridViewCell.h"
#import "OECoverGridViewCellRatingLayer.h"
#import "OECoverGridViewCellIndicationLayer.h"

@interface OECoverGridViewCell : OEGridViewCell
{
@private
    NSTimer *_dropDelayedTimer;

    CGRect _titleFrame;
    CGRect _ratingFrame;
    CGRect _imageFrame;

    CGSize _cachedSize;
    OEGridLayer *_proposedImageLayer;
    OEGridLayer *_imageLayer;
    CATextLayer *_titleLayer;
    OECoverGridViewCellRatingLayer *_ratingLayer;

    OECoverGridViewCellIndicationLayer *_statusIndicatorLayer;
    OEGridLayer *_glossyOverlayLayer;
    OEGridLayer *_selectionIndicatorLayer;

    NSImage *_image;

    BOOL _needsLayoutImageAndSelection;
    BOOL _activeSelector;
    OECoverGridViewCellIndicationType _indicationType;

    NSMutableArray *_animationGroupStack;
}

#pragma mark - Properties
@property (nonatomic, retain) NSImage *image;
@property (nonatomic, assign) NSUInteger rating;
@property (nonatomic, copy) NSString *title;
@property (nonatomic, assign) OECoverGridViewCellIndicationType indicationType;

@end
