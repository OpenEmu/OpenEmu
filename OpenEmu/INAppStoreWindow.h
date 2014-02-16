//
//  INAppStoreWindow.h
//
//  Copyright 2011-2014 Indragie Karunaratne. All rights reserved.
//
//  Licensed under the BSD 2-clause License. See LICENSE file distributed in the source
//  code of this project.
//

#import <Cocoa/Cocoa.h>

#if __has_feature(objc_arc)
#define INAppStoreWindowStrong strong
#define INAppStoreWindowBridge __bridge
#else
#define INAppStoreWindowStrong retain
#define INAppStoreWindowBridge
#endif

@class INWindowButton;

/**
 Draws a default style OS X title bar.
 */
@interface INTitlebarView : NSView

@end

/**
 Creates a window similar to the Mac App Store window, with centered traffic lights and an
 enlarged title bar. This does not handle creating the toolbar.
 */
@interface INAppStoreWindow : NSWindow

/**
 Prototype for a block used to implement custom drawing code for a window's title bar.
 @param drawsAsMainWindow Whether the window should be drawn in main state.
 @param drawingRect Drawing area of the window's title bar.
 @param clippingPath Path to clip drawing according to window's rounded corners.
 */
typedef void (^INAppStoreWindowTitleBarDrawingBlock)(BOOL drawsAsMainWindow,
													 CGRect drawingRect, CGPathRef clippingPath);

/**
 The height of the title bar. By default, this is set to the standard title bar height.
 */
@property (nonatomic) CGFloat titleBarHeight;

/**
 Container view for custom views added to the title bar.
 
 Add subviews to this view that you want to show in the title bar (e.g. buttons, a toolbar, etc.).
 This view can also be set if you want to use a different style title bar from the default one
 (textured, etc.).
 */
@property (nonatomic, INAppStoreWindowStrong) NSView *titleBarView;

/**
 Whether the fullscreen button is vertically centered.
 */
@property (nonatomic) BOOL centerFullScreenButton;

/**
 Whether the traffic light buttons are vertically centered.
 */
@property (nonatomic) BOOL centerTrafficLightButtons;

/**
 Whether the traffic light buttons are displayed in vertical orientation.
 */
@property (nonatomic) BOOL verticalTrafficLightButtons;

/**
 Whether the title is centered vertically.
 */
@property (nonatomic) BOOL verticallyCenterTitle;

/**
 Whether to hide the title bar in fullscreen mode.
 */
@property (nonatomic) BOOL hideTitleBarInFullScreen;

/**
 Whether to display the baseline separator between the window's title bar and content area.
 */
@property (nonatomic) BOOL showsBaselineSeparator;

/**
 Distance between the traffic light buttons and the left edge of the window.
 */
@property (nonatomic) CGFloat trafficLightButtonsLeftMargin;

/**
 * Distance between the traffic light buttons and the top edge of the window.
 */
@property (nonatomic) CGFloat trafficLightButtonsTopMargin;

/**
 Distance between the fullscreen button and the right edge of the window.
 */
@property (nonatomic) CGFloat fullScreenButtonRightMargin;

/**
 Distance between the fullscreen button and the top edge of the window.
 */
@property (nonatomic) CGFloat fullScreenButtonTopMargin;

/**
 Spacing between the traffic light buttons.
 */
@property (nonatomic) CGFloat trafficLightSeparation;

/**
 Number of points in any direction above which the window will be allowed to reposition.
 A Higher value indicates coarser movements but much reduced CPU overhead. Defaults to 1.
 */
@property (nonatomic) CGFloat mouseDragDetectionThreshold;

/**
 Whether to show the window's title text. If \c YES, the title will be shown even if
 \a titleBarDrawingBlock is set. To draw the title manually, set this property to \c NO
 and draw the title using \a titleBarDrawingBlock.
 */
@property (nonatomic) BOOL showsTitle;

/**
 Whether to show the window's title text in fullscreen mode.
 */
@property (nonatomic) BOOL showsTitleInFullscreen;

/**
 Whether the window displays the document proxy icon (for document-based applications).
 */
@property (nonatomic) BOOL showsDocumentProxyIcon;

/**
 The button to use as the window's close button.
 If this property is nil, the default button will be used.
 */
@property (nonatomic, INAppStoreWindowStrong) INWindowButton *closeButton;

/**
 The button to use as the window's minimize button.
 If this property is nil, the default button will be used.
 */
@property (nonatomic, INAppStoreWindowStrong) INWindowButton *minimizeButton;

/**
 The button to use as the window's zoom button.
 If this property is nil, the default button will be used.
 */
@property (nonatomic, INAppStoreWindowStrong) INWindowButton *zoomButton;

