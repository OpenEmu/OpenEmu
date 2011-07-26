//
//  LibrarySplitView.h
//  OpenEmu
//
//  Created by Christoph Leimbrock on 30.03.11.
//  Copyright 2011 none. All rights reserved.
//

#import <Foundation/Foundation.h>
// TODO: rewrite this whole splitview thingy, create an independent one for List-/FlowView

@interface LibrarySplitView : NSSplitView <NSSplitViewDelegate>{
    BOOL resizesLeftView;
	BOOL _hidingLeftView;
	
	NSPoint lastMousePoint;
	
	BOOL drawsWindowResizer;
}

// returns view on the right/left that can be used wthout changeing toolbar background
- (NSView*)rightContentView;
- (NSView*)leftContentView;

- (void)replaceLeftContentViewWithView:(NSView*)contentView animated:(BOOL)animationFlag;
- (void)replaceRightContentViewWithView:(NSView*)contentView animated:(BOOL)animationFlag;
@property BOOL resizesLeftView;
@property BOOL drawsWindowResizer;

- (void)setSplitterPosition:(CGFloat)newPosition animated:(BOOL)animatedFlag;
- (float)splitterPosition;
@end
