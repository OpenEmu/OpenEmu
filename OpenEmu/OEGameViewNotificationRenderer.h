//
//  OEGameViewNotificationRenderer.h
//  OpenEmu
//
//  Created by Christoph Leimbrock on 8/12/15.
//
//

#import <Foundation/Foundation.h>

extern NSString * const OEShowSaveStateNotificationKey;
extern NSString * const OEShowScreenShotNotificationKey;

@interface OEGameViewNotificationRenderer : NSObject
- (void)setupInContext:(NSOpenGLContext*)context;
- (void)render;
- (void)cleanUp;

- (void)showQuickStateNotification;
- (void)showScreenShotNotification;

@property GLfloat scaleFactor;
@property BOOL disableNotifications;
#pragma mark - for lazynes reasons
@property CGRect bounds;
@property OEIntSize aspectSize;
@end
