//
//  IKSGridItemLayerEventsProtocl.h
//  Sonora
//
//  Created by Indragie Karunaratne on 11-01-30.
//  Copyright 2011 PCWiz Computer. All rights reserved.
//
@class IKSGridItemLayer;
@class IKSGridView;
@protocol IKSGridItemLayerEventProtocol
@optional
- (BOOL)mouseDown:(NSEvent*)theEvent;
- (BOOL)mouseUp:(NSEvent*)theEvent;
- (BOOL)mouseDragged:(NSEvent*)theEvent;
- (BOOL)mouseEntered:(NSEvent*)theEvent;
- (BOOL)mouseExited:(NSEvent*)theEvent;
- (BOOL)mouseMoved:(NSEvent*)theEvent;
@end


@protocol IKSGridItemLayerDragProtocol
- (NSDragOperation)draggingEntered:(id < NSDraggingInfo >)sender;
- (void)draggingExited:(id < NSDraggingInfo >)sender;
- (NSDragOperation)draggingUpdated:(id < NSDraggingInfo >)sender;

- (BOOL)performDragOperation:(id < NSDraggingInfo >)sender;
@end


@protocol IKSGridViewDrag <NSObject>
- (BOOL)gridItemBrowser:(id)browser shouldDragItem:(id)item;
@end


@protocol IKSGridViewDataSource <NSObject>
- (NSUInteger)numberOfItemsInGridView:(IKSGridView*)aView;
- (id)gridView:(IKSGridView*)aView objectValueOfItemAtIndex:(NSUInteger)index;
@end