//
//  NSManagedObjectContext+OEAdditions.h
//  OpenEmu
//
//  Created by Christoph Leimbrock on 01/07/14.
//
//

#import <CoreData/CoreData.h>
#import "OELibraryDatabase.h"

@interface NSManagedObjectContext (OEAdditions)
- (OELibraryDatabase*)libraryDatabase;
@end
