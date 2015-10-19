//
//  OEGameCoreRenderer.h
//  OpenEmu
//
//  Created by Alexander Strange on 10/18/15.
//
//

#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>

@class OEGameCore;
@protocol OERenderDelegate;

@protocol OEGameRenderer

// Setup
@property (nonatomic) OEGameCore   *gameCore;
@property (nonatomic) IOSurfaceRef  ioSurface;
@property (nonatomic) OEIntSize     surfaceSize;

- (void)updateRenderer; // gameCore, ioSurface or gameCore.screenRect changed

// Properties
- (BOOL)hasAlternateThread;
- (BOOL)canChangeBufferSize; ///!< If the app can ask the IOSurface to change size if the window resizes

// Execution
- (void)willExecuteFrame;
- (void)didExecuteFrame;
- (void)willRenderOnAlternateThread;
- (void)startRenderingOnAlternateThread;
- (void)willRenderFrameOnAlternateThread;
- (void)didRenderFrameOnAlternateThread;

@end
