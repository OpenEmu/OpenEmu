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
@class LibraryDatabase;
@interface OEROMImporter : NSObject{
	BOOL canceld;
	
	OEImportErrorBehavior errorBehaviour;
	NSMutableArray* import;
	
	LibraryDatabase* database;
}
- (id)initWithDatabase:(LibraryDatabase*)_database;

- (BOOL)importROMsAtPath:(NSString*)path inBackground:(BOOL)bg error:(NSError**)outError;
- (BOOL)importROMsAtPaths:(NSArray*)pathArray inBackground:(BOOL)bg error:(NSError**)outError;

@property OEImportErrorBehavior errorBehaviour;
@property (assign) LibraryDatabase* database;
@end
