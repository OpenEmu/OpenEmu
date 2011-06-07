//
//  OELibraryWindow.h
//  OpenEmuMockup
//
//  Created by Christoph Leimbrock on 08.04.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>

// OELibraryWindow is a window object that replaces the gray line at the lower edge of a window's title bar with a black one
@class OELibraryWindowTitleBarView;
@interface OELibraryWindow : NSWindow {
@private
}

@end

@interface OELibraryWindowTitleBarView : NSView {
@private
}
@end