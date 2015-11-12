//
//  OEGridCell.h
//  OpenEmu
//
//  Created by Daniel Nagel on 31.08.13.
//
//

#import "OEGridCell.h"

@interface OEGridGameCell : OEGridCell
+ (NSImage *)missingArtworkImageWithSize:(NSSize)size;
- (NSImage *)missingArtworkImageWithSize:(NSSize)size;

- (NSRect)ratingFrame;
@end
