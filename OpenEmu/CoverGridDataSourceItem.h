//
//  CoverGridDataSourceItem.h
//  OpenEmuMockup
//
//  Created by Christoph Leimbrock on 10.04.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>

// used for cover grid and cover flow
@protocol CoverGridDataSourceItem <NSObject>
- (NSString*)gridTitle;
- (void)setGridTitle:(NSString*)str;

- (int)gridStatus;

- (void)setGridRating:(NSUInteger)newRating;
- (NSUInteger)gridRating;

- (NSImage*)gridImage;
- (void)setGridImage:(NSImage*)gridImage;
@end
