/*
 Copyright (c) 2009, OpenEmu Team
 
 
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

#import <Cocoa/Cocoa.h>
#import <Quartz/Quartz.h>
#import <OpenGL/OpenGL.h>
#import <OpenGL/gl.h>

#import "OEGameCoreHelper.h"
#import "OEGameShader.h"

@class GameCore, GameDocument, GameDocumentController;

@interface OEGameLayer : CAOpenGLLayer <OEGameCoreHelperDelegate>
{
    // from our gameDocument - where we get our IOSurface refs from:
    id<OEGameCoreHelper> rootProxy;
    
    NSString             *filterName;
    NSView               *ownerView;
    
    // for rendering
    CGLContextObj         layerContext;
    CGLContextObj         sharedLayerContext;
    
    GLuint gameTexture;
    
    // replace our basic scaling filters with straight up shaders
    NSDictionary         *filters;
    
    BOOL                  usesShader;
    BOOL                  vSyncEnabled;
    
    QCRenderer           *filterRenderer;
    
    CGColorSpaceRef       ntscColorSpace;
    CGColorSpaceRef       rgbColorSpace;
    
    // for QCRenderer
    NSTimeInterval        startTime; // time for rendering, input to time based effects, sources, 
    NSTimeInterval        time;
    
    BOOL                  filterHasOutputMousePositionKeys;
    
    // Screenshot
    
    void (^screenshotHandler)(NSImage *img);
    
    CIImage *gameCIImage;
    CGSize cachedTextureSize;
    
    IOSurfaceID surfaceID;
    OEIntSize screenSize;
}

@property(copy) void (^screenshotHandler)(NSImage *img);

@property(nonatomic,assign) id<OEGameCoreHelper> rootProxy;

@property         BOOL          vSyncEnabled;
@property(assign) NSView       *ownerView;
@property(retain) NSString     *filterName;
@property(retain) CIImage      *gameCIImage;

- (CGFloat)preferredWindowScale;

@end
