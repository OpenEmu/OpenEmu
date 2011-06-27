//
//  OEDBCollectionFolder.h
//  OpenEmu
//
//  Created by Christoph Leimbrock on 24.06.11.
//  Copyright 2011 none. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "OEDBCollection.h"

@interface OEDBCollectionFolder : OEDBCollection {
@private
    
}

// Core Data utilities
+ (NSString *)entityName;
+ (NSEntityDescription *)entityDescriptionInContext:(NSManagedObjectContext *)context;
@end
