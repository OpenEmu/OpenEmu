//
//  OELibraryWindow.h
//  OpenEmuMockup
//
//  Created by Christoph Leimbrock on 08.04.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "NSWindow+OECustomWindow.h"

// OELibraryWindow is a window object that replaces the gray line at the lower edge of a window's title bar with a black one
@class OEMainWindowTitleBarView;
@interface OEMainWindow : NSWindow <OECustomWindow>
{
    NSView* mainContentView;
}

- (void)setMainContentView:(NSView*)view;
- (NSView*)mainContentView;
@end

@interface OEMainWindowTitleBarView : NSView 
@end

@interface OEMainWindowContentView : NSView 
@end