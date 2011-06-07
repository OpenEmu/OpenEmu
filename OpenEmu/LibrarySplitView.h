//
//  LibrarySplitView.h
//  OpenEmu
//
//  Created by Christoph Leimbrock on 30.03.11.
//  Copyright 2011 none. All rights reserved.
//

#import <Foundation/Foundation.h>


@interface LibrarySplitView : NSSplitView <NSSplitViewDelegate>{
    BOOL resizesLeftView;
	BOOL _hidingLeftView;
	
	NSPoint lastMousePoint;
}

@property BOOL resizesLeftView;

- (void)setSplitterPosition:(CGFloat)newPosition animated:(BOOL)animatedFlag;
- (float)splitterPosition;
@end
