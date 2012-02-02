//
//  CollectionViewController.h
//  OpenEmu
//
//  Created by Christoph Leimbrock on 24.06.11.
//  Copyright 2011 none. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "IKSGridView.h"
#import "IKSGridViewProtocols.h"
#import "IKImageFlowView.h"

#import "OECollectionViewItemProtocol.h"

@class OELibraryController;
@class OEHorizontalSplitView;
@interface OECollectionViewController : NSViewController <IKSGridViewDelegate, IKSGridViewDataSource, NSTableViewDelegate, NSTableViewDataSource>
{
@private
    NSArrayController *gamesController;
    
    id <NSObject, OECollectionViewItemProtocol> collectionItem;
    
    IBOutlet NSView *gridViewContainer;// gridview
    IBOutlet IKSGridView *gridView;// scrollview for gridview
    
    IBOutlet OEHorizontalSplitView *flowlistViewContainer; // cover flow and simple list container
    IBOutlet IKImageFlowView *coverFlowView;
    IBOutlet NSTableView *listView;
    
    NSTimer *reloadTimer;
}

- (void)finishSetup;

#pragma mark -
- (NSArray*)selectedGames;
#pragma mark -
#pragma mark View Selection
- (IBAction)selectGridView:(id)sender;
- (IBAction)selectFlowView:(id)sender;
- (IBAction)selectListView:(id)sender;

#pragma mark -
#pragma mark Toolbar Actions
- (IBAction)search:(id)sender;
- (IBAction)changeGridSize:(id)sender;

#pragma mark -
- (void)willShow;

- (void)setNeedsReload;
#pragma mark -
@property (assign) OELibraryController *libraryController;
@property (nonatomic, retain) id <NSObject, OECollectionViewItemProtocol> collectionItem;

@property (nonatomic, retain) IBOutlet NSView *emptyCollectionView;
@property (nonatomic, retain) IBOutlet NSView *emptyConsoleView;
@end
