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
@class OEMenuView;
@interface OEMenu : NSWindow 
{
@private
    NSMenu* menu;
	NSMenuItem* highlightedItem;
    
    OEMenu* submenu;
	OEMenu* supermenu;
	
	OEControlsPopupButton* popupButton;
	
	NSSize minSize, maxSize;
    int itemsAboveScroller, itemsBelowScroller;
    
	id _localMonitor;
	BOOL visible;	
    BOOL closing;
    
    id <OEMenuDelegate> delegate;
}
#pragma mark -
- (void)openAtPoint:(NSPoint)p ofWindow:(NSWindow*)win;
- (void)closeMenuWithoutChanges:(id)sender;
- (void)closeMenu;
#pragma mark -
#pragma mark NSMenu wrapping
- (NSArray *)itemArray;

@property (readwrite) NSSize minSize, maxSize;
@property (retain) OEControlsPopupButton* popupButton;
@property (nonatomic, retain) OEMenu* submenu;
@property (nonatomic, retain) OEMenu* supermenu;

@property (nonatomic, retain) NSMenu* menu;
@property (retain) NSMenuItem* highlightedItem;
@property (readonly, getter = isVisible) BOOL visible;

@property int itemsAboveScroller, itemsBelowScroller;
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

@interface OEMenuView : NSView 
{
@private
    BOOL imageIncluded;
}
- (void)updateView; // TODO: rename method
#pragma mark -
- (NSMenuItem*)itemAtPoint:(NSPoint)p;
- (NSRect)rectOfItem:(NSMenuItem*)m;
#pragma mark -
#pragma mark TextAttributes
- (NSDictionary*)itemTextAttributes;
- (NSDictionary*)selectedItemTextAttributes;
- (NSDictionary*)disabledItemTextAttributes;
@property (nonatomic, readonly) OEMenu* menu;
@end