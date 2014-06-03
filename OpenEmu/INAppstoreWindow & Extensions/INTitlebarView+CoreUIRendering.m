//
//	INTitlebarView+CoreUIRendering.m
//
//  Copyright (c) 2014 Petroules Corporation. All rights reserved.
//
//  Licensed under the BSD 2-clause License. See LICENSE file distributed in the source
//  code of this project.
//

#import "INAppStoreWindow.h"
#import "INAppStoreWindowSwizzling.h"
#import "INTitlebarView+CoreUIRendering.h"

#if !defined(INAPPSTOREWINDOW_NO_COREUI) || !INAPPSTOREWINDOW_NO_COREUI

typedef CFTypeRef CUIRendererRef;
typedef void (*_CUIDraw)(CUIRendererRef renderer, CGRect frame, CGContextRef context, CFDictionaryRef options, CFDictionaryRef *result);
static _CUIDraw CUIDraw = 0;

@interface NSWindow (NSWindowPrivate)

+ (CUIRendererRef)coreUIRenderer;

@end

@interface INTitlebarView ()

- (void)drawWindowBackgroundGradient:(NSRect)drawingRect showsBaselineSeparator:(BOOL)showsBaselineSeparator clippingPath:(CGPathRef)clippingPath;

@end

#endif // !defined(INAPPSTOREWINDOW_NO_COREUI) || !INAPPSTOREWINDOW_NO_COREUI

@implementation INTitlebarView (CoreUIRendering)

#if !defined(INAPPSTOREWINDOW_NO_COREUI) || !INAPPSTOREWINDOW_NO_COREUI

+ (void)load
{
    NSBundle *coreUIBundle = [NSBundle bundleWithIdentifier:@"com.apple.coreui"];
    CFBundleRef bundle = CFBundleCreate(kCFAllocatorDefault, (__bridge CFURLRef)coreUIBundle.bundleURL);
    if ((CUIDraw = CFBundleGetFunctionPointerForName(bundle, (__bridge CFStringRef)@"CUIDraw")) &&
        [NSWindow respondsToSelector:@selector(coreUIRenderer)]) {
        INAppStoreWindowSwizzle(self, @selector(drawWindowBackgroundGradient:showsBaselineSeparator:clippingPath:), @selector(drawCoreUIWindowBackgroundGradient:showsBaselineSeparator:clippingPath:));
    } else {
        NSLog(@"Failed to load CoreUI, falling back to custom drawing");
    }
    CFRelease(bundle);
}

- (void)drawCoreUIWindowBackgroundGradient:(NSRect)drawingRect showsBaselineSeparator:(BOOL)showsBaselineSeparator clippingPath:(CGPathRef)clippingPath
{
    INAppStoreWindow *window = (INAppStoreWindow *)self.window;
    NSDictionary *options = @{@"widget": @"kCUIWidgetWindowFrame",
                              @"state": (window.isMainWindow ? @"normal" : @"inactive"),
                              @"windowtype": @"regularwin",
                              @"kCUIWindowFrameUnifiedTitleBarHeightKey": @(window.titleBarHeight + window.toolbarHeight),
                              @"kCUIWindowFrameDrawTitleSeparatorKey": window.toolbar ? @(window.toolbar.showsBaselineSeparator) : @(window.showsBaselineSeparator),
                              @"is.flipped": @(self.isFlipped)};
    CUIDraw([NSWindow coreUIRenderer], drawingRect, [[NSGraphicsContext currentContext] graphicsPort], (__bridge CFDictionaryRef) options, nil);
}

#endif // !defined(INAPPSTOREWINDOW_NO_COREUI) || !INAPPSTOREWINDOW_NO_COREUI

@end
