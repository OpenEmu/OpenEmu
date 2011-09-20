//
//  OEGameWindow.m
//  popoutwindow
//
//  Created by Christoph Leimbrock on 07.06.11.
//  Copyright 2011 none. All rights reserved.
//

#import "OEHUDWindow.h"
#import "NSImage+OEDrawingAdditions.h"

#import "OEImageButton.h"
#import "NSColor+IKSAdditions.h"
@interface OEHUDWindow (Private)
- (void)_initialSetup;
@end
@interface OEHUDBorderWindow : NSWindow
@end
@implementation OEHUDWindow

+ (void)initialize{
	NSImage* img = [NSImage imageNamed:@"hud_window"];
	
	[img setName:@"hud_window_active" forSubimageInRect:NSMakeRect(0, 0, img.size.width/2, img.size.height)];
	[img setName:@"hud_window_inactive" forSubimageInRect:NSMakeRect(img.size.width/2, 0, img.size.width/2, img.size.height)];
}
#pragma mark -
- (id)initWithContentRect:(NSRect)contentRect
				styleMask:(NSUInteger)windowStyle
				  backing:(NSBackingStoreType)bufferingType
					defer:(BOOL)deferCreation
{
	self = [super
			initWithContentRect:contentRect
			styleMask:NSBorderlessWindowMask|NSResizableWindowMask
			backing:bufferingType
			defer:deferCreation];
	
	if (self){
		[self _initialSetup];		
	}
    return self;
}

- (id)initWithContentRect:(NSRect)frame {
    self = [self initWithContentRect:frame styleMask:NSBorderlessWindowMask backing:NSWindowBackingLocationDefault defer:NO];
    if (self) {}
    return self;
}

- (void)awakeFromNib{
	[super awakeFromNib];
	[self _initialSetup];
}

- (void)dealloc {
	[_borderWindow release];
	_borderWindow = nil;
	
    [super dealloc];
}
#pragma mark -
#pragma mark Private
- (void)_initialSetup{
	[self setHasShadow:NO];
	[self setOpaque:NO];
	[self setBackgroundColor:[NSColor clearColor]];
	
	NSRect frame;
	frame.size = self.frame.size;
	frame.origin = NSMakePoint(0, 0);
	[super setContentView:[[[NSView alloc] initWithFrame:NSZeroRect] autorelease]];
	[self setContentView:[[[NSView alloc] initWithFrame:NSZeroRect] autorelease]];
	// Register for notifications
	NSNotificationCenter* nc = [NSNotificationCenter defaultCenter];
	[nc addObserver:self selector:@selector(_layout) name:NSWindowDidResizeNotification object:self];

	[nc addObserver:self selector:@selector(_layout) name:NSWindowDidResignKeyNotification object:self];
	[nc addObserver:self selector:@selector(_layout) name:NSWindowDidBecomeKeyNotification object:self];
		
	_borderWindow = [[OEHUDBorderWindow alloc] init];
	[self addChildWindow:_borderWindow ordered:NSWindowAbove];
	[_borderWindow orderFront:self];
}

- (void)_layout{
	[_borderWindow setFrame:self.frame display:YES];
	[_borderWindow display];
}

- (id)contentView{
	return [[[super contentView] subviews] lastObject];
}

- (void)setContentView:(NSView *)aView{
	NSView* contentView = [[[super contentView] subviews] lastObject];

	if(contentView)[contentView removeFromSuperview];
	
	NSView* actualContentView = [super contentView];
	[actualContentView addSubview:aView];
	
	
	NSRect contentRect;
	contentRect.origin = NSMakePoint(0, 0);
	contentRect.size = self.frame.size;
	
	contentRect = NSInsetRect(contentRect, 1, 1);
	contentRect.size.height -= 21;
	
	[aView setFrame:contentRect];
	[aView setAutoresizingMask:NSViewWidthSizable|NSViewHeightSizable];
}
#pragma mark -
#pragma mark NSWindow Overrides
- (BOOL)canBecomeKeyWindow{
	return YES;
}

- (BOOL)canBecomeMainWindow{
	return YES;
}

@end

@implementation OEHUDBorderWindow
- (id)init{
	self = [self initWithContentRect:NSZeroRect styleMask:NSBorderlessWindowMask backing:NSWindowBackingLocationDefault defer:NO];
	if(self){		
		[self setHasShadow:NO];
		[self setMovableByWindowBackground:NO];
		
		[self setOpaque:NO];
		[self setBackgroundColor:[NSColor clearColor]];
		
		NSView* borderView = [[OEHUDWindowThemeView alloc] initWithFrame:NSZeroRect];
		[super setContentView:borderView];
//		[[self contentView] addSubview:borderView];
// 		[borderView setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
		[borderView release];
	}

	return self;
}

- (void)setContentView:(NSView *)aView{}

- (void)display{
	NSLog(@"displaay called on some part of the hud window");
	[[self contentView] display];
}
- (void)setParentWindow:(NSWindow *)window{
	[super setParentWindow:window];
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
	
    [super dealloc];
}

- (BOOL)canBecomeKeyWindow{
	return NO;
}

- (BOOL)canBecomeMainWindow{
	return NO;
}
@end

@implementation OEHUDWindowThemeView

