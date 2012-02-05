//
//  OERatingCell.h
//  OpenEmuMockup
//
//  Created by Christoph Leimbrock on 20.04.11.
//  Copyright 2011 none. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "OEUIDrawingUtils.h"

@interface OERatingCell : NSCell {
@private
}

- (NSRect)imageRectForValue:(int)rating andState:(OEUIState)state;
- (void)determineRatingForPos:(NSPoint)pos inTableView:(NSTableView*)view;
@end
