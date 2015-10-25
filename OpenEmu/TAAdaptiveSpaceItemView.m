//
//  TAAdaptiveSpaceItemView.m
//  TAAdaptiveSpaceItem
//
//  Created by Timothy Armes on 17/02/2014.
//  Copyright (c) 2014 Timothy Armes. All rights reserved.
//

#import "TAAdaptiveSpaceItemView.h"
#import "TAAdaptiveSpaceItem.h"

@implementation TAAdaptiveSpaceItemView

- (BOOL)acceptsFirstMouse:(NSEvent *)theEvent
{
    return NO;
}

- (void)viewDidMoveToWindow
{
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(windowResized:) name:NSWindowDidResizeNotification object:[self window]];
}

- (void)dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}

- (void)windowResized:(NSNotification *)notification;
{
    [_adaptiveSpaceItem updateWidth];
}


@end
