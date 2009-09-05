//
//  OESaveState.h
//  OpenEmu
//
//  Created by Steve Streza on 9/3/09.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <Quartz/Quartz.h>

@class OEROMFile;

@interface OESaveState : NSManagedObject {
	NSString *emulatorID;
	NSString *userDescription;
}

-(id)initInsertedIntoManagedObjectContext:(NSManagedObjectContext *)context;

@property (nonatomic, retain) OEROMFile *romFile;
@property (readonly) NSBundle *bundle;

@property (nonatomic, retain) NSData *pathAlias;
@property (nonatomic, retain) NSString *bundlePath;
@property (readonly) NSString *screenshotPath;
@property (readonly) NSString *saveDataPath;

@property (nonatomic, retain) NSImage *screenshot;
@property (nonatomic, retain) NSData  *saveData;
@property (nonatomic, retain) NSString *emulatorID;

@property (nonatomic, retain) NSDate *timeStamp;

@property (readonly) NSString *resourcePath;
@property (readonly) NSString *infoPlistPath;

@end
