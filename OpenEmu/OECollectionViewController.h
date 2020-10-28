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

#import "OEGridView.h"
#import "OEBlankSlateView.h"

#import "OECollectionViewItemProtocol.h"

@class OEDBGame;
@class OEGameTableView;
@class OELibraryToolbar;
@class OELibraryDatabase;
@class OEArrayController;

typedef NS_ENUM(NSInteger, OECollectionViewControllerViewTag) {
    OEBlankSlateTag = -1,
    OEGridViewTag   = 0,
    OEListViewTag   = 2
};

NS_ASSUME_NONNULL_BEGIN;

extern NSString * const OELastGridSizeKey;
extern NSString * const OELastCollectionViewKey;

@interface OECollectionViewController : NSViewController <OEBlankSlateViewDelegate, NSTableViewDelegate, NSTableViewDataSource, OEGridViewDelegate, OEGridViewMenuSource, QLPreviewPanelDelegate, QLPreviewPanelDataSource>

- (void)reloadData;
- (void)setNeedsReload;
- (void)setNeedsReloadVisible;
- (void)reloadDataIndexes:(NSIndexSet *)indexSet;
- (void)fetchItems;
- (NSArray *)defaultSortDescriptors;
- (void)setSortDescriptors:(NSArray*)descriptors;

- (void)updateBlankSlate;
- (BOOL)shouldShowBlankSlate;

- (void)showGridView;
- (void)showListView;
- (void)zoomGridViewWithValue:(CGFloat)zoomValue;

#pragma mark -
@property (readonly) NSArray<OEDBGame *> *selectedGames;
@property (nonatomic) NSIndexSet *selectionIndexes;

#pragma mark - Context Menu
- (NSMenu * _Nullable)menuForItemsAtIndexes:(NSIndexSet *)indexes;

#pragma mark - Quick Look
- (void)refreshPreviewPanelIfNeeded;
/* subclass these to implement quicklook for a specific collection */
- (BOOL)acceptsPreviewPanelControl:(QLPreviewPanel * _Nullable)panel;
- (NSInteger)numberOfPreviewItemsInPreviewPanel:(QLPreviewPanel * _Nullable)panel;
- (id <QLPreviewItem>)previewPanel:(QLPreviewPanel * _Nullable)panel previewItemAtIndex:(NSInteger)index;
- (NSInteger)imageBrowserViewIndexForPreviewItem:(id <QLPreviewItem> _Nullable)item;

#pragma mark -
@property(strong, nullable) id<OECollectionViewItemProtocol> representedObject;

@property(nonatomic, readonly) OECollectionViewControllerViewTag selectedViewTag;
@property(nonatomic, nullable) OELibraryDatabase *database;
@property(nonatomic, readonly, nullable) OELibraryToolbar *toolbar;

@end

@interface OECollectionViewController ()
@property (assign, nullable) IBOutlet OEGameTableView *listView;
@property (assign, nullable) IBOutlet OEGridView      *gridView;
- (void)OE_switchToView:(OECollectionViewControllerViewTag)tag;
- (void)OE_showView:(OECollectionViewControllerViewTag)tag;

@end

NS_ASSUME_NONNULL_END;
