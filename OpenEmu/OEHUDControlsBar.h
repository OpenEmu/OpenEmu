//
//  OEOSDControls.h
//  OpenEmu
//
//  Created by Christoph Leimbrock on 12.06.11.
//  Copyright 2011 none. All rights reserved.
//

#import <Cocoa/Cocoa.h>
@class OEGameViewController;
@interface OEHUDControlsBarWindow : NSWindow
{
    NSTimer* fadeTimer;
    id       eventMonitor;
    NSDate*  lastMouseMovement;
}

- (id)initWithGameViewController:(OEGameViewController*)controller;

- (void)show;
- (void)hide;
@property (retain, nonatomic) NSDate* lastMouseMovement;
@property (assign) OEGameViewController* gameViewController;
@end
@class OEHUDSlider;
@interface OEHUDControlsBarView : NSView 
{
    OEHUDSlider* slider;
}

@property (readonly) OEHUDSlider* slider;

- (void)setupControls;
@end
