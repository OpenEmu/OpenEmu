//
//  OEHorizontalSplitView.h
//  OpenEmu
//
//  Created by Christoph Leimbrock on 25.06.11.
//  Copyright 2011 none. All rights reserved.
//

#import <Foundation/Foundation.h>


@interface OEHorizontalSplitView : NSSplitView <NSSplitViewDelegate>{
@private
    BOOL resizesLeftView;
    
    NSPoint lastMousePoint;
    
}
- (void)setSplitterPosition:(CGFloat)newPosition animated:(BOOL)animatedFlag;
- (float)splitterPosition;
@property BOOL resizesLeftView;
@end
