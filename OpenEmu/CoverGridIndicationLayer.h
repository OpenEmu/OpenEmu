//
//  CoverGridIndicationLayer.h
//  OpenEmuMockup
//
//  Created by Christoph Leimbrock on 02.05.11.
//  Copyright 2011 none. All rights reserved.
//

#import <Foundation/Foundation.h>


@interface CoverGridIndicationLayer : CALayer {
@private
	int type;
}

- (void)setType:(int)newType;
- (NSImage*)fileMissingImage;

@property (assign, readonly) int type;
@end
