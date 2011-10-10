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
    NSTimer* fadeTimer;
    id       eventMonitor;
    NSDate*  lastMouseMovement;
}

- (id)initWithGameDocument:(OENewGameDocument*)doc;

- (void)show;
- (void)hide;
@property (assign) OENewGameDocument* gameDocument;
@property (retain, nonatomic) NSDate* lastMouseMovement;
@end

@interface OEHUDControlsView : NSView {}
- (void)setupControls;
@end