/**
 The button to use as the window's fullscreen button.
 If this property is nil, the default button will be used.
 */
@property (nonatomic, INAppStoreWindowStrong) INWindowButton *fullScreenButton;

/**
 The font used to draw the window's title text.
 */
@property (nonatomic, INAppStoreWindowStrong) NSFont *titleFont;

/**
 Starting (top) color of the window's title bar gradient, when the window is main.
 
 If this property is \c nil, the default color will be used.
 */
@property (nonatomic, INAppStoreWindowStrong) NSColor *titleBarStartColor;

/**
 Ending (bottom) color of the window's title bar gradient, when the window is main.
 
 If this property is \c nil, the default color will be used.
 */
@property (nonatomic, INAppStoreWindowStrong) NSColor *titleBarEndColor;

/**
 Color of the separator line between a window's title bar and content area,
 when the window is main.
 
 If this property is \c nil, the default color will be used.
 */
@property (nonatomic, INAppStoreWindowStrong) NSColor *baselineSeparatorColor;

/**
 Color of the window's title text, when the window is main.
 
 If this property is \c nil, the default color will be used.
 */
@property (nonatomic, INAppStoreWindowStrong) NSColor *titleTextColor;

/**
 Drop shadow under the window's title text, when the window is main.
 
 If this property is \c nil, the default shadow will be used.
 */
@property (nonatomic, INAppStoreWindowStrong) NSShadow *titleTextShadow;

/**
 Starting (top) color of the window's title bar gradient, when the window is not main.
 
 If this property is \c nil, the default color will be used.
 */
@property (nonatomic, INAppStoreWindowStrong) NSColor *inactiveTitleBarStartColor;

/**
 Ending (bottom) color of the window's title bar gradient, when the window is not main.
 
 If this property is \c nil, the default color will be used.
 */
@property (nonatomic, INAppStoreWindowStrong) NSColor *inactiveTitleBarEndColor;

/**
 Color of the separator line between a window's title bar and content area,
 when the window is not main.
 
 If this property is \c nil, the default color will be used.
 */
@property (nonatomic, INAppStoreWindowStrong) NSColor *inactiveBaselineSeparatorColor;

/**
 Color of the window's title text, when the window is not main.
 
 If this property is \c nil, the default color will be used.
 */
@property (nonatomic, INAppStoreWindowStrong) NSColor *inactiveTitleTextColor;

/**
 Drop shadow under the window's title text, when the window is not main.
 
 If this property is \c nil, the default shadow will be used.
 */
@property (nonatomic, INAppStoreWindowStrong) NSShadow *inactiveTitleTextShadow;

/**
 Block to override the drawing of the window title bar with a custom implementation.
 */
@property (nonatomic, copy) INAppStoreWindowTitleBarDrawingBlock titleBarDrawingBlock;

/*!
 Default system color of the starting (top) color of a window's title bar gradient.
 @param drawsAsMainWindow \c YES to return the color used when the window is drawn in its main
 state, \c NO to return the color used when the window is inactive.
 
 @note This color may be an approximation and is subject to change at any time.
 */
+ (NSColor *)defaultTitleBarStartColor:(BOOL)drawsAsMainWindow;

/*!
 Default system color of the ending (bottom) color of a window's title bar gradient.
 @param drawsAsMainWindow \c YES to return the color used when the window is drawn in its main
 state, \c NO to return the color used when the window is inactive.
 
 @note This color may be an approximation and is subject to change at any time.
 */
+ (NSColor *)defaultTitleBarEndColor:(BOOL)drawsAsMainWindow;

/*!
 Default system color of the separator line between a window's title bar and content area.
 @param drawsAsMainWindow \c YES to return the color used when the window is drawn in its main
 state, \c NO to return the color used when the window is inactive.
 
 @note This color may be an approximation and is subject to change at any time.
 */
+ (NSColor *)defaultBaselineSeparatorColor:(BOOL)drawsAsMainWindow;

/*!
 Default system color of a window's title text.
 @param drawsAsMainWindow \c YES to return the color used when the window is drawn in its main
 state, \c NO to return the color used when the window is inactive.
 
 @note This color may be an approximation and is subject to change at any time.
 */
+ (NSColor *)defaultTitleTextColor:(BOOL)drawsAsMainWindow;

/**
 Sets the height of the title bar. By default, this is set to the standard title bar height.
 
 @param adjustWindowFrame Whether to adjust the window frame in response to the change in
 the title bar height. By default, the window frame is adjusted when the title bar height
 is changed.
 */
- (void)setTitleBarHeight:(CGFloat)titleBarHeight adjustWindowFrame:(BOOL)adjustWindowFrame;

@end
