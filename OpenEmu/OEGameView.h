/*
 Copyright (c) 2010, OpenEmu Team

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
     * Neither the name of the OpenEmu Team nor the
       names of its contributors may be used to endorse or promote products
       derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY OpenEmu Team ''AS IS'' AND ANY
 EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL OpenEmu Team BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

@import Cocoa;
#import "OEGameCoreHelper.h"
@class SyphonServer;

extern NSString * const OEScreenshotAspectRationCorrectionDisabled;
extern NSString * const OEDefaultVideoFilterKey;

@protocol OEGameViewDelegate;

@interface OEGameView : NSOpenGLView

@property (nonatomic, weak) id<OEGameViewDelegate> delegate;

@property (copy) NSDictionary *filters;
@property (nonatomic, copy) NSString *filterName;
@property (nonatomic, copy) NSColor  *backgroundColor;

/* Returns a screenshot containing the game viewport with its current size in the window and filters */
- (NSImage *)screenshot;

/* Returns a screenshot as rendered by the emulator core: native size and no filters */
- (NSImage *)nativeScreenshot;

- (void)setEnableVSync:(BOOL)enable;
- (void)setAspectSize:(OEIntSize)newAspectSize;
- (void)setScreenSize:(OEIntSize)newScreenSize withIOSurfaceID:(IOSurfaceID)newSurfaceID;
- (void)setScreenSize:(OEIntSize)newScreenSize aspectSize:(OEIntSize)newAspectSize withIOSurfaceID:(IOSurfaceID)newSurfaceID;

- (void)showQuickSaveNotification;
- (void)showScreenShotNotification;
- (void)showFastForwardNotification:(BOOL)enable;
- (void)showRewindNotification:(BOOL)enable;
- (void)showStepForwardNotification;
- (void)showStepBackwardNotification;

- (NSSize)correctScreenSize:(OEIntSize)screenSize forAspectSize:(OEIntSize)aspectSize returnVertices:(BOOL)flag;
@end

#ifdef SYPHON_SUPPORT
// Syphon support
@interface OEGameView ()
@property (nonatomic, copy)  NSString *syphonTitle;
@property (readonly, strong) SyphonServer *syphonServer;
@end
#endif

@protocol OEGameViewDelegate <NSObject>
- (NSString *)systemIdentifier;
- (void)gameView:(OEGameView *)gameView didReceiveMouseEvent:(OEEvent *)event;
@end
