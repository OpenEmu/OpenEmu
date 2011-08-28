//
//  OENSMenu.m
//  OEPreferencesMockup
//
//  Created by Christoph Leimbrock on 04.06.11.
//  Copyright 2011 none. All rights reserved.
//

#import "OEMenu.h"
#import "OEMenuView.h"
#import "NSImage+OEDrawingAdditions.h"
@interface OEMenu (Private)
- (void)_performcloseMenu;
@end

@implementation OEMenu
@synthesize menu, supermenu, visible, minSize, maxSize, btn, delegate;

+ (void)initialize{
	NSImage* menuArrows = [NSImage imageNamed:@"dark_menu_popover_arrow"];
	[menuArrows setName:@"dark_menu_popover_arrow_normal" forSubimageInRect:NSMakeRect(0, menuArrows.size.height/2, menuArrows.size.width, menuArrows.size.height/2)];
	[menuArrows setName:@"dark_menu_popover_arrow_selected" forSubimageInRect:NSMakeRect(0, 0, menuArrows.size.width, menuArrows.size.height/2)];
}

- (id)init{
    self = [super initWithContentRect:NSZeroRect styleMask:NSBorderlessWindowMask backing:NSBackingStoreBuffered defer:NO];
    if (self) {
		self.maxSize = NSMakeSize(192, 500);
		self.minSize = NSMakeSize(82, 19*2);
		
		OEMenuView* view = [[OEMenuView alloc] initWithFrame:NSZeroRect];
		[view setAutoresizingMask:NSViewWidthSizable|NSViewHeightSizable];
		[self setContentView:view];
		[self setHasShadow:NO];
		[view release];
		
		[self setLevel:NSTornOffMenuWindowLevel];
		[self setBackgroundColor:[NSColor clearColor]];	
	}
    return self;
}

- (void)dealloc{
	self.btn = nil;
	self.menu = nil;
	self.highlightedItem = nil;
	
    [super dealloc];
}
#pragma mark -
#pragma mark NSWindow overrides
- (BOOL)isOpaque{
	return NO;
}

#pragma mark -
- (void)openAtPoint:(NSPoint)p ofWindow:(NSWindow*)win{
	visible = YES;
	
	NSAssert(_localMonitor == nil, @"_localMonitor still exists somehow");
    _localMonitor = [NSEvent addLocalMonitorForEventsMatchingMask:NSLeftMouseDownMask | NSRightMouseDownMask | NSOtherMouseDownMask handler:^(NSEvent *incomingEvent) {
		
		if([[incomingEvent window] isKindOfClass:[self class]]){ // mouse down in window, will be handle by content view
			return incomingEvent;
		} else {
			// event is outside of window, close menu without changes and remove event
			[self closeMenuWithoutChanges];
		}		
        return (NSEvent *)nil;
    }];
	[_localMonitor retain];
	
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(closeMenuWithoutChanges) name:NSApplicationWillResignActiveNotification object:NSApp];
	
	[self setFrameOrigin:p];
	[win addChildWindow:self ordered:NSWindowAbove];
	
	[(OEMenuView*)[self contentView] updateView];
	
	if(self.delegate && [self.delegate respondsToSelector:@selector(menuDidShow:)]) [self.delegate performSelector:@selector(menuDidShow:) withObject:self];
}

- (void)closeMenuWithoutChanges{
	if(self.submenu) [self.submenu closeMenuWithoutChanges];
	[self _performcloseMenu];
	
	if(self.delegate && [self.delegate respondsToSelector:@selector(menuDidCancel:)]) [self.delegate performSelector:@selector(menuDidCancel:) withObject:self];
}

- (void)closeMenu{
	OEMenu* superMen = self;
	while(superMen.supermenu){
		superMen = superMen.supermenu;
	}

	if(superMen != self){
		[superMen closeMenu];
		return;
	}
	
	OEMenu* subMen = self;
	while(subMen.submenu){
		subMen = subMen.submenu;
	}
	
	NSMenuItem* selectedItem = subMen.highlightedItem;
	
	if(selectedItem && [selectedItem isEnabled] && [selectedItem target] && [selectedItem action]!=NULL && [[selectedItem target] respondsToSelector:[selectedItem action]]){
		[[selectedItem target] performSelectorOnMainThread:[selectedItem action] withObject:self waitUntilDone:NO];
	}

	if([selectedItem isEnabled] && self.btn){
		[self.btn selectItem:selectedItem];
	}
	
	if(self.submenu) [self.submenu closeMenuWithoutChanges];
	[self _performcloseMenu];

	if(self.delegate && [self.delegate respondsToSelector:@selector(menuDidSelect:)]) [self.delegate performSelector:@selector(menuDidSelect:) withObject:self];
}


- (void)_performcloseMenu{
	visible = NO;
	self.highlightedItem = nil;
	
	[[NSNotificationCenter defaultCenter] removeObserver:self];
	
	if(_localMonitor!=nil){
		[NSEvent removeMonitor:_localMonitor];
		[_localMonitor release];
		_localMonitor = nil;
	}
	
	[[self parentWindow] removeChildWindow:self];
	[self orderOut:self];
	
	if(self.delegate && [self.delegate respondsToSelector:@selector(menuDidHide:)]) [self.delegate performSelector:@selector(menuDidHide:) withObject:self];
}

- (void)setMenu:(NSMenu *)nmenu{
	[nmenu retain];
	[menu release];
	
	menu = nmenu;
	
	[(OEMenuView*)[self contentView] updateView];
}
#pragma mark -
#pragma mark Setter / getter
- (void)setHighlightedItem:(NSMenuItem *)_highlightedItem{
	[_highlightedItem retain];
	[highlightedItem release];
	highlightedItem = _highlightedItem;
	
	self.submenu = [[highlightedItem submenu] convertToOEMenu];
}
- (NSMenuItem*)highlightedItem{
	return highlightedItem;
}

- (void)setSubmenu:(OEMenu *)_submenu{
	if(submenu){
		[submenu closeMenuWithoutChanges];
	}
	
	if(_submenu){
		[[_submenu contentView] updateView];
		
		NSRect selectedItemRect = [(OEMenuView*)[self contentView] rectOfItem:self.highlightedItem];
		NSPoint submenuSpawnPoint = self.frame.origin;
				
		submenuSpawnPoint.x += self.frame.size.width;
		submenuSpawnPoint.x -= 9;
		
		
		submenuSpawnPoint.y = 8 - selectedItemRect.origin.y + self.frame.origin.y -_submenu.frame.size.height + self.frame.size.height;

		_submenu.btn = self.btn;
		_submenu.supermenu = self;
		[_submenu openAtPoint:submenuSpawnPoint ofWindow:self];		
	}
	
	[_submenu retain];
	[submenu release];
	submenu = _submenu;
}

- (OEMenu*)submenu{
	return submenu;
}

#pragma mark -
#pragma mark NSMenu wrapping
- (NSArray *)itemArray{
	return [self.menu itemArray];
}
@end

@implementation NSMenu (OEAdditions)
- (OEMenu*)convertToOEMenu{
	OEMenu* menu = [[OEMenu alloc] init];
	
	menu.menu = self;
	
	return [menu autorelease];
}
@end