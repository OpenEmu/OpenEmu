//
//  OEROMImporter.m
//  OpenEmu
//
//  Created by Carl Leimbrock on 19.09.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "OEROMImporter.h"
#import "LibraryDatabase.h"
@interface OEROMImporter (Private)
- (BOOL)_performImportWithPath:(NSString*)path error:(NSError**)outError;	// paths must not contain tilde, path must be absolute
- (BOOL)_performImportWithPaths:(NSArray*)paths error:(NSError**)outError;	// paths must be absolute
- (BOOL)_performImportWithPaths:(NSArray*)paths relativeTo:(NSString*)path error:(NSError**)outError;
- (void)_performCancel:(BOOL)deleteChanges;

- (BOOL)_performImportWithFile:(NSString*)filePath error:(NSError**)outError; // filePath must not point to directory
@end
@implementation OEROMImporter
@synthesize errorBehaviour, database;

- (id)initWithDatabase:(LibraryDatabase*)_database{
    self = [super init];
    if (self) {
		self.database = _database;
    }
    return self;
}

- (BOOL)importROMsAtPath:(NSString*)path inBackground:(BOOL)bg error:(NSError**)outError{
	NSArray* pathArray = [NSArray arrayWithObject:path];
	return [self importROMsAtPaths:pathArray inBackground:bg error:outError];
}

- (BOOL)importROMsAtPaths:(NSArray*)pathArray inBackground:(BOOL)bg error:(NSError**)outError{
	if(!self.database){
		// TODO: Create proper error
		NSLog(@"Import without Database!");
		if(outError!=NULL) *outError = [NSError errorWithDomain:@"IMPORT WITHOUT DATABASE" code:0 userInfo:nil];
		return NO;
	}
	
	if(![NSThread isMainThread]){
		// if we do not run on main thread it is very possible that bg and outError hold garbage!
		NSError* error = nil;
		bg = NO;
		outError = &error;
	} else if(bg){
		if(outError!=NULL) *outError = nil;
		
		// this will pass random values as bg and outError
		[self performSelectorInBackground:@selector(importROMsAtPaths:inBackground:error:) withObject:pathArray];
		
		return YES;
	}
	
	// remove tildes
	NSMutableArray* normalizedPaths = [NSMutableArray arrayWithCapacity:[pathArray count]];
	[pathArray enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
		[normalizedPaths insertObject:[obj stringByExpandingTildeInPath] atIndex:idx];
	}];
		
	canceld = NO;
	return [self _performImportWithPaths:normalizedPaths error:outError];
}

- (BOOL)_performImportWithPaths:(NSArray*)paths error:(NSError**)outError{
	return [self _performImportWithPaths:paths relativeTo:nil error:outError];
}

- (BOOL)_performImportWithPaths:(NSArray*)paths relativeTo:(NSString *)basePath error:(NSError **)outError{	
	if (canceld)
		return YES;
	
	NSError* error = nil;
	
	BOOL success = YES;
	for (NSString* aPath in paths) {
		if(canceld) return YES;
		
		if(basePath){
			aPath = [basePath stringByAppendingPathComponent:aPath];
		}
		success = [self _performImportWithPath:aPath error:&error];
		if(!success){
			OEImportErrorBehavior beh = errorBehaviour;
			if(errorBehaviour==OEImportErrorAskUser){
				
				NSAlert* alert = [NSAlert alertWithMessageText:@"An error occured" defaultButton:@"Continue" alternateButton:@"Stop" otherButton:@"Stop (keep changes)" informativeTextWithFormat:@"%@", [error localizedDescription]];
				[alert setShowsSuppressionButton:YES];
	
				// TODO: run alert on main thread
				NSUInteger result = [alert runModal];
			
				switch (result) {
					case NSAlertDefaultReturn:
						beh = OEImportErrorIgnore;
						break;
					case NSAlertAlternateReturn:
						beh = OEImportErrorCancelDeleteChanges;
						break;
					case NSAlertOtherReturn:
						beh = OEImportErrorCancelKeepChanges;						
						break;				
					default:
						break;
				}
				
				// TODO: decide if suppression is forever
				if([[alert suppressionButton] state] == NSOnState)
					errorBehaviour = beh;
			}
			
			if(outError!=NULL){
				*outError = error;
			}
			
			if(beh != OEImportErrorIgnore){
				[self _performCancel:beh==OEImportErrorCancelDeleteChanges];
				// returning YES because error was handled
				return	YES;
			}
		}
		error = nil;
	}
	return success;
}



- (BOOL)_performImportWithPath:(NSString*)path error:(NSError**)outError{
	if (canceld)
		return YES;
	
	// skip invisible files
	// TODO: implement proper check (hidden files without .)
	// TODO: what if we want to import those as well? add option
	if([[path lastPathComponent] characterAtIndex:0] == '.'){
		return YES;
	}
	
	NSFileManager* defaultManager = [NSFileManager defaultManager];
	
	BOOL isDir = NO;
	
	if(![defaultManager fileExistsAtPath:path]){
		NSLog(@"file does not exist at path: %@", path);
		// TODO: add proper error!
		if(outError!=NULL) *outError = [NSError errorWithDomain:@"ERRORDOMAIN" code:0 userInfo:nil];
		return NO;
	}
	
	if([defaultManager fileExistsAtPath:path isDirectory:&isDir] && !isDir){
		return [self _performImportWithFile:path error:outError];
	}
	
	NSArray* paths = [defaultManager contentsOfDirectoryAtPath:path error:outError];
	if(!paths){
		return NO;
	}
	
	BOOL success = [self _performImportWithPaths:paths relativeTo:path error:outError];
	if(success){
		// TODO: initiate archive sync if required
		// TODO: initiate lib organization if requested
	}
	
	return success;
}

- (void)_performCancel:(BOOL)deleteChanges{
	//TODO: IMPLEMENT!!!!!
	canceld = YES;
}

- (BOOL)_performImportWithFile:(NSString*)filePath error:(NSError**)outError{
#warning finish method implementation
	// check if path has readable suffix
	BOOL hasReadableSuffix = YES;
	if(!hasReadableSuffix) return YES;
	
	NSError* error = nil;
	
	NSUserDefaults* standardDefaults = [NSUserDefaults standardUserDefaults];
/*
	BOOL automaticallyGetInfo = [standardDefaults boolForKey:UDAutmaticallyGetInfoKey];
	BOOL quickImport = [standardDefaults boolForKey:UDUseQuickImportKey] && !automaticallyGetInfo;
	BOOL organizeLibrary = [standardDefaults boolForKey:UDOrganizeLibraryKey];
	
	BOOL md5 = [standardDefaults boolForKey:UDUseMD5HashingKey];
 */
 
	// get managed object
	// check if path is in database

	BOOL isInDatabase = [self.database isFileInDatabaseWithPath:filePath error:&error];
	if(error!=nil){
		NSLog(@"Error checking database");
		if (outError!=NULL) *outError = error;
		return NO;
	}
	
	if(isInDatabase){
		if(outError!=NULL) *outError = nil;
		return YES;
	}
	
	BOOL copyToDatabase = [standardDefaults boolForKey:UDCopyToLibraryKey];
	if(copyToDatabase){
		NSString* newPath = @"";
		// determine new file path within databse folder
		// copy to db unsorted
		filePath = newPath;
	}

	// create rom
	// add rom id to imported fiels
	
	return YES;
}
@end
