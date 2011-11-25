//
//  NSWindow+OECustomWindow.h
//  OpenEmu
//
//  Created by Carl Leimbrock on 25.11.11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#import <AppKit/AppKit.h>

@protocol OECustomWindow <NSObject>
- (BOOL)drawsAboveDefaultThemeFrame;
- (void)drawThemeFrame:(NSValue*)dirtyRectValue;
@end

@interface NSWindow (OECustomWindow)
+ (void)registerWindowClassForCustomThemeFrameDrawing:(Class)windowClass;
@end
