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

NSString * const OEShowNotificationsKey = @"OEShowNotifications";

typedef NS_ENUM(NSInteger, OEGameViewNotificationName) {
    OENotificationNone,
    OENotificationQuickSave,
    OENotificationScreenShot,
    OENotificationFastForward,
    OENotificationRewind,
    OENotificationStepForward,
    OENotificationStepBackward,

    OENotificationCount
};

@interface OEGameViewNotification : NSObject
@property (readonly) OEGameViewNotificationName type;
@property (readonly) GLuint texture;
@property (readonly) NSTimeInterval seconds;
@property BOOL state;

+ (instancetype)notificationWithNotification:(OEGameViewNotificationName)notification texture:(GLuint)texture seconds:(NSTimeInterval)seconds;
@end

@interface OEGameViewNotificationRenderer ()
@property GLuint quickSaveTexture, screenShotTexture, fastForwardTexture, rewindTexture, stepForwardTexture, stepBackwardTexture, textureToRender;
@property NSTimeInterval visibleTimeInSeconds, lastNotificationTime;
@property OEGameViewNotification *notificationQuickSave, *notificationScreenShot, *notificationFastForward, *notificationRewind, *notificationStepForward, *notificationStepBackward;
@end

@implementation OEGameViewNotificationRenderer
+ (void)initialize
{
    if([self class] == [OEGameViewNotificationRenderer class])
    {
        [[NSUserDefaults standardUserDefaults] registerDefaults:@{ OEShowNotificationsKey:@(YES) }];
    }
}

- (void)setupInContext:(NSOpenGLContext*)context
{
    CGLContextObj cgl_ctx = [context CGLContextObj];

    _quickSaveTexture   = [self loadImageNamed:@"hud_quicksave_notification" inContext:cgl_ctx];
    _screenShotTexture  = [self loadImageNamed:@"hud_screenshot_notification" inContext:cgl_ctx];
    _fastForwardTexture = [self loadImageNamed:@"hud_fastforward_notification" inContext:cgl_ctx];
    _rewindTexture      = [self loadImageNamed:@"hud_rewind_notification" inContext:cgl_ctx];
    _stepForwardTexture = [self loadImageNamed:@"hud_stepforward_notification" inContext:cgl_ctx];
    _stepBackwardTexture = [self loadImageNamed:@"hud_stepbackward_notification" inContext:cgl_ctx];
    _textureToRender = 0;

    _visibleTimeInSeconds = 0.0;
    _lastNotificationTime = 0.0;

    _notificationQuickSave   = [OEGameViewNotification notificationWithNotification:OENotificationQuickSave
                                                                            texture:_quickSaveTexture
                                                                            seconds:1.25];
    _notificationScreenShot  = [OEGameViewNotification notificationWithNotification:OENotificationScreenShot
                                                                            texture:_screenShotTexture
                                                                            seconds:1.25];
    _notificationFastForward = [OEGameViewNotification notificationWithNotification:OENotificationFastForward
                                                                            texture:_fastForwardTexture
                                                                            seconds:DBL_MAX];
    _notificationRewind      = [OEGameViewNotification notificationWithNotification:OENotificationRewind
                                                                            texture:_rewindTexture
                                                                            seconds:DBL_MAX];
    _notificationStepForward = [OEGameViewNotification notificationWithNotification:OENotificationStepForward
                                                                            texture:_stepForwardTexture
                                                                            seconds:1.25];
    _notificationStepBackward = [OEGameViewNotification notificationWithNotification:OENotificationStepBackward
                                                                            texture:_stepBackwardTexture
                                                                            seconds:1.25];

    _scaleFactor = 1.0;

    _disableNotifications = false;
}

- (void)render
{
    if(![[NSUserDefaults standardUserDefaults] boolForKey:OEShowNotificationsKey]) return;

    const static NSTimeInterval fadeIn  = 0.25;
    const static NSTimeInterval fadeOut = 0.25;

    if(_disableNotifications) return;

    NSTimeInterval notificationDuration = fadeIn+_visibleTimeInSeconds+fadeOut;
    NSTimeInterval now = [NSDate timeIntervalSinceReferenceDate];
    NSTimeInterval notificationTimeDifference  = now - _lastNotificationTime;

    if(notificationTimeDifference < notificationDuration)
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

        double textureAlpha = 1.0;
        if(notificationTimeDifference < _visibleTimeInSeconds)
            textureAlpha = notificationTimeDifference/fadeIn;
        else if(notificationTimeDifference >= fadeIn+_visibleTimeInSeconds)
            textureAlpha = 1.0 - (notificationTimeDifference-fadeIn-_visibleTimeInSeconds) / fadeOut;
        [self OE_renderTexture:_textureToRender withAlpha:textureAlpha inRect:rect];

        glBindTexture(GL_TEXTURE_2D, 0);
        glDisable(GL_TEXTURE_2D);

        glDisable(GL_BLEND);
        glEnable(GL_TEXTURE_RECTANGLE_EXT);
    }
    else
        [self OE_resumeNotification];
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

