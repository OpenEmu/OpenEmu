/*
 Copyright (c) 2015, OpenEmu Team
 
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

#import "OEGameViewNotificationRenderer.h"
#import <OpenGL/gl.h>

NSString * const OEShowSaveStateNotificationKey = @"OEShowSaveStateNotification";
NSString * const OEShowScreenShotNotificationKey = @"OEShowScreenShotNotification";

@interface OEGameViewNotificationRenderer ()
@property GLuint quickSaveTexture, screenShotTexture;
@property NSTimeInterval lastQuickSave, lastScreenShot;
@end

@implementation OEGameViewNotificationRenderer
+ (void)initialize
{
    if([self class] == [OEGameViewNotificationRenderer class])
    {
        [[NSUserDefaults standardUserDefaults] registerDefaults:@{ OEShowSaveStateNotificationKey:@(YES),
                                                                   OEShowScreenShotNotificationKey:@(YES)
                                                                   }];
    }
}

- (void)setupInContext:(NSOpenGLContext*)context
{
    CGLContextObj cgl_ctx = [context CGLContextObj];

    _quickSaveTexture  = [self loadImageNamed:@"hud_quicksave_notification" inContext:cgl_ctx];
    _screenShotTexture = [self loadImageNamed:@"hud_screenshot_notification" inContext:cgl_ctx];

    _lastQuickSave = _lastScreenShot = 0;
    _scaleFactor = 1.0;

    _disableNotifications = false;
}

- (void)render
{
    const static NSTimeInterval fadeIn  = 0.25;
    const static NSTimeInterval visible = 1.25;
    const static NSTimeInterval fadeOut = 0.25;
    const static NSTimeInterval duration = fadeIn+visible+fadeOut;

    if(_disableNotifications) return;

    NSTimeInterval now = [NSDate timeIntervalSinceReferenceDate];
    NSTimeInterval quickSaveDifference  = now - _lastQuickSave;
    NSTimeInterval screenShotDifference = now - _lastScreenShot;

    if(quickSaveDifference < duration || screenShotDifference < duration)
    {
        glDisable(GL_TEXTURE_RECTANGLE_EXT);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glEnable(GL_TEXTURE_2D);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

        const OEIntSize   aspectSize     = _aspectSize;
        const NSSize      viewSize       = [self bounds].size;
        const OEIntSize viewIntSize = (OEIntSize){viewSize.width, viewSize.height};
        const OEIntSize textureIntSize = [self OE_correctTextureSize:viewIntSize forAspectSize:aspectSize];
        const NSRect  bounds = [self bounds];

        const NSSize  imageSize   = {56.0, 56.0};
        const CGPoint imageOrigin = {20.0, 20.0};

        const NSRect rect = (NSRect){{-1.0* (textureIntSize.width-imageOrigin.x)/NSWidth(bounds), (textureIntSize.height-imageSize.height-imageOrigin.y)/NSHeight(bounds)}, {imageSize.width/NSWidth(bounds),imageSize.height/NSHeight(bounds)}};

        double quickSaveAlpha = 1.0;
        if(quickSaveDifference < visible)
            quickSaveAlpha = quickSaveDifference/fadeIn;
        else if(quickSaveDifference >= fadeIn+visible)
            quickSaveAlpha = 1.0 - (quickSaveDifference-fadeIn-visible) / fadeOut;
        [self OE_renderTexture:_quickSaveTexture withAlpha:quickSaveAlpha inRect:rect];

        double screenShotAlpha = 1.0;
        if(screenShotDifference < visible)
            screenShotAlpha = screenShotDifference/fadeIn;
        else if(screenShotDifference >= fadeIn+visible)
            screenShotAlpha = 1.0 - (screenShotDifference-fadeIn-visible) / fadeOut;
        [self OE_renderTexture:_screenShotTexture withAlpha:screenShotAlpha inRect:rect];

        glBindTexture(GL_TEXTURE_2D, 0);
        glDisable(GL_TEXTURE_2D);

        glDisable(GL_BLEND);
        glEnable(GL_TEXTURE_RECTANGLE_EXT);
    }
}

- (void)OE_renderTexture:(GLuint)texture withAlpha:(CGFloat)alpha inRect:(NSRect)rect {
    glColor4f(alpha, alpha, alpha, alpha);
    glBindTexture(GL_TEXTURE_2D, texture);

    glBegin(GL_QUADS);
    glTexCoord2f(0.0, 0.0); glVertex2d(NSMinX(rect), NSMinY(rect));
    glTexCoord2f(0.0, 1.0); glVertex2d(NSMinX(rect), NSMaxY(rect));
    glTexCoord2f(1.0, 1.0); glVertex2d(NSMaxX(rect), NSMaxY(rect));
    glTexCoord2f(1.0, 0.0); glVertex2d(NSMaxX(rect), NSMinY(rect));
    glEnd();

}

- (void)cleanUp
{
    if(_quickSaveTexture)
    {
        glDeleteTextures(1, &_quickSaveTexture);
        _quickSaveTexture = 0;
    }

    if(_screenShotTexture)
    {
        glDeleteTextures(1, &_screenShotTexture);
        _screenShotTexture = 0;
    }
}

- (void)showQuickStateNotification
{
    if([[NSUserDefaults standardUserDefaults] boolForKey:OEShowSaveStateNotificationKey])
        _lastQuickSave = [NSDate timeIntervalSinceReferenceDate];
}

- (void)showScreenShotNotification
{
    if([[NSUserDefaults standardUserDefaults] boolForKey:OEShowScreenShotNotificationKey])
        _lastScreenShot = [NSDate timeIntervalSinceReferenceDate];
}

- (GLuint)loadImageNamed:(NSString*)name inContext:(CGLContextObj)cgl_ctx
{
    NSURL *imageURL = [[NSBundle mainBundle] URLForImageResource:name];
    CGImageSourceRef imageSource = CGImageSourceCreateWithURL((__bridge CFURLRef)imageURL, NULL);

    size_t index = MIN(_scaleFactor > 1.0 ? 0 : 1, CGImageSourceGetCount(imageSource) -1);

    CGImageRef image = CGImageSourceCreateImageAtIndex(imageSource, index, NULL);
    CFRelease(imageSource);
    size_t width  = CGImageGetWidth(image);
    size_t height = CGImageGetHeight(image);
    CGRect rect = CGRectMake(0.0f, 0.0f, width, height);

    void *imageData = malloc(width * height * 4);
    CGColorSpaceRef colourSpace = CGColorSpaceCreateDeviceRGB();
    CGContextRef ctx = CGBitmapContextCreate(imageData, width, height, 8, width * 4, colourSpace, kCGBitmapByteOrder32Host | kCGImageAlphaPremultipliedFirst);
    CFRelease(colourSpace);
    CGContextTranslateCTM(ctx, 0, height);
    CGContextScaleCTM(ctx, 1.0f, -1.0f);
    CGContextSetBlendMode(ctx, kCGBlendModeCopy);
    CGContextDrawImage(ctx, rect, image);
    CGContextRelease(ctx);
    CFRelease(image);

    GLuint texture;

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glPixelStorei(GL_UNPACK_ROW_LENGTH, (GLint)width);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, (int)width, (int)height, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, imageData);
    free(imageData);

    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    return texture;
}

- (OEIntSize)OE_correctTextureSize:(OEIntSize)textureSize forAspectSize:(OEIntSize)aspectSize
{
    // calculate aspect ratio
    float     wr             = (CGFloat) textureSize.width / aspectSize.width;
    float     hr             = (CGFloat) textureSize.height / aspectSize.height;
    OEIntSize textureIntSize = (wr > hr ?
                                (OEIntSize){hr * aspectSize.width, textureSize.height      } :
                                (OEIntSize){textureSize.width    , wr * aspectSize.height  });

    return textureIntSize;
}

@end
