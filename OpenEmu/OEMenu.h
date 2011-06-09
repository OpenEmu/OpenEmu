//
//  OENSMenu.h
//  OEPreferencesMockup
//
//  Created by Christoph Leimbrock on 04.06.11.
//  Copyright 2011 none. All rights reserved.
//

#import <Foundation/Foundation.h>

@class OEControlsPopupButton;
@protocol OEMenuDelegate;
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
	
	id <OEMenuDelegate> delegate;
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

@property (nonatomic, retain) NSMenu* menu;
@property (retain) NSMenuItem* highlightedItem;
@property (readonly, getter = isVisible) BOOL visible;

@property (nonatomic, retain) id <OEMenuDelegate> delegate;
@end

@interface NSMenu (OEAdditions)
- (OEMenu*)convertToOEMenu;
@end

@protocol OEMenuDelegate <NSObject>
@optional
- (void)menuDidShow:(OEMenu*)men;
- (void)menuDidHide:(OEMenu*)men;
- (void)menuDidSelect:(OEMenu*)men;
- (void)menuDidCancel:(OEMenu*)men;

@end