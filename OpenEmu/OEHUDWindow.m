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
@interface OEHUDWindow (Private)
- (void)_setup;
@end
@implementation OEHUDWindow

+ (void)initialize{
	NSImage* img = [NSImage imageNamed:@"hud_window"];
	
	[[img setName:@"hud_window_active" forSubimageInRect:NSMakeRect(0, 0, img.size.width/2, img.size.height)] setFlipped:TRUE];
	[[img setName:@"hud_window_inactive" forSubimageInRect:NSMakeRect(img.size.width/2, 0, img.size.width/2, img.size.height)] setFlipped:TRUE];
}

- (id)initWithContentRect:(NSRect)contentRect
				styleMask:(NSUInteger)windowStyle
				  backing:(NSBackingStoreType)bufferingType
					defer:(BOOL)deferCreation
{
	self = [super
			initWithContentRect:contentRect
			styleMask:NSBorderlessWindowMask
			backing:bufferingType
			defer:deferCreation];
	if (self)
	{
		[self setOpaque:NO];
		[self setBackgroundColor:[NSColor clearColor]];
		
		[[NSNotificationCenter defaultCenter]
		 addObserver:self
		 selector:@selector(mainWindowChanged:)
		 name:NSWindowDidBecomeMainNotification
		 object:self];
		
		[[NSNotificationCenter defaultCenter]
		 addObserver:self
		 selector:@selector(mainWindowChanged:)
		 name:NSWindowDidResignMainNotification
		 object:self];
		
		

	}
	return self;
}

//
// dealloc
//
// Releases instance memory.
//
- (void)dealloc
{
	[[NSNotificationCenter defaultCenter] removeObserver:self];
	[super dealloc];
}

//
// setContentSize:
//
// Convert from childContentView to frameView for size.
//
- (void)setContentSize:(NSSize)newSize
{
	NSSize sizeDelta = newSize;
	NSSize childBoundsSize = [childContentView bounds].size;
	sizeDelta.width -= childBoundsSize.width;
	sizeDelta.height -= childBoundsSize.height;
	
	OEHUDWindowThemeView *frameView = [super contentView];
	NSSize newFrameSize = [frameView bounds].size;
	newFrameSize.width += sizeDelta.width;
	newFrameSize.height += sizeDelta.height;
	
	[super setContentSize:newFrameSize];
}

//
// mainWindowChanged:
//
// Redraw the close button when the main window status changes.
//
- (void)mainWindowChanged:(NSNotification *)aNotification{
	[[super contentView] display];
}

