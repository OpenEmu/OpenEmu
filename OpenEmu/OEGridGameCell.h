//
//  OEGridCell.h
//  OpenEmu
//
//  Created by Daniel Nagel on 31.08.13.
//
//

#import "OEGridCell.h"

@interface OEGridGameCell : OEGridCell
- (NSRect)ratingFrame;

- (void)updateTextLayer;
@end
