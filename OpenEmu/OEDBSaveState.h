//
//  OEDBSaveState.h
//  OpenEmu
//
//  Created by Carl Leimbrock on 06.10.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <CoreData/CoreData.h>

@interface OEDBSaveState : NSManagedObject{

}
+ (id)newSaveStateInContext:(NSManagedObjectContext*)context;
@end
