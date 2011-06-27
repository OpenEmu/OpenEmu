//
//  OEDBCollection.m
//  OpenEmuMockup
//
//  Created by Christoph Leimbrock on 10.04.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "OEDBCollection.h"


@implementation OEDBCollection

+ (NSString *)entityName{
    return @"Collection";
}

+ (NSEntityDescription *)entityDescriptionInContext:(NSManagedObjectContext *)context{
    return [NSEntityDescription entityForName:[self entityName] inManagedObjectContext:context];
}


- (NSMutableArray*)items{
    return [NSMutableArray array];
}

@end
