//
//  CoverGridRatingLayer.h
//  GridViewItem Test
//
//  Created by Christoph Leimbrock on 03.05.11.
//  Copyright 2011 none. All rights reserved.
//

#import <Foundation/Foundation.h>


@interface OECoverGridRatingLayer : CALayer {
@private
    int rating;		// holds rating value
	BOOL pressed;	// remembers state
	
}

- (void)setRating:(NSUInteger)rating pressed:(BOOL)pressed;
- (int)rating;

#pragma mark -
#pragma mark Helpers
- (NSImage*)ratingImage;
@end
