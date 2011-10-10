//
//  OEDBRom.h
//  OpenEmu
//
//  Created by Carl Leimbrock on 10.10.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <CoreData/CoreData.h>

@interface OEDBRom : NSManagedObject
- (NSArray*)saveStatesByTimestampAscending:(BOOL)ascFlag;
@end
