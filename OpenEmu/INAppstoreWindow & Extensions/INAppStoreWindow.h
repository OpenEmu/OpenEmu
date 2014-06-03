//
//  INAppStoreWindow.h
//
//  Copyright 2011-2014 Indragie Karunaratne. All rights reserved.
//
//  Licensed under the BSD 2-clause License. See LICENSE file distributed in the source
//  code of this project.
//

#import <Cocoa/Cocoa.h>

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
@property (nonatomic, strong) NSView *titleBarView;

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
@property (nonatomic, strong) INWindowButton *closeButton;

/**
 The button to use as the window's minimize button.
 If this property is nil, the default button will be used.
 */
@property (nonatomic, strong) INWindowButton *minimizeButton;

/**
 The button to use as the window's zoom button.
 If this property is nil, the default button will be used.
 */
@property (nonatomic, strong) INWindowButton *zoomButton;

/**
 The button to use as the window's fullscreen button.
 If this property is nil, the default button will be used.
 */
@property (nonatomic, strong) INWindowButton *fullScreenButton;

/**
 The font used to draw the window's title text.
 */
@property (nonatomic, strong) NSFont *titleFont;

/**
 Gradient used to draw the window's title bar, when the window is main.
 
 If this property is \c nil, the system gradient will be used.
 */
@property (nonatomic, strong) NSGradient *titleBarGradient;

/**
 Color of the separator line between a window's title bar and content area,
 when the window is main.

 If this property is \c nil, the default color will be used.
 */
@property (nonatomic, strong) NSColor *baselineSeparatorColor;

/**
 Color of the window's title text, when the window is main.

 If this property is \c nil, the default color will be used.
 */
@property (nonatomic, strong) NSColor *titleTextColor;

/**
 Drop shadow under the window's title text, when the window is main.

 If this property is \c nil, the default shadow will be used.
 */
@property (nonatomic, strong) NSShadow *titleTextShadow;

/**
 Gradient used to draw the window's title bar, when the window is not main.
 
 If this property is \c nil, the system gradient will be used.
 */
@property (nonatomic, strong) NSGradient *inactiveTitleBarGradient;

/**
 Color of the separator line between a window's title bar and content area,
 when the window is not main.

 If this property is \c nil, the default color will be used.
 */
@property (nonatomic, strong) NSColor *inactiveBaselineSeparatorColor;

/**
 Color of the window's title text, when the window is not main.

 If this property is \c nil, the default color will be used.
 */
@property (nonatomic, strong) NSColor *inactiveTitleTextColor;

/**
 Drop shadow under the window's title text, when the window is not main.

 If this property is \c nil, the default shadow will be used.
 */
@property (nonatomic, strong) NSShadow *inactiveTitleTextShadow;

/**
 Block to override the drawing of the window title bar with a custom implementation.
 */
@property (nonatomic, copy) INAppStoreWindowTitleBarDrawingBlock titleBarDrawingBlock;

/*!
 Default system gradient used to draw a window's title bar.
 @param drawsAsMainWindow \c YES to return the gradient used when the window is drawn in its main
 state, \c NO to return the color used when the window is inactive.
 
 @note This gradient is identical to the one used by AppKit in OS X versions 10.7 through 10.9.
 For OS X 10.6 it is currently an approximation.
 */
+ (NSGradient *)defaultTitleBarGradient:(BOOL)drawsAsMainWindow;

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

/**
 Returns the height of the window's NSToolbar. If the window has no NSToolbar, or if it's not visible, returns 0.
 */
- (CGFloat)toolbarHeight;

@end