- (void)OE_showNotification:(OEGameViewNotification *)notification
{
    if (![notification isKindOfClass:[OEGameViewNotification class]]) return;

    // Only need to set for key-repeating notifications
    if (notification.seconds == DBL_MAX)
        notification.state = YES;

    // Rewind + Fast Forward = Fast Rewind
    BOOL isRewinding = _notificationRewind.state;
    if ((notification.type == OENotificationFastForward) && isRewinding) return;

    _textureToRender = notification.texture;
    _visibleTimeInSeconds = notification.seconds;
    _lastNotificationTime = [NSDate timeIntervalSinceReferenceDate];
}

- (void)OE_hideNotification:(OEGameViewNotification *)notification
{
    if (![notification isKindOfClass:[OEGameViewNotification class]]) return;

    notification.state = NO;

    // Rewind + Fast Forward = Fast Rewind
    BOOL isRewinding = _notificationRewind.state;
    if ((notification.type == OENotificationFastForward) && isRewinding) return;

    _textureToRender = notification.texture;
    _visibleTimeInSeconds = 0.0;
    _lastNotificationTime = [NSDate timeIntervalSinceReferenceDate];

    [self OE_resumeNotification];
}

- (void)OE_resumeNotification
{
    // E.g. resume fast forward notification after rewind ends if fast forward is still pressed
    BOOL isFastForwarding = _notificationFastForward.state;
    BOOL isRewinding = _notificationRewind.state;
    if (isFastForwarding)
        [self OE_showNotification:_notificationFastForward];
    if (isRewinding)
        [self OE_showNotification:_notificationRewind];
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

    if(_fastForwardTexture)
    {
        glDeleteTextures(1, &_fastForwardTexture);
        _fastForwardTexture = 0;
    }

    if(_rewindTexture)
    {
        glDeleteTextures(1, &_rewindTexture);
        _rewindTexture = 0;
    }

    if(_stepForwardTexture)
    {
        glDeleteTextures(1, &_stepForwardTexture);
        _stepForwardTexture = 0;
    }

    if(_stepBackwardTexture)
    {
        glDeleteTextures(1, &_stepBackwardTexture);
        _stepBackwardTexture = 0;
    }
}

- (void)showQuickStateNotification
{
    [self OE_showNotification:_notificationQuickSave];
}

- (void)showScreenShotNotification
{
    [self OE_showNotification:_notificationScreenShot];
}

- (void)showFastForwardNotification:(BOOL)enable
{
    BOOL isFastForwarding = _notificationFastForward.state;
    if (enable && isFastForwarding) return;

    if (enable) {
        [self OE_showNotification:_notificationFastForward];
    }
    else {
        [self OE_hideNotification:_notificationFastForward];
    }
}

- (void)showRewindNotification:(BOOL)enable
{
    BOOL isRewinding = _notificationRewind.state;
    if (enable && isRewinding) return;

    if (enable) {
        [self OE_showNotification:_notificationRewind];
    }
    else {
        [self OE_hideNotification:_notificationRewind];
    }
}

- (void)showStepForwardNotification
{
    [self OE_showNotification:_notificationStepForward];
}

- (void)showStepBackwardNotification
{
    [self OE_showNotification:_notificationStepBackward];
}

- (GLuint)loadImageNamed:(NSString*)name inContext:(CGLContextObj)cgl_ctx
{
    NSURL *imageURL = [[NSBundle mainBundle] URLForImageResource:name];
    NSAssert(imageURL, ([NSString stringWithFormat:@"Notification image \"%@\" not reachable.", name]));
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

@implementation OEGameViewNotification

+ (instancetype)notificationWithNotification:(OEGameViewNotificationName)notification texture:(GLuint)texture seconds:(NSTimeInterval)seconds
{
    OEGameViewNotification *newNotification = [self new];
    newNotification->_type    = notification;
    newNotification->_texture = texture;
    newNotification->_seconds = seconds;
    newNotification->_state   = NO;
    return newNotification;
}

@end
