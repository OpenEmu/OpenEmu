//
//  OEDBItem.m
//  OpenEmu
//
//  Created by Carl Leimbrock on 04.05.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import "OEDBItem.h"

@implementation OEDBItem
- (OELibraryDatabase*)libraryDatabase
{
    return [[[self managedObjectContext] userInfo] valueForKey:LibraryDatabaseKey];
}
@end
