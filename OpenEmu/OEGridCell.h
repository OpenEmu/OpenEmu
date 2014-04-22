//
//  OEGridCell.h
//  OpenEmu
//
//  Created by Daniel Nagel on 31.08.13.
//
//

#import <ImageKit/ImageKit.h>
#import "OEGridView.h"
@interface OEGridCell : IKImageBrowserCell
+ (NSImage *)missingArtworkImageWithSize:(NSSize)size;
- (NSImage *)missingArtworkImageWithSize:(NSSize)size;

- (NSRect)ratingFrame;
- (OEGridView*)imageBrowserView;
@end
