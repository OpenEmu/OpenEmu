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

@import Cocoa;
#import "OESidebarOutlineView.h"

NS_ASSUME_NONNULL_BEGIN

extern NSNotificationName const OESidebarSelectionDidChangeNotification;

extern NSString *const OESidebarMinWidth;
extern NSString *const OESidebarMaxWidth;
extern NSString *const OEMainViewMinWidth;

@class OELibraryDatabase, OEDBCollection;
@protocol OEGameCollectionViewItemProtocol;
@protocol OESidebarItem;

@interface OESidebarController : NSViewController <NSOutlineViewDelegate, NSOutlineViewDataSource>

- (IBAction)addCollectionAction:(nullable id)sender;

- (void)setEnabled:(BOOL)enabled;

- (void)reloadData;
- (id)addCollection:(BOOL)isSmart;
- (OEDBCollection *)addCollection;
- (id)duplicateCollection:(id)originalCollection;

- (void)selectItem:(id)item;
- (void)startEditingItem:(id)item;
- (void)expandCollections:(id)sender;
- (void)removeItemAtIndex:(NSUInteger)index;
- (void)renameItemAtIndex:(NSUInteger)index;
- (void)removeSelectedItemsOfOutlineView:(NSOutlineView *)outlineView;
- (void)removeItemForMenuItem:(NSMenuItem *)menuItem;
- (void)renameItemForMenuItem:(NSMenuItem *)menuItem;

- (void)changeDefaultCore:(id)sender;

@property (nonatomic, readonly, nullable) id<OESidebarItem> selectedSidebarItem;

@property (strong) OESidebarOutlineView *view;
@property (strong, nonatomic, nullable) OELibraryDatabase *database;
@property (strong, readonly) NSArray *groups;
@property (strong, readonly) NSArray *systems, *collections;
@property (nonatomic, strong, readwrite, nullable) id editingItem;
@end

NS_ASSUME_NONNULL_END
