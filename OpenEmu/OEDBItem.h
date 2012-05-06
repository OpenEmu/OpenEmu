//
//  OEDBItem.h
//  OpenEmu
//
//  Created by Carl Leimbrock on 04.05.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import <CoreData/CoreData.h>
@class OELibraryDatabase;
@interface OEDBItem : NSManagedObject
- (OELibraryDatabase*)libraryDatabase;
@end
