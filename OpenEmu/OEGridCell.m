//
//  OEGridCell.m
//  OpenEmu
//
//  Created by Christoph Leimbrock on 15/07/14.
//
//

#import "OEGridCell.h"

@implementation OEGridCell
#pragma mark - Interaction
- (BOOL)isInteractive
{
    return NO;
}

- (BOOL)mouseEntered:(NSEvent *)theEvent
{
    return NO;
}

- (void)mouseMoved:(NSEvent *)theEvent
{}

- (void)mouseExited:(NSEvent*)theEvent
{}

- (BOOL)mouseDown:(NSEvent*)theEvent
{
    return NO;
}

- (void)mouseUp:(NSEvent*)theEvent
{
}

- (NSRect)trackingRect
{
    return NSZeroRect;
}
#pragma mark - Helpers
- (OEGridView*)imageBrowserView
{
    return (OEGridView*)[super imageBrowserView];
}
@end
