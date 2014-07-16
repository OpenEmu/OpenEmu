//
//  OEGridCell.h
//  OpenEmu
//
//  Created by Christoph Leimbrock on 15/07/14.
//
//

#import <ImageKit/ImageKit.h>
#import "OEGridView.h"

@interface OEGridCell : IKImageBrowserCell
- (BOOL)isInteractive;
- (OEGridView*)imageBrowserView;

- (BOOL)mouseEntered:(NSEvent *)theEvent;
- (BOOL)mouseMoved:(NSEvent *)theEvent;
- (void)mouseExited:(NSEvent*)theEvent;
- (BOOL)mouseDown:(NSEvent*)theEvent;
- (void)mouseUp:(NSEvent*)theEvent;
- (NSRect)trackingRect;
@end
