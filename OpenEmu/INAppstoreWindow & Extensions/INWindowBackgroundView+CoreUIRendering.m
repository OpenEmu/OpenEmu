//
//	INWindowBackgroundView+CoreUIRendering.m
//
//  Copyright (c) 2014 Petroules Corporation. All rights reserved.
//
//  Licensed under the BSD 2-clause License. See LICENSE file distributed in the source
//  code of this project.
//

#import "INAppStoreWindow.h"
#import "INAppStoreWindowCompatibility.h"
#import "INAppStoreWindowSwizzling.h"
#import "INWindowBackgroundView+CoreUIRendering.h"
#import <objc/runtime.h>

#if !defined(INAPPSTOREWINDOW_NO_COREUI) || !INAPPSTOREWINDOW_NO_COREUI

typedef CFTypeRef CUIRendererRef;
typedef void (*_CUIDraw)(CUIRendererRef renderer, CGRect frame, CGContextRef context, CFDictionaryRef options, CFDictionaryRef *result);
static _CUIDraw CUIDraw = 0;

static NSString * const kCUIIsFlippedKey = @"is.flipped";
static NSString * const kCUIStateActive = @"normal";
static NSString * const kCUIStateInactive = @"inactive";
static NSString * const kCUIStateKey = @"state";
static NSString * const kCUIWidgetKey = @"widget";
static NSString * const kCUIWidgetWindowBottomBar = @"kCUIWidgetWindowBottomBar";
static NSString * const kCUIWidgetWindowFrame = @"kCUIWidgetWindowFrame";
static NSString * const kCUIWindowFrameBottomBarHeightKey = @"kCUIWindowFrameBottomBarHeightKey";
static NSString * const kCUIWindowFrameDrawBottomBarSeparatorKey = @"kCUIWindowFrameDrawBottomBarSeparatorKey";
static NSString * const kCUIWindowFrameDrawTitleSeparatorKey = @"kCUIWindowFrameDrawTitleSeparatorKey";
static NSString * const kCUIWindowFrameRoundedBottomCornersKey = @"kCUIWindowFrameRoundedBottomCornersKey";
static NSString * const kCUIWindowFrameRoundedTopCornersKey = @"kCUIWindowFrameRoundedTopCornersKey";
static NSString * const kCUIWindowFrameUnifiedTitleBarHeightKey = @"kCUIWindowFrameUnifiedTitleBarHeightKey";
static NSString * const kCUIWindowTypeDocument = @"regularwin";
static NSString * const kCUIWindowTypeKey = @"windowtype";

@interface NSWindow (NSWindowPrivate)

+ (CUIRendererRef)coreUIRenderer;

@end

@interface /* NSAppearance */ NSObject (NSAppearancePrivate)

- (void)_drawInRect:(CGRect)rect context:(CGContextRef)cgContext options:(CFDictionaryRef)options;

@end

@interface INWindowBackgroundView ()

- (void)drawWindowBackgroundLayersInRect:(NSRect)drawingRect forEdge:(NSRectEdge)drawingEdge showsSeparator:(BOOL)showsSeparator clippingPath:(CGPathRef)clippingPath;

@end

#endif // !defined(INAPPSTOREWINDOW_NO_COREUI) || !INAPPSTOREWINDOW_NO_COREUI

@implementation INWindowBackgroundView (CoreUIRendering)

#if !defined(INAPPSTOREWINDOW_NO_COREUI) || !INAPPSTOREWINDOW_NO_COREUI

