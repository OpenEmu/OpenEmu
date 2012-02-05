/*
 Copyright (c) 2011, OpenEmu Team
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
     * Neither the name of the OpenEmu Team nor the
       names of its contributors may be used to endorse or promote products
       derived from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY OpenEmu Team ''AS IS'' AND ANY
 EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL OpenEmu Team BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#import <AppKit/AppKit.h>
#import "NSWindow+OECustomWindow.h"
@class OEPopupButton;
@class OENewMenu;
@protocol OENewMenuDelegate <NSObject>
@optional
- (void)menuDidShow:(OENewMenu*)men;
- (void)menuDidHide:(OENewMenu*)men;
- (void)menuDidSelect:(OENewMenu*)men;
- (void)menuDidCancel:(OENewMenu*)men;
@end
@interface OENewMenu : NSWindow <OECustomWindow, NSTableViewDelegate, NSTableViewDataSource>
{
    NSTableView *upperTableView;
    NSTableView *mainTableView;
    NSTableView *lowerTableView;
    
    id _localMonitor;
    BOOL visible;
    BOOL closing;
}

- (void)openAtPoint:(NSPoint)p ofWindow:(NSWindow*)win;
- (void)closeMenuWithoutChanges:(id)sender;
- (void)closeMenu;
#pragma mark -
- (NSArray *)itemArray;
#pragma mark -
- (void)highlightItemAtPoint:(NSPoint)p;
- (id)itemAtPoint:(NSPoint)p;
@property int itemsAboveScroller, itemsBelowScroller;
@property (getter=isAlernate) BOOL alernate;

@property (retain) NSMenu *menu;
@property (retain) OENewMenu *supermenu;
@property (retain) OENewMenu *activeSubmenu;
@property (retain) OEPopupButton *popupButton;
@property (retain) NSMenuItem *highlightedItem;

@property (assign) id <OENewMenuDelegate> delegate;
@end

@interface NSMenu (OEOENewMenuAdditions)
- (OENewMenu*)convertToOENewMenu;
@end

