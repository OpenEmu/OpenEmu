//
//  OEScroller.h
//  OpenEmuMockup
//
//  Created by Christoph Leimbrock on 02.04.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "OEUIDrawingUtils.h"

#define NSScrollerArrowsSingle 3
#define NSScrollerArrowsDoubleBoth 4

#define BoundsArrowSpace 5
#define BoundsArrowNoSpace 0

#define minKnobHeight 20
@interface OEScroller : NSScroller {
@private
	NSScrollArrowPosition scrollArrowPos;
}

- (void)drawKnob;
- (void)drawKnobSlot;
- (void)drawArrows;

- (NSScrollArrowPosition)myArrowsPosition;

#pragma mark -
- (BOOL)autohidesScrollers;
#pragma mark -
- (NSRect)knobSubimageRectForState:(OEUIState)state;
- (NSRect)arrowSubimageRectForState:(OEUIState)state;

@property BOOL isVertical;
@end