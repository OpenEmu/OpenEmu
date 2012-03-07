/*
 Copyright (c) 2011, OpenEmu Team
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
     * Neither the name of the OpenEmu Team nor the
       names of its contributors may be used to endorse or promote products
       derived from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY OpenEmu Team ''AS IS'' AND ANY
 EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL OpenEmu Team BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

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
    
    importedRoms = nil;
    
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
        NSError __autoreleasing *error = nil;
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
         @autoreleasepool 
         {
             [normalizedPaths insertObject:[obj stringByExpandingTildeInPath] atIndex:idx];
         }
     }];
    
    DLog(@"normalizedPaths: %@", normalizedPaths);
    
    canceld = NO;
    return [self _performImportWithPaths:normalizedPaths error:outError];
}

- (BOOL)importROMsAtURL:(NSURL*)url inBackground:(BOOL)bg error:(NSError**)outError{
    return [self importROMsAtPath:[url path] inBackground:bg error:outError];
}
- (BOOL)importROMsAtURLs:(NSArray*)urlArray inBackground:(BOOL)bg error:(NSError**)outError{
    NSMutableArray *paths = [[NSMutableArray alloc] initWithCapacity:[urlArray count]];
    [urlArray enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
        [paths addObject:[obj path]];
    }];
    
    BOOL success = [self importROMsAtPaths:paths inBackground:bg error:outError];
    
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
    for (__strong NSString *aPath in paths) 
    {
        @autoreleasepool
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

                    __block NSUInteger result;
                    __block BOOL isSuppression = NO;
                    dispatch_sync(dispatch_get_main_queue(), ^{
                        NSAlert *alert = [NSAlert alertWithMessageText:@"An error occured" defaultButton:@"Continue" alternateButton:@"Stop" otherButton:@"Stop (keep changes)" informativeTextWithFormat:@"%@", [error localizedDescription]];
                        [alert setShowsSuppressionButton:YES];
                        
                        result = [alert runModal];
                        
                        isSuppression = ([[alert suppressionButton] state] == NSOnState);
                    });
                    
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
                    if(isSuppression)
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
    NSError *strongError;
    @try {
        @autoreleasepool
        {
            // TODO: check if path has readable suffix
            BOOL hasReadableSuffix = YES;
            if(!hasReadableSuffix) return YES;
            
            OEDBGame *game = [OEDBGame gameWithFilePath:filePath createIfNecessary:YES inDatabase:self.database error:&strongError];
            if(game)
            {
                BOOL lookupGameInfo = [[NSUserDefaults standardUserDefaults] boolForKey:UDAutmaticallyGetInfoKey];
                if(lookupGameInfo)
                {
                    [game performFullSyncWithArchiveVG:&strongError];
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
    }
    @finally {
        if (outError) *outError = strongError;
    }
}

- (NSArray*)importedRoms
{
    return importedRoms;
}

@end
