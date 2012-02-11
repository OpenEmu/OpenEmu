//
//  OEInteractiveLayer.h
//  OEGridViewDemo
//
//  Created by Faustino Osuna on 2/10/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import <QuartzCore/QuartzCore.h>
#import "OEGridViewLayoutManager.h"

@interface OEGridLayer : CALayer <OEGridViewLayoutManagerProtocol>
{
@public
    BOOL _tracking;
    BOOL _interactive;
}

- (void)mouseDownAtPointInLayer:(NSPoint)point withEvent:(NSEvent *)theEvent;
- (void)mouseUpAtPointInLayer:(NSPoint)point withEvent:(NSEvent *)theEvent;
- (void)mouseMovedAtPointInLayer:(NSPoint)point withEvent:(NSEvent *)theEvent;

- (NSDragOperation)draggingEntered:(id<NSDraggingInfo>)sender;
- (void)draggingExited:(id<NSDraggingInfo>)sender;
- (NSDragOperation)draggingUpdated:(id<NSDraggingInfo>)sender;
- (BOOL)performDragOperation:(id<NSDraggingInfo>)sender;

- (void)willMoveToSuperlayer:(OEGridLayer *)superlayer;
- (void)didMoveToSuperlayer;

#pragma mark - Properties
@property (nonatomic, assign, getter = isTracking) BOOL tracking;
@property (nonatomic, readonly) NSWindow *window;
@property (nonatomic, readonly) NSView *view;
@property (nonatomic, assign, getter = isInteractive) BOOL interactive;

@end
