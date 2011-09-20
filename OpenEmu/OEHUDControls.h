//
//  OEOSDControls.h
//  OpenEmu
//
//  Created by Christoph Leimbrock on 12.06.11.
//  Copyright 2011 none. All rights reserved.
//

#import <Cocoa/Cocoa.h>
@class OENewGameDocument;
@interface OEHUDControlsWindow : NSWindow{
	OENewGameDocument* gameDocument;
}
- (id)initWithGameDocument:(OENewGameDocument*)doc;
@property (assign) OENewGameDocument* gameDocument;
@end

@interface OEHUDControlsView : NSView {}
- (void)setupControls;
@end
