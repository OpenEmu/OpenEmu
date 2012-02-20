//
//  OEInteractiveLayer.m
//  OEGridViewDemo
//
//  Created by Faustino Osuna on 2/10/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import "OEGridLayer.h"

@implementation OEGridLayer

- (id)init
{
    if(!(self = [super init]))
        return nil;

    [self setLayoutManager:[OEGridViewLayoutManager layoutManager]];
    [self setNeedsDisplayOnBoundsChange:YES];

    return self;
}

- (CALayer *)hitTest:(CGPoint)p
{
    if(!_interactive)
        return nil;

    if(CGRectContainsPoint([self frame], p))
    {
        CALayer *result = [super hitTest:p];
        return (result ?: self);
    }

    return nil;
}

- (id<CAAction>)actionForKey:(NSString *)event
{
    return nil;
}

- (void)layoutSublayers
{
    if([[self delegate] respondsToSelector:@selector(layoutSublayers)])
        [[self delegate] layoutSublayers];
}

- (void)mouseDownAtPointInLayer:(NSPoint)point withEvent:(NSEvent *)theEvent
{
}

- (void)mouseUpAtPointInLayer:(NSPoint)point withEvent:(NSEvent *)theEvent
{
}

- (void)mouseMovedAtPointInLayer:(NSPoint)point withEvent:(NSEvent *)theEvent
{
}

- (NSDragOperation)draggingEntered:(id<NSDraggingInfo>)sender
{
    return NSDragOperationNone;
}

- (void)draggingExited:(id<NSDraggingInfo>)sender
{
}

- (NSDragOperation)draggingUpdated:(id<NSDraggingInfo>)sender
{
    return NSDragOperationNone;
}

- (BOOL)performDragOperation:(id<NSDraggingInfo>)sender
{
    return NO;
}

- (void)willMoveToSuperlayer:(OEGridLayer *)superlayer
{
}

- (void)didMoveToSuperlayer
{
}

- (void)addSublayer:(CALayer *)layer
{
    if([layer isKindOfClass:[OEGridLayer class]])
    {
        [(OEGridLayer *)layer willMoveToSuperlayer:self];
        [super addSublayer:layer];
        [(OEGridLayer *)layer didMoveToSuperlayer];
    }
    else
        [super addSublayer:layer];
}

- (void)insertSublayer:(CALayer *)layer atIndex:(unsigned int)idx
{
    if([layer isKindOfClass:[OEGridLayer class]])
    {
        [(OEGridLayer *)layer willMoveToSuperlayer:self];
        [super insertSublayer:layer atIndex:idx];
        [(OEGridLayer *)layer didMoveToSuperlayer];
    }
    else
        [super insertSublayer:layer atIndex:idx];
}

- (void)removeFromSuperlayer
{
    [self willMoveToSuperlayer:nil];
    [super removeFromSuperlayer];
    [self didMoveToSuperlayer];
}

#pragma mark - Properties
@synthesize tracking = _tracking;

- (NSWindow *)window
{
    return [[self view] window];;
}

- (NSView *)view
{
    CALayer *superlayer = self;
    while(superlayer)
    {
        NSView *delegate = [superlayer delegate];
        if([delegate isKindOfClass:[NSView class]])
            return delegate;

        superlayer = [superlayer superlayer];
    }
    return nil;
}

@synthesize interactive = _interactive;

@end
