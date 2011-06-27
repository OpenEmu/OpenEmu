//
//  OEGameWindow.h
//  popoutwindow
//
//  Created by Christoph Leimbrock on 07.06.11.
//  Copyright 2011 none. All rights reserved.
//

#import <Foundation/Foundation.h>


@interface OEHUDWindow : NSWindow {
@private
    NSView* childContentView;
	NSButton *closeButton;

}

@end
@interface OEHUDWindowThemeView : NSView {
	BOOL isResizing;
	NSPoint lastMouseLocation;
}
@end
