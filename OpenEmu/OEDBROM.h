//
//  OEDBRom.h
//  OpenEmu
//
//  Created by Carl Leimbrock on 10.10.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <CoreData/CoreData.h>

@class OELibraryDatabase;
@interface OEDBRom : NSManagedObject
- (NSArray*)saveStatesByTimestampAscending:(BOOL)ascFlag;
#pragma mark -
- (void)doInitialSetupWithDatabase:(OELibraryDatabase*)db;
#pragma mark -
#pragma mark Core Data utilities
+ (NSString *)entityName;
+ (NSEntityDescription *)entityDescriptionInContext:(NSManagedObjectContext *)context;
@end
