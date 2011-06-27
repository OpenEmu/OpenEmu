//
//  OENSMenuView.h
//  OEPreferencesMockup
//
//  Created by Christoph Leimbrock on 04.06.11.
//  Copyright 2011 none. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class OEMenu;
@interface OEMenuView : NSView {
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
