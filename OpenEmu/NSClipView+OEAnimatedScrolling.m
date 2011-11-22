//
//  NSClipView+OEAnimatedScrolling.m
//  OpenEmu
//
//  Created by Carl Leimbrock on 22.11.11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#import "NSClipView+OEAnimatedScrolling.h"

@implementation NSClipView (OEAnimatedScrolling)
- (void)scrollToPoint:(NSPoint)p animated:(BOOL)animFlag
{
    [NSAnimationContext beginGrouping];
    
    p.x -= self.frame.size.width;
    p.y -= self.frame.size.height;
    
    [[self animator] setBoundsOrigin:p];
    
    
    [NSAnimationContext endGrouping];
    
    
}
@end
