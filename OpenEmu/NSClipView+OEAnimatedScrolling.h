//
//  NSClipView+OEAnimatedScrolling.h
//  OpenEmu
//
//  Created by Carl Leimbrock on 22.11.11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#import <AppKit/AppKit.h>

@interface NSClipView (OEAnimatedScrolling)
- (void)scrollToPoint:(NSPoint)p animated:(BOOL)animFlag;
@end
