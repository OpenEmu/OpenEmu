//
//  OEROMImporter.m
//  OpenEmu
//
//  Created by Carl Leimbrock on 19.09.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "OEROMImporter.h"
#import "OELibraryDatabase.h"
#import "OEDBRom.h"
#import "OEDBGame.h"
@interface OEROMImporter (Private)
- (BOOL)_performImportWithPath:(NSString*)path error:(NSError**)outError;// paths must not contain tilde, path must be absolute
- (BOOL)_performImportWithPaths:(NSArray*)paths error:(NSError**)outError;// paths must be absolute
- (BOOL)_performImportWithPaths:(NSArray*)paths relativeTo:(NSString*)path error:(NSError**)outError;
- (void)_performCancel:(BOOL)deleteChanges;

- (BOOL)_performImportWithFile:(NSString*)filePath error:(NSError**)outError; // filePath must not point to directory

- (void)_processImportQueue;
@end
@implementation OEROMImporter
@synthesize errorBehaviour, database, queueCount;

- (id)initWithDatabase:(OELibraryDatabase*)_database
{
    self = [super init];
    if (self) {
        self.database = _database;
        self.queueCount = 0;
        
        processingQueue = dispatch_queue_create("org.openemu.processROMs", NULL);
        dispatch_queue_t priority = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
        dispatch_set_target_queue(processingQueue, priority);
        
        importedRoms = [[NSMutableArray alloc] init];
        canceld = NO;
    }
    return self;
}

- (void)dealloc {
    dispatch_release(processingQueue);
    
    [importedRoms release];
    importedRoms = nil;
    
    [super dealloc];
}

- (BOOL)importROMsAtPath:(NSString*)path inBackground:(BOOL)bg error:(NSError**)outError
{
    NSArray *pathArray = [NSArray arrayWithObject:path];
    return [self importROMsAtPaths:pathArray inBackground:bg error:outError];
}

- (BOOL)importROMsAtPaths:(NSArray*)pathArray inBackground:(BOOL)bg error:(NSError**)outError
{
    DLog(@"inPaths: %@", pathArray);
    if(!self.database)
    {
        // TODO: Create proper error
        NSLog(@"Import without Database!");
        if(outError!=NULL) *outError = [NSError errorWithDomain:@"IMPORT WITHOUT DATABASE" code:0 userInfo:nil];
        return NO;
    }
    
    if(![NSThread isMainThread])
    {
        DLog(@"Not on main thread - trashin some values");
        // if we do not run on main thread it is very possible that bg and outError hold garbage!
        NSError *error = nil;
        outError = &error;
    } 
    else if(bg)
    {
        if(outError!=NULL) *outError = nil;
        
        DLog(@"WILL RUN IN BACKGROUND");
        // this will pass random values as bg and outError
        [self performSelectorInBackground:@selector(importROMsAtPaths:inBackground:error:) withObject:pathArray];
        
        return YES;
    }
    
    // remove tildes
    NSMutableArray *normalizedPaths = [NSMutableArray arrayWithCapacity:[pathArray count]];
    [pathArray enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) 
     {
         [normalizedPaths insertObject:[obj stringByExpandingTildeInPath] atIndex:idx];
     }];
    
    DLog(@"normalizedPaths: %@", normalizedPaths);
    
    canceld = NO;
    return [self _performImportWithPaths:normalizedPaths error:outError];
}

- (BOOL)importROMsAtURL:(NSURL*)url inBackground:(BOOL)bg error:(NSError**)outError{
    return [self importROMsAtPath:[url path] inBackground:bg error:outError];
}
- (BOOL)importROMsAtURLs:(NSArray*)urlArray inBackground:(BOOL)bg error:(NSError**)outError{
    __block NSMutableArray *paths = [[NSMutableArray alloc] initWithCapacity:[urlArray count]];
    [urlArray enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
        [paths addObject:[obj path]];
    }];
    
    BOOL success = [self importROMsAtPaths:paths inBackground:bg error:outError];
    [paths release];
    
    return success;
}

- (BOOL)_performImportWithPaths:(NSArray*)paths error:(NSError**)outError
{
    return [self _performImportWithPaths:paths relativeTo:nil error:outError];
}

