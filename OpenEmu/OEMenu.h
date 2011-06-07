//
//  OENSMenu.h
//  OEPreferencesMockup
//
//  Created by Christoph Leimbrock on 04.06.11.
//  Copyright 2011 none. All rights reserved.
//

#import <Foundation/Foundation.h>

@class OEControlsPopupButton;
@interface OEMenu : NSWindow {
@private
	OEControlsPopupButton* btn;
	NSMenu* menu;
	NSMenuItem* highlightedItem;
	OEMenu* submenu;
	OEMenu* supermenu;
	
	NSSize minSize, maxSize;
	
	id _localMonitor;
	
	BOOL visible;
}
#pragma mark -
- (void)openAtPoint:(NSPoint)p ofWindow:(NSWindow*)win;

- (void)closeMenuWithoutChanges;
- (void)closeMenu;
#pragma mark -
#pragma mark NSMenu wrapping
- (NSArray *)itemArray;

@property (readwrite) NSSize minSize, maxSize;
@property (retain) OEControlsPopupButton* btn;
@property (nonatomic, retain) OEMenu* submenu;
@property (nonatomic, retain) OEMenu* supermenu;

@property (retain) NSMenu* menu;
@property (retain) NSMenuItem* highlightedItem;
@property (readonly, getter = isVisible) BOOL visible;
@end

@interface NSMenu (OEAdditions)
- (OEMenu*)convertToOEMenu;
@end