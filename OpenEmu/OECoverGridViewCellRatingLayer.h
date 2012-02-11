//
//  OECoverGridViewCellRatingView.h
//  OEKitDemo
//
//  Created by Faustino Osuna on 2/4/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import "OEGridLayer.h"

@interface OECoverGridViewCellRatingLayer : OEGridLayer
{
@private
    NSUInteger _rating;
}

#pragma mark - Rating
@property (nonatomic, readwrite, assign) NSUInteger rating;

@end
