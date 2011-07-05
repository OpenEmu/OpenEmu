//
//  CollectionViewItemProtocol.h
//  OpenEmu
//
//  Created by Christoph Leimbrock on 24.06.11.
//  Copyright 2011 none. All rights reserved.
//

#import <Foundation/Foundation.h>


@protocol CollectionViewItemProtocol <NSObject>
- (NSString*)collectionViewName;
- (BOOL)isCollectionEditable;
- (BOOL)removingGamesDeletesThem;

- (NSPredicate*)predicate;
@end
