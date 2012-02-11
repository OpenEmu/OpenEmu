//
//  IKSGridItemLayer.h
//  Sonora
//
//  Created by Indragie Karunaratne on 10-11-15.
//  Copyright 2010 PCWiz Computer. All rights reserved.
//

#import <QuartzCore/QuartzCore.h>
#import "IKSGridViewProtocols.h"
#import "IKSGridView.h"

/**
 Custom CALayer subclass. All grid layers should inherit from this class
 */

@interface IKSGridItemLayer : CALayer <IKSGridItemLayerEventProtocol>
- (NSRect)hitRect;
/**
 Automatically set by the grid view when the selection status of the layer changes. Whenever this value is changed, the layer will be redrawn 
 */
@property (nonatomic, assign, getter=isSelected) BOOL selected;
@property (nonatomic, assign, getter=isFirstResponder) BOOL firstResponder;
@property (nonatomic, weak) IKSGridView *gridView;
@property (assign) NSInteger representedIndex;
@property (strong) id representedObject;

- (void)reloadData;
- (void)beginValueChange;
- (void)endValueChange;
/**
 Called when the layer needs to redraw its content. The default implementation of this method takes the CGContextRef and sets it as the current graphics context (NSGraphicsContext) and calls -drawInRect: 
 @param ctx the context to draw into
 */
//- (void)drawInContext:(CGContextRef)ctx;

/** Override in order to implement custom drawing for the layer
 @param boundsRect the rect to draw into
 */
//- (void)drawRect:(NSRect)boundsRect;

/**
 The keys that the grid view will observe-- when the value for one of these keys changes, the layer will be drawn. Subclasses should override this to return the keys that affect the drawing of the layer
 @returns keys that affect drawing
 */
- (NSArray*)drawingAttributeKeys;
- (NSImage*)dragImage;
@end