- (BOOL)_performImportWithPaths:(NSArray*)paths relativeTo:(NSString *)basePath error:(NSError **)outError
{
    DLog(@"canceld: %d", canceld);
    if (canceld)
        return YES;
    
    NSError *error = nil;
    
    BOOL success = YES;
    for (NSString *aPath in paths) 
    {
        if(canceld) return YES;
        
        if(basePath)
        {
            aPath = [basePath stringByAppendingPathComponent:aPath];
        }
        success = [self _performImportWithPath:aPath error:&error];
        if(!success)
        {
            OEImportErrorBehavior behavior = errorBehaviour;
            if(errorBehaviour==OEImportErrorAskUser)
            {
                DLog(@"ERROR");  

                NSAlert *alert = [NSAlert alertWithMessageText:@"An error occured" defaultButton:@"Continue" alternateButton:@"Stop" otherButton:@"Stop (keep changes)" informativeTextWithFormat:@"%@", [error localizedDescription]];
                [alert setShowsSuppressionButton:YES];
                
                // TODO: run alert on main thread
                NSUInteger result = [alert runModal];
                
                switch (result) {
                    case NSAlertDefaultReturn:
                        behavior = OEImportErrorIgnore;
                        break;
                    case NSAlertAlternateReturn:
                        behavior = OEImportErrorCancelDeleteChanges;
                        break;
                    case NSAlertOtherReturn:
                        behavior = OEImportErrorCancelKeepChanges;
                        break;
                    default:
                        break;
                }
                
                // TODO: decide if suppression is forever
                if([[alert suppressionButton] state] == NSOnState)
                    errorBehaviour = behavior;
            }
            
            if(outError!=NULL)
            {
                *outError = error;
            }
            
            if(behavior != OEImportErrorIgnore)
            {
                [self _performCancel:behavior==OEImportErrorCancelDeleteChanges];
                // returning YES because error was handled
                return YES;
            }
        }
        error = nil;
    }
    return success;
}



- (BOOL)_performImportWithPath:(NSString*)path error:(NSError**)outError
{
    DLog(@"%d", canceld);
    if (canceld)
        return YES;
    
    // skip invisible files
    // TODO: implement proper check (hidden files without .)
    // TODO: what if we want to import those as well? add option
    if([[path lastPathComponent] characterAtIndex:0] == '.')
    {
        return YES;
    }
    
    NSFileManager *defaultManager = [NSFileManager defaultManager];
    
    BOOL isDir = NO;
    
    if(![defaultManager fileExistsAtPath:path])
    {
        NSLog(@"file does not exist at path: %@", path);
        // TODO: add proper error!
        if(outError!=NULL) *outError = [NSError errorWithDomain:@"ERRORDOMAIN" code:0 userInfo:nil];
        return NO;
    }
    
    if([defaultManager fileExistsAtPath:path isDirectory:&isDir] && !isDir)
    {
        return [self _performImportWithFile:path error:outError];
    }
    
    NSArray *paths = [defaultManager contentsOfDirectoryAtPath:path error:outError];
    if(!paths)
    {
        NSLog(@"no paths – contentsOfDirectoryAtPath:error");
        return NO;
    }
    
    BOOL success = [self _performImportWithPaths:paths relativeTo:path error:outError];

    
    return success;
}

- (void)_performCancel:(BOOL)deleteChanges
{
    // TODO: IMPLEMENT!!!!!
    canceld = YES;
}

- (BOOL)_performImportWithFile:(NSString*)filePath error:(NSError**)outError
{    
    // TODO: check if path has readable suffix
    BOOL hasReadableSuffix = YES;
    if(!hasReadableSuffix) return YES;
    
    OEDBGame *game = [OEDBGame gameWithFilePath:filePath createIfNecessary:YES inDatabase:self.database error:outError];
    if(game)
    {
        BOOL lookupGameInfo = [[NSUserDefaults standardUserDefaults] boolForKey:UDAutmaticallyGetInfoKey];
        if(lookupGameInfo)
        {
            [game performSyncWithArchiveVG:outError];
             // TODO: decide if we are interesed in success of sync operation
        }
        
        BOOL organizeLibrary = [[NSUserDefaults standardUserDefaults] boolForKey:UDOrganizeLibraryKey];
        if(organizeLibrary)
        {
            NSLog(@"organize library");
            // TODO: initiate lib organization if requested
        }
    }
    return game!=nil;
}

- (NSArray*)importedRoms
{
    return importedRoms;
}

@end
