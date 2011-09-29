//
//  OEDBRom  (DataSourceAdditions).h
//  OpenEmuMockup
//
//  Created by Christoph Leimbrock on 10.04.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>

#import "OEDBGame.h"
#import "OEDBSystem.h"
#import "OEDBCollection.h"
#import "OEDBSmartCollection.h"
#import "OEDBCollectionFolder.h"

#import <ImageKit/ImageKit.h>

#import "OECoverGridDataSourceItem.h"
#import "OECoverFlowDataSourceItem.h"
#import "OESideBarDataSourceItem.h"
#import "OEListViewDataSourceItem.h"
#import "OEDBAllGamesCollection.h"

#import "OECollectionViewItemProtocol.h"
@interface OEDBGame (DataSourceAdditions) <OECoverGridDataSourceItem, OECoverFlowDataSourceItem, OEListViewDataSourceItem>
@end

@interface OEDBSystem (DataSourceAdditions) <OESidebarDataSourceItem> 
@end


@interface OEDBCollection (DataSourceAdditions) <OESidebarDataSourceItem> 
@end

@interface OEDBSmartCollection (DataSourceAdditions) <OESidebarDataSourceItem> 
@end

@interface OEDBCollectionFolder (DataSourceAdditions) <OESidebarDataSourceItem> 
@end

#pragma mark -
#pragma mark Implementation of items that can be presented by CollectionView

@interface OEDBSystem (OECollectionViewItemAdditions) <OECollectionViewItemProtocol>
- (NSString*)collectionViewName;
- (BOOL)isCollectionEditable;
- (BOOL)removingGamesDeletesThem;
- (NSPredicate*)predicate;
@end

@interface OEDBCollection (OECollectionViewItemAdditions) <OECollectionViewItemProtocol>
- (NSString*)collectionViewName;
- (BOOL)isCollectionEditable;
- (BOOL)removingGamesDeletesThem;
- (NSPredicate*)predicate;
@end
@interface OEDBCollectionFolder (OECollectionViewItemAdditions) <OECollectionViewItemProtocol>
- (NSString*)collectionViewName;
- (BOOL)isCollectionEditable;
- (BOOL)removingGamesDeletesThem;
- (NSPredicate*)predicate;
@end
@interface OEDBSmartCollection (OECollectionViewItemAdditions) <OECollectionViewItemProtocol>
- (NSString*)collectionViewName;
- (BOOL)isCollectionEditable;
- (BOOL)removingGamesDeletesThem;
- (NSPredicate*)predicate;
@end
@interface OEDBAllGamesCollection (OECollectionViewItemAdditions) <OECollectionViewItemProtocol>
- (NSString*)collectionViewName;
- (BOOL)isCollectionEditable;
- (BOOL)removingGamesDeletesThem;
- (NSPredicate*)predicate;
@end