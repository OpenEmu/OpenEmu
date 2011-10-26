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

@class OELibraryDatabase;
@protocol IKSGridViewDataSource <NSObject>
- (NSUInteger)numberOfItemsInGridView:(IKSGridView*)aView;

- (void)gridView:(IKSGridView *)aView setObject:(id)obj forKey:(NSString*)key withRepresentedObject:(id)obj;
- (id)gridView:(IKSGridView *)aView objectValueForKey:(NSString*)key withRepresentedObject:(id)obj;

@optional
- (id)gridView:(IKSGridView*)aView objectValueOfItemAtIndex:(NSUInteger)index;
- (void)gridView:(IKSGridView *)aView setObject:(id)obj forKey:(NSString*)key atIndex:(NSUInteger)index;
- (id)gridView:(IKSGridView *)aView objectValueForKey:(NSString*)key atIndex:(NSUInteger)index;

@end