+ (void)load
{
	CFBundleRef bundle = CFBundleGetBundleWithIdentifier(CFSTR("com.apple.coreui"));
	if ((CUIDraw = CFBundleGetFunctionPointerForName(bundle, CFSTR("CUIDraw"))) &&
		[NSWindow respondsToSelector:@selector(coreUIRenderer)]) {
		INAppStoreWindowSwizzle(self,
								@selector(drawWindowBackgroundLayersInRect:forEdge:showsSeparator:clippingPath:),
								@selector(drawCUIWindowBackgroundLayersInRect:forEdge:showsSeparator:clippingPath:));

		[self loadConstant:&kCUIIsFlippedKey named:@"kCUIIsFlippedKey" fromBundle:bundle];
		[self loadConstant:&kCUIStateActive named:@"kCUIStateActive" fromBundle:bundle];
		[self loadConstant:&kCUIStateInactive named:@"kCUIStateInactive" fromBundle:bundle];
		[self loadConstant:&kCUIStateKey named:@"kCUIStateKey" fromBundle:bundle];
		[self loadConstant:&kCUIWidgetKey named:@"kCUIWidgetKey" fromBundle:bundle];
		[self loadConstant:&kCUIWidgetWindowBottomBar named:@"kCUIWidgetWindowBottomBar" fromBundle:bundle];
		[self loadConstant:&kCUIWidgetWindowFrame named:@"kCUIWidgetWindowFrame" fromBundle:bundle];
		[self loadConstant:&kCUIWindowFrameBottomBarHeightKey named:@"kCUIWindowFrameBottomBarHeightKey" fromBundle:bundle];
		[self loadConstant:&kCUIWindowFrameDrawBottomBarSeparatorKey named:@"kCUIWindowFrameDrawBottomBarSeparatorKey" fromBundle:bundle];
		[self loadConstant:&kCUIWindowFrameDrawTitleSeparatorKey named:@"kCUIWindowFrameDrawTitleSeparatorKey" fromBundle:bundle];
		[self loadConstant:&kCUIWindowFrameRoundedBottomCornersKey named:@"kCUIWindowFrameRoundedBottomCornersKey" fromBundle:bundle];
		[self loadConstant:&kCUIWindowFrameRoundedTopCornersKey named:@"kCUIWindowFrameRoundedTopCornersKey" fromBundle:bundle];
		[self loadConstant:&kCUIWindowFrameUnifiedTitleBarHeightKey named:@"kCUIWindowFrameUnifiedTitleBarHeightKey" fromBundle:bundle];
		[self loadConstant:&kCUIWindowTypeDocument named:@"kCUIWindowTypeDocument" fromBundle:bundle];
		[self loadConstant:&kCUIWindowTypeKey named:@"kCUIWindowTypeKey" fromBundle:bundle];
	} else {
		NSLog(@"Failed to load CoreUI, falling back to custom drawing");
	}
}

+ (void)loadConstant:(NSString * const *)constant named:(NSString *)constantName fromBundle:(CFBundleRef)bundle
{
	NSString * const *_constant = (NSString * const *)(CFBundleGetDataPointerForName(bundle, (__bridge CFStringRef)(constantName)));
	if (_constant) {
		*((NSString **)constant) = *_constant;
	} else {
		NSLog(@"Failed to load %@, using fallback value %@", constantName, *constant);
	}
}

- (void)drawCUIWindowBackgroundLayersInRect:(CGRect)drawingRect forEdge:(CGRectEdge)edge showsSeparator:(BOOL)showsSeparator clippingPath:(CGPathRef)clippingPath
{
	INAppStoreWindow *window = (INAppStoreWindow *)self.window;
	NSMutableDictionary *options = [NSMutableDictionary dictionaryWithDictionary:
									@{kCUIStateKey: (window.isMainWindow ? kCUIStateActive : kCUIStateInactive),
									  kCUIWidgetKey: kCUIWidgetWindowFrame,
									  kCUIWindowTypeKey: kCUIWindowTypeDocument,
									  kCUIIsFlippedKey: @(self.isFlipped)}];
	if (edge == CGRectMaxYEdge) {
		options[kCUIWindowFrameUnifiedTitleBarHeightKey] = @(window.titleBarHeight + window.toolbarHeight);
		options[kCUIWindowFrameDrawTitleSeparatorKey] = @(showsSeparator);
		options[kCUIWindowFrameRoundedBottomCornersKey] = @(NO);
		options[kCUIWindowFrameRoundedTopCornersKey] = @(YES);
	} else if (edge == CGRectMinYEdge) {
		// NOTE: While kCUIWidgetWindowBottomBar can be used to draw a bottom bar, this only allows
		// it to be drawn with the separator shown as kCUIWindowFrameDrawBottomBarSeparatorKey only
		// applies to the kCUIWidgetWindowFrame widget. So we use that instead; it produces
		// identical results... except in Yosemite where the other options work correctly.
		if (INRunningYosemite()) {
			options[kCUIWidgetKey] = kCUIWidgetWindowBottomBar;
		}
		options[kCUIWindowFrameBottomBarHeightKey] = @(window.bottomBarHeight);
		options[kCUIWindowFrameDrawBottomBarSeparatorKey] = @(showsSeparator);
		options[kCUIWindowFrameRoundedBottomCornersKey] = @(YES);
		options[kCUIWindowFrameRoundedTopCornersKey] = @(NO);
	}

	id appearance = nil;
	if (INRunningMavericks()) {
		Class clazz = NSClassFromString(@"NSAppearance");
		if ([clazz respondsToSelector:@selector(currentAppearance)]) {
			appearance = [clazz performSelector:@selector(currentAppearance)];
		}
	}

	if ([appearance respondsToSelector:@selector(_drawInRect:context:options:)]) {
		[appearance _drawInRect:drawingRect context:[[NSGraphicsContext currentContext] graphicsPort] options:(__bridge CFDictionaryRef)options];
	} else {
		CUIDraw([NSWindow coreUIRenderer], drawingRect, [[NSGraphicsContext currentContext] graphicsPort], (__bridge CFDictionaryRef) options, nil);
	}
}

#endif // !defined(INAPPSTOREWINDOW_NO_COREUI) || !INAPPSTOREWINDOW_NO_COREUI

@end
