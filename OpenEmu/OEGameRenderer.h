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

@property (nonatomic, readonly) id presentationFramebuffer;

- (void)updateRenderer;           // gameCore, ioSurface or gameCore.screenRect changed

// Properties
- (BOOL)canChangeBufferSize;      ///!< If the app can safely ask the IOSurface to change size if the window resizes.

// Execution
- (void)willExecuteFrame;
- (void)didExecuteFrame;

- (void)presentDoubleBufferedFBO;
- (void)willRenderFrameOnAlternateThread;
- (void)didRenderFrameOnAlternateThread;

- (void)suspendFPSLimiting;       ///!< (Temporarily) disable the FPS limiter for saving/setup, to avoid deadlock.
- (void)resumeFPSLimiting;        ///!< Resume the FPS limiter when entering normal gameplay.

@end
