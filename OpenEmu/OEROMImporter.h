//
//  OEROMImporter.h
//  OpenEmu
//
//  Created by Carl Leimbrock on 19.09.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>

enum _OEImportErrorBehavior {
    OEImportErrorAskUser,
    OEImportErrorCancelKeepChanges,
    OEImportErrorCancelDeleteChanges,
    OEImportErrorIgnore
};
typedef enum _OEImportErrorBehavior OEImportErrorBehavior;
@class OELibraryDatabase;
@interface OEROMImporter : NSObject{
    BOOL canceld;
    
    OEImportErrorBehavior errorBehaviour;    
    OELibraryDatabase *database;
    
    dispatch_queue_t processingQueue;
    __block int queueCount;
    
    NSMutableArray *importedRoms;
}
- (id)initWithDatabase:(OELibraryDatabase*)_database;

- (BOOL)importROMsAtPath:(NSString*)path inBackground:(BOOL)bg error:(NSError**)outError;
- (BOOL)importROMsAtPaths:(NSArray*)pathArray inBackground:(BOOL)bg error:(NSError**)outError;

- (BOOL)importROMsAtURL:(NSURL*)url inBackground:(BOOL)bg error:(NSError**)outError;
- (BOOL)importROMsAtURLs:(NSArray*)urlArray inBackground:(BOOL)bg error:(NSError**)outError;
@property OEImportErrorBehavior errorBehaviour;
@property (assign) OELibraryDatabase *database;
@property __block int queueCount;

@property (readonly) NSArray *importedRoms;
@end