//
// setContentView:
//
// Keep our frame view as the content view and make the specified "aView"
// the child of that.
//
- (void)setContentView:(NSView *)aView
{
	if ([childContentView isEqualTo:aView])
	{
		return;
	}
	
	NSRect bounds = [self frame];
	bounds.origin = NSZeroPoint;
	
	NSView *frameView = [super contentView];
	if (!frameView)
	{
		frameView = [[[NSView alloc] initWithFrame:bounds] autorelease];
		
		[super setContentView:frameView];
		
		OEHUDWindowThemeView* themeView = [[[OEHUDWindowThemeView alloc] initWithFrame:bounds] autorelease];
		[frameView addSubview:themeView];
		[themeView setAutoresizingMask:NSViewWidthSizable|NSViewHeightSizable];
		[themeView setFrame:frameView.bounds];

		closeButton = [[OEImageButton alloc] init];
		OEImageButtonCell* pushCell = [[OEToolbarButtonPushCell alloc] init];
		
		[closeButton setCell:pushCell];
		[pushCell setImage:[NSImage imageNamed:@"hud_close_button"]];
		[pushCell release];
		
		[closeButton setAction:@selector(close)];
		[closeButton setTarget:self];
				
		[closeButton setFrame:NSMakeRect(1, bounds.size.height-22, 21, 21)];
		[closeButton setAutoresizingMask:NSViewMaxXMargin | NSViewMinYMargin];
		[frameView addSubview:closeButton];
	}
	
	if (childContentView)
	{
		[childContentView removeFromSuperview];
	}
	childContentView = aView;
	[childContentView setFrame:[self contentRectForFrameRect:bounds]];
	[childContentView setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
	[frameView addSubview:childContentView positioned:NSWindowBelow relativeTo:[[frameView subviews] objectAtIndex:0]];
}

//
// contentView
//
// Returns the child of our frame view instead of our frame view.
//
- (NSView *)contentView
{
	return childContentView;
}

//
// canBecomeKeyWindow
//
// Overrides the default to allow a borderless window to be the key window.
//
- (BOOL)canBecomeKeyWindow
{
	return YES;
}

//
// canBecomeMainWindow
//
// Overrides the default to allow a borderless window to be the main window.
//
- (BOOL)canBecomeMainWindow
{
	return YES;
}

//
// contentRectForFrameRect:
//
// Returns the rect for the content rect, taking the frame.
//
- (NSRect)contentRectForFrameRect:(NSRect)windowFrame
{
	windowFrame.origin = NSZeroPoint;
	
	windowFrame = NSInsetRect(windowFrame, 1, 1);
	windowFrame.size.height -= 21;
	
	return windowFrame;
}

//
// frameRectForContentRect:styleMask:
//
// Ensure that the window is make the appropriate amount bigger than the content.
//
+ (NSRect)frameRectForContentRect:(NSRect)windowContentRect styleMask:(NSUInteger)windowStyle
{
	windowContentRect = NSInsetRect(windowContentRect, -1, -1);
	windowContentRect.size.height += 21;
	
	return windowContentRect;
}

- (void)setTitle:(NSString *)aString{
	[super setTitle:aString];
	[[super contentView] display];
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
- (NSRect)resizeRect{
	const CGFloat resizeBoxSize = 11.0;
	const CGFloat contentViewPadding = 2.0;
	
	return NSMakeRect([self bounds].size.width-11, 0, 11, 11);
}

- (NSRect)titleBarRect{
	NSRect titleBarRect = [self bounds];
	
	titleBarRect.size.height = 22;
	titleBarRect.origin.y = [self bounds].size.height-titleBarRect.size.height;
		
	return titleBarRect;

}

- (void)drawRect:(NSRect)dirtyRect{
	BOOL isFocued = [[self window] isMainWindow] && [NSApp isActive];
	NSImage* borderImage = isFocued ? [NSImage imageNamed:@"hud_window_active"] : [NSImage imageNamed:@"hud_window_inactive"];
	[borderImage drawInRect:self.bounds fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:TRUE hints:nil leftBorder:14 rightBorder:14 topBorder:23 bottomBorder:23];

	
	NSMutableDictionary* titleAttribtues = [NSMutableDictionary dictionary];
	
	NSMutableParagraphStyle* ps = [[[NSMutableParagraphStyle alloc] init] autorelease];
	[ps setLineBreakMode:NSLineBreakByTruncatingMiddle];
	[ps setAlignment:NSCenterTextAlignment];
	[titleAttribtues setObject:ps forKey:NSParagraphStyleAttributeName];
	
	NSColor* textColor = isFocued ? [NSColor colorWithDeviceWhite:0.86 alpha:1.0] : [NSColor colorWithDeviceWhite:0.61 alpha:1.0];
	NSFont* font = [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:0 weight:2.0 size:13.0];
	NSShadow* shadow = [[[NSShadow alloc] init] autorelease];
	[shadow setShadowColor:[NSColor colorWithDeviceRed:0.129 green:0.129 blue:0.129 alpha:1.0]];
	[shadow setShadowBlurRadius:1.0];
	[shadow setShadowOffset:NSMakeSize(0, 1)];
	
	[titleAttribtues setObject:textColor forKey:NSForegroundColorAttributeName];
	[titleAttribtues setObject:font forKey:NSFontAttributeName];
	[titleAttribtues setObject:shadow forKey:NSShadowAttributeName];
	
	NSRect titleBarRect = NSInsetRect([self titleBarRect], 10, 0);
	titleBarRect.origin.y -= 2;
	
	NSString* windowTitle = [[self window] title];
	NSAttributedString* attributedWindowTitle = [[NSAttributedString alloc] initWithString:windowTitle attributes:titleAttribtues];
	[attributedWindowTitle drawInRect:titleBarRect];
	[attributedWindowTitle release];
}

- (void)mouseDown:(NSEvent *)theEvent{
	NSPoint pointInView = [self convertPoint:[theEvent locationInWindow] fromView:nil];
	
	lastMouseLocation = NSZeroPoint;
	
	isResizing = NO;
	if (NSPointInRect(pointInView, [self resizeRect])){
		isResizing = YES;
	}
	
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
@end
