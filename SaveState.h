//
//  SaveState.h
//  OpenEmu
//
//  Created by Joshua Weinberg on 7/30/09.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import <CoreData/CoreData.h>
#import <Quartz/Quartz.h>

@interface SaveState :  NSManagedObject  
{
}

@property (nonatomic, retain) NSDate * timeStamp;
@property (nonatomic, retain) NSString * emulatorID;
@property (nonatomic, retain) NSString * rompath;
@property (nonatomic, retain) NSManagedObject * screenShot;
@property (nonatomic, retain) NSManagedObject * saveData;

@end



