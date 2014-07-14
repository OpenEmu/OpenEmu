//
//  OEScrollDummyView.m
//  OpenEmu
//
//  Created by Christoph Leimbrock on 15/07/14.
//
//

#import "OEScrollDummyView.h"

@implementation OEScrollDummyView

- (void)scrollWheel:(NSEvent *)theEvent
{
    [[self nextResponder] scrollWheel:theEvent];
}

- (BOOL)canBecomeKeyView
{
    return NO;
}

- (BOOL)acceptsFirstResponder
{
    return NO;
}

- (BOOL)hasHorizontalScroller
{
    return NO;
}
- (BOOL)hasVerticalScroller
{
    return NO;
}
@end
