//
//  OEROMFile.h
//  OpenEmu
//
//  Created by Steve Streza on 8/15/09.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <CoreData/CoreData.h>

typedef enum {
    OEROMFileSystemTypeNES,
    OEROMFileSystemTypeSNES,
    OEROMFileSystemTypeGenesis,
    OEROMFileSystemTypeGameBoy,
    OEROMFileSystemTypeGameBoyAdvance,
    OEROMFileSystemTypeTurboGrafx16,
    OEROMFileSystemTypeAtariLynx,
    OEROMFileSystemTypeNeoGeo,
    OEROMFileSystemTypeUnknown
} OEROMFileSystemType;

@interface OEROMFile : NSManagedObject {
    //STUB add your instance variables here
}

//Core Data utilities
+(NSString *)entityName;
+(NSEntityDescription *)entityDescriptionInContext:(NSManagedObjectContext *)context;

+(OEROMFile *)fileWithPath:(NSString *)path
         createIfNecessary:(BOOL)create
    inManagedObjectContext:(NSManagedObjectContext *)context;
+(OEROMFile *)createFileWithPath:(NSString *)path 
insertedIntoManagedObjectContext:(NSManagedObjectContext *)context;

//Loading a ROM from a file
+(NSString *)nameForPath:(NSString *)path;
+(OEROMFileSystemType)systemTypeForFileAtPath:(NSString *)path;

@property (nonatomic, copy) NSString *path;
@property (nonatomic, copy) NSDate *lastPlayedDate;
-(NSURL *)pathURL;
@end
