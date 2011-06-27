//
//  OEDBCollectionFolder.m
//  OpenEmu
//
//  Created by Christoph Leimbrock on 24.06.11.
//  Copyright 2011 none. All rights reserved.
//

#import "OEDBCollectionFolder.h"


@implementation OEDBCollectionFolder

+ (NSString *)entityName{
    return @"CollectionFolder";
}

+ (NSEntityDescription *)entityDescriptionInContext:(NSManagedObjectContext *)context{
    return [NSEntityDescription entityForName:[self entityName] inManagedObjectContext:context];
}

@end
