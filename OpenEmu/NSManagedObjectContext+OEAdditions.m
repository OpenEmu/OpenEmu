//
//  NSManagedObjectContext+OEAdditions.m
//  OpenEmu
//
//  Created by Christoph Leimbrock on 01/07/14.
//
//

#import "NSManagedObjectContext+OEAdditions.h"
@implementation NSManagedObjectContext (OEAdditions)
- (OELibraryDatabase*)libraryDatabase
{
    return [[self userInfo] valueForKey:OELibraryDatabaseUserInfoKey];
}
@end
