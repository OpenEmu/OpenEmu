//
//  OEDBSmarCollection.m
//  OpenEmuMockup
//
//  Created by Christoph Leimbrock on 10.04.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "OEDBSmartCollection.h"


@implementation OEDBSmartCollection

+ (NSString *)entityName{
    return @"SmartCollection";
}

+ (NSEntityDescription *)entityDescriptionInContext:(NSManagedObjectContext *)context{
    return [NSEntityDescription entityForName:[self entityName] inManagedObjectContext:context];
}


@end
