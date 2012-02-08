//
//  IKSGridItemLayer.m
//  Sonora
//
//  Created by Indragie Karunaratne on 10-11-15.
//  Copyright 2010 PCWiz Computer. All rights reserved.
//

#import "IKSGridItemLayer.h"

@implementation IKSGridItemLayer
@synthesize selected, firstResponder, gridView;

- (NSRect)hitRect
{
    return [self frame];
}

- (void)setSelected:(BOOL)newSelected
{
    if (selected != newSelected) 
    {
        selected = newSelected;
        return;
    }
}

- (void)setFirstResponder:(BOOL)newFirstResponder
{
    if (firstResponder != newFirstResponder)
    {
        firstResponder = newFirstResponder;
        if (self.selected) 
        {
            return; [self setNeedsLayout]; 
        }
    }
}

- (NSArray*)drawingAttributeKeys
{
    // Overriden by subclasses
    return nil;
}
#pragma mark -
- (NSImage*)dragImage
{
    return [[NSImage alloc] init];
}
#pragma mark -
- (void)beginValueChange
{
    // called before changing represented object
}
- (void)endValueChange
{
    // called after changing represented object
}
- (void)reloadData
{}
#pragma mark -
#pragma mark IKSGridItemLayerEventProtocol
- (BOOL)mouseDown:(NSEvent*)theEvent
{
    return NO;
}

- (BOOL)mouseDragged:(NSEvent*)theEvent
{
    return NO;
}

- (BOOL)mouseEntered:(NSEvent*)theEvent
{
    return NO;
}

- (BOOL)mouseExited:(NSEvent*)theEvent
{
    return NO;
}

- (BOOL)mouseMoved:(NSEvent*)theEvent
{
    return NO;
}

- (BOOL)mouseUp:(NSEvent*)theEvent
{
    return NO;
}
#pragma mark -
#pragma mark IKSGridItemLayerEventProtocol
- (NSDragOperation)draggingEntered:(id < NSDraggingInfo >)sender
{
    return NSDragOperationNone;
}
- (void)draggingExited:(id < NSDraggingInfo >)sender
{
}
- (NSDragOperation)draggingUpdated:(id < NSDraggingInfo >)sender
{
    return NSDragOperationNone;
}
- (BOOL)performDragOperation:(id < NSDraggingInfo >)sender
{
    return NO;
}
@synthesize representedIndex, representedObject;
@end