- (id)init {
    self = [super init];
    if (self) {
    }
    return self;
}
- (id)initWithCoder:(NSCoder *)coder {
    self = [super initWithCoder:coder];
    if (self) {
    }
    return self;
}
- (id)initWithFrame:(NSRect)frame {
    self = [super initWithFrame:frame];
    if (self) {
    }
    return self;
}
#pragma mark -
- (BOOL)isOpaque{
	return NO;
}

- (NSRect)resizeRect{
	// unused // const CGFloat resizeBoxSize = 11.0;
	// unused // const CGFloat contentViewPadding = 2.0;
	
	return NSMakeRect([self bounds].size.width-11, 0, 11, 11);
}

- (NSRect)titleBarRect{
	NSRect titleBarRect = [self bounds];
	
	titleBarRect.size.height = 22;
	titleBarRect.origin.y = [self bounds].size.height-titleBarRect.size.height;
		
	return titleBarRect;
}

- (void)drawRect:(NSRect)dirtyRect{
	[[NSColor clearColor] setFill];
	NSRectFill([self bounds]);
	
	BOOL isFocused = [[self window].parentWindow isMainWindow] && [NSApp isActive];
	NSImage* borderImage = isFocused ? [NSImage imageNamed:@"hud_window_active"] : [NSImage imageNamed:@"hud_window_inactive"];
	[borderImage drawInRect:self.bounds fromRect:NSZeroRect operation:NSCompositeSourceOver/*NSCompositeSourceOver*/ fraction:1.0 respectFlipped:TRUE hints:nil leftBorder:14 rightBorder:14 topBorder:23 bottomBorder:23];
	NSLog(@"isFocused: %d", isFocused);
	
	NSMutableDictionary* titleAttribtues = [NSMutableDictionary dictionary];
	
	NSMutableParagraphStyle* ps = [[NSMutableParagraphStyle alloc] init];
	[ps setLineBreakMode:NSLineBreakByTruncatingMiddle];
	[ps setAlignment:NSCenterTextAlignment];
	[titleAttribtues setObject:ps forKey:NSParagraphStyleAttributeName];
	[ps release];
	
	NSColor* textColor = isFocused ? [NSColor colorWithDeviceWhite:0.86 alpha:1.0] : [NSColor colorWithDeviceWhite:0.61 alpha:1.0];
	NSFont* font = [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:0 weight:2.0 size:13.0];
	NSShadow* shadow = [[NSShadow alloc] init];
	[shadow setShadowColor:[NSColor colorWithDeviceRed:0.129 green:0.129 blue:0.129 alpha:1.0]];
	[shadow setShadowBlurRadius:1.0];
	[shadow setShadowOffset:NSMakeSize(0, 1)];
	
	[titleAttribtues setObject:textColor forKey:NSForegroundColorAttributeName];
	[titleAttribtues setObject:font forKey:NSFontAttributeName];
	[titleAttribtues setObject:shadow forKey:NSShadowAttributeName];
	[shadow release];
	
	NSRect titleBarRect = NSInsetRect([self titleBarRect], 10, 0);
	titleBarRect.origin.y -= 2;
	
	NSString* windowTitle = [[self window].parentWindow title];
	NSAttributedString* attributedWindowTitle = [[NSAttributedString alloc] initWithString:windowTitle attributes:titleAttribtues];
	[attributedWindowTitle drawInRect:titleBarRect];
	[attributedWindowTitle release];
}
/*
- (void)mouseDown:(NSEvent *)theEvent{
	NSPoint pointInView = [self convertPoint:[theEvent locationInWindow] fromView:nil];
	
	lastMouseLocation = NSZeroPoint;
	
	isResizing = NO;
	
	if(!isResizing && !NSPointInRect(pointInView, [self titleBarRect])){
		[[self nextResponder] mouseDown:theEvent];
		return;
	}
	
	NSWindow *window = [self window];
	lastMouseLocation = [window convertBaseToScreen:[theEvent locationInWindow]];
}

- (void)mouseDragged:(NSEvent *)theEvent{
	if(NSEqualPoints(lastMouseLocation, NSZeroPoint)){
		[[self nextResponder] mouseDragged:theEvent];		
		return;
	}
	
	NSWindow *window = [self window];
	NSPoint newMousePosition = [window convertBaseToScreen:[theEvent locationInWindow]];
	
	NSPoint delta = NSMakePoint(newMousePosition.x-lastMouseLocation.x, newMousePosition.y-lastMouseLocation.y);
	
	if(isResizing){
		NSRect frame = [[self window] frame];
		
		frame.size.width += delta.x;
		frame.size.height -= delta.y;
		frame.origin.y += delta.y;
		
		[[self window] setFrame:frame display:TRUE];
	} else {
		NSPoint frameOrigin = [[self window] frame].origin;
		
		frameOrigin.x += delta.x;
		frameOrigin.y += delta.y;
	
		[[self window] setFrameOrigin:frameOrigin];
	}
	
	
	lastMouseLocation = newMousePosition;
}

- (void)mouseUp:(NSEvent *)theEvent{	
	if(NSEqualPoints(lastMouseLocation, NSZeroPoint)){
		[[self nextResponder] mouseUp:theEvent];		
		return;
	}
	
	isResizing = NO;
}
 */
@end
