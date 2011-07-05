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

#import "CoverGridDataSourceItem.h"
#import "CoverFlowDataSourceItem.h"
#import "SideBarDataSourceItem.h"
#import "ListViewDataSourceItem.h"
#import "OEDBAllGamesCollection.h"

#import "CollectionViewItemProtocol.h"
@interface OEDBGame (DataSourceAdditions) <CoverGridDataSourceItem, CoverFlowDataSourceItem, ListViewDataSourceItem>
@end

@interface OEDBSystem (DataSourceAdditions) <SidebarDataSourceItem> 
@end


@interface OEDBCollection (DataSourceAdditions) <SidebarDataSourceItem> 
@end

@interface OEDBSmartCollection (DataSourceAdditions) <SidebarDataSourceItem> 
@end

@interface OEDBCollectionFolder (DataSourceAdditions) <SidebarDataSourceItem> 
@end

#pragma mark -
#pragma mark Implementation of items that can be presented by CollectionView

@interface OEDBSystem (OECollectionViewItemAdditions) <CollectionViewItemProtocol>
- (NSString*)collectionViewName;
- (BOOL)isCollectionEditable;
- (BOOL)removingGamesDeletesThem;
- (NSPredicate*)predicate;
@end

@interface OEDBCollection (OECollectionViewItemAdditions) <CollectionViewItemProtocol>
- (NSString*)collectionViewName;
- (BOOL)isCollectionEditable;
- (BOOL)removingGamesDeletesThem;
- (NSPredicate*)predicate;
@end
@interface OEDBCollectionFolder (OECollectionViewItemAdditions) <CollectionViewItemProtocol>
- (NSString*)collectionViewName;
- (BOOL)isCollectionEditable;
- (BOOL)removingGamesDeletesThem;
- (NSPredicate*)predicate;
@end
@interface OEDBSmartCollection (OECollectionViewItemAdditions) <CollectionViewItemProtocol>
- (NSString*)collectionViewName;
- (BOOL)isCollectionEditable;
- (BOOL)removingGamesDeletesThem;
- (NSPredicate*)predicate;
@end
@interface OEDBAllGamesCollection (OECollectionViewItemAdditions) <CollectionViewItemProtocol>
- (NSString*)collectionViewName;
- (BOOL)isCollectionEditable;
- (BOOL)removingGamesDeletesThem;
- (NSPredicate*)predicate;
@end