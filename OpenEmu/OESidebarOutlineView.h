//
//  SidebarOutlineView.h
//  SidebarViewTest
//
//  Created by Christoph Leimbrock on 16.05.11.
//  Copyright 2011 none. All rights reserved.
//

#import <Foundation/Foundation.h>

@protocol OEDraggingDestinationDelegate <NSObject>
- (NSDragOperation)draggingEntered:(id < NSDraggingInfo >)sender;
- (NSDragOperation)draggingUpdated:(id < NSDraggingInfo >)sender;
- (void)draggingEnded:(id < NSDraggingInfo >)sender;
- (void)draggingExited:(id < NSDraggingInfo >)sender;

- (BOOL)prepareForDragOperation:(id < NSDraggingInfo >)sender;
- (BOOL)performDragOperation:(id < NSDraggingInfo >)sender;
- (void)concludeDragOperation:(id < NSDraggingInfo >)sender;
@end

@interface OESidebarOutlineView : NSOutlineView <NSDraggingDestination>
@property (assign) id <OEDraggingDestinationDelegate> dragDelegate;
@end

@interface OESidebarOutlineView (OEPrivate)
- (void)setupOutlineCell;
@end