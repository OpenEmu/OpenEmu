//
//  OEGameWindow.h
//  popoutwindow
//
//  Created by Christoph Leimbrock on 07.06.11.
//  Copyright 2011 none. All rights reserved.
//

#import <Foundation/Foundation.h>


@interface OEGameWindow : NSWindow {
@private
    NSView* childContentView;
	NSButton *closeButton;

}

@end
@interface OEGameWindowThemeView : NSView {
	BOOL isResizing;
	NSPoint lastMouseLocation;
}
@end
