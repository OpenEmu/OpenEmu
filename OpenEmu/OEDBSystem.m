//
//  OEDBConsole.m
//  OpenEmuMockup
//
//  Created by Christoph Leimbrock on 10.04.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "OEDBSystem.h"


@implementation OEDBSystem

#pragma mark -
#pragma mark Core Data utilities
+ (NSString *)entityName{
    return @"System";
}

+ (NSEntityDescription *)entityDescriptionInContext:(NSManagedObjectContext *)context{
    return [NSEntityDescription entityForName:[self entityName] inManagedObjectContext:context];
}
#pragma mark -
- (NSImage*)icon{
    NSImage* image = [NSImage imageNamed:[self valueForKey:@"name"]];
        
    return image;
}

@end
