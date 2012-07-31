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
#import "OESystemPlugin.h"
#import "NSURL+OELibraryAdditions.h"

@interface OEROMImporter ()
{
    dispatch_queue_t processingQueue;
    NSMutableArray *importedRoms;
    BOOL cancelled;
}

- (void)OE_performCancel:(BOOL)deleteChanges;
- (BOOL)OE_performImportWithURL:(NSURL *)url error:(NSError *__autoreleasing *)outError;
- (BOOL)OE_performImportWithURLs:(NSArray *)urls error:(NSError *__autoreleasing *)outError;
// url must not point to a directory
- (BOOL)OE_performImportWithFileURL:(NSURL *)url error:(NSError *__autoreleasing *)outError;

@property (weak) OELibraryDatabase *database;

// spotlight search results.
@property(readwrite, retain) NSMutableArray *searchResults;

@property BOOL isBusy;
@end

@implementation OEROMImporter
@synthesize errorBehaviour, database, isBusy, delegate;

- (id)initWithDatabase:(OELibraryDatabase *)aDatabase
{
    self = [super init];
    if (self) {
        [self setDatabase:aDatabase];
        
        processingQueue = dispatch_queue_create("org.openemu.processROMs", NULL);
        dispatch_queue_t priority = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
        dispatch_set_target_queue(processingQueue, priority);
        
        importedRoms = [[NSMutableArray alloc] init];
        self.searchResults = [[NSMutableArray alloc] initWithCapacity:1];
        
        self.isBusy = YES;
    }
    return self;
}

- (void)dealloc 
{
    dispatch_release(processingQueue);
    
    importedRoms = nil;
}

- (BOOL)importROMsAtPath:(NSString *)path inBackground:(BOOL)bg error:(NSError *__autoreleasing*)outError
{
    return [self importROMsAtURL:[NSURL fileURLWithPath:path] inBackground:bg error:outError];
}

- (BOOL)importROMsAtPaths:(NSArray *)pathArray inBackground:(BOOL)bg error:(NSError *__autoreleasing*)outError
{
    NSMutableArray *urlArray = [NSMutableArray arrayWithCapacity:[pathArray count]];
    [pathArray enumerateObjectsUsingBlock:
     ^(id obj, NSUInteger idx, BOOL *stop)
     {
         [urlArray addObject:[NSURL fileURLWithPath:obj]]; 
     }];
    return [self importROMsAtURLs:urlArray inBackground:bg error:outError];
}

- (BOOL)importROMsAtURL:(NSURL *)url inBackground:(BOOL)bg error:(NSError *__autoreleasing*)outError
{
    NSArray *urlArray = [NSArray arrayWithObject:url];
    return [self importROMsAtURLs:urlArray inBackground:bg error:outError];
}

- (BOOL)importROMsAtURLs:(NSArray *)urlArray inBackground:(BOOL)bg error:(NSError *__autoreleasing*)outError
{ 
    if([self database] == nil)
    {
        // TODO: Create proper error
        // DLog(@"Import without Database!");
        if(outError != NULL) *outError = [NSError errorWithDomain:@"IMPORT WITHOUT DATABASE" code:0 userInfo:nil];
        return NO;
    }
    
    if(![NSThread isMainThread])
    {
        // DLog(@"Not on main thread - trashin some values");
        // if we do not run on main thread it is very possible that bg and outError hold garbage!
        NSError __autoreleasing *error = nil;
        outError = &error;
    } 
    else if(bg)
    {
        if(outError != NULL) *outError = nil;
        
        // DLog(@"WILL RUN IN BACKGROUND");
        // this will pass random values as bg and outError
        [self performSelectorInBackground:@selector(importROMsAtURLs:inBackground:error:) withObject:urlArray];
        
        return YES;
    }

    // DLog(@"normalizedPaths: %@", normalizedPaths);
    
    cancelled = NO;
    return [self OE_performImportWithURLs:urlArray error:outError];
}

- (BOOL)OE_performImportWithURLs:(NSArray *)urls error:(NSError *__autoreleasing*)outError
{
    // DLog(@"canceld: %d", canceld);
    if(cancelled) return YES;
    
    NSError *error = nil;
    
    BOOL success = YES;
    for(__strong NSURL *aURL in urls) 
    {
        @autoreleasepool
        {
            if(cancelled) return YES;
        
            success = [self OE_performImportWithURL:aURL error:&error];
            if(!success)
            {
                OEImportErrorBehavior behavior = errorBehaviour;
                if(errorBehaviour==OEImportErrorAskUser)
                {
                    // DLog(@"ERROR");  

                    __block NSUInteger result;
                    __block BOOL isSuppression = NO;
                    dispatch_sync(dispatch_get_main_queue(), ^{
                        NSAlert *alert = [NSAlert alertWithMessageText:@"An error occured" defaultButton:@"Continue" alternateButton:@"Stop" otherButton:@"Stop (keep changes)" informativeTextWithFormat:@"%@", [error localizedDescription]];
                        [alert setShowsSuppressionButton:YES];
                        
                        result = [alert runModal];
                        
                        isSuppression = ([[alert suppressionButton] state] == NSOnState);
                    });
                    
                    switch(result)
                    {
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
                    if(isSuppression) errorBehaviour = behavior;
                }
                
                if(behavior != OEImportErrorIgnore)
                {
                    if(outError != NULL) *outError = error;
                    
                    [self OE_performCancel:behavior == OEImportErrorCancelDeleteChanges];
                    
                    // returning YES because error was handled
                    return YES;
                }
                
                error = nil;
            }
        }
    }
    
    return success;
}

- (BOOL)OE_performImportWithURL:(NSURL *)url error:(NSError *__autoreleasing*)outError
{
    // DLog(@"%d", canceld);
    if(cancelled) return YES;
    
    // skip invisible files
    // TODO: implement proper check (hidden files without .)
    // TODO: what if we want to import those as well? add option
    
    NSFileManager *defaultManager = [NSFileManager defaultManager];
    if(![url checkResourceIsReachableAndReturnError:nil])
    {
        // DLog(@"file does not exist at path: %@", path);
        // TODO: add proper error!
        if(outError!=NULL) *outError = [NSError errorWithDomain:@"ERRORDOMAIN" code:0 userInfo:nil];
        return NO;
    }
    
    if(![url isDirectory]) return [self OE_performImportWithFileURL:url error:outError];
    
    NSArray *urls = [defaultManager contentsOfDirectoryAtURL:url 
                                  includingPropertiesForKeys:[NSArray arrayWithObjects:NSURLNameKey, NSURLIsDirectoryKey, NSURLIsPackageKey, nil]
                                                     options:NSDirectoryEnumerationSkipsSubdirectoryDescendants | NSDirectoryEnumerationSkipsPackageDescendants | NSDirectoryEnumerationSkipsHiddenFiles 
                                                       error:outError];
    if(urls == nil)
    {
        // DLog(@"no paths – contentsOfDirectoryAtPath:error");
        return NO;
    }
    
    return [self OE_performImportWithURLs:urls error:outError];
}

- (void)OE_performCancel:(BOOL)deleteChanges
{
    // TODO: IMPLEMENT!!!!!
    cancelled = YES;
}

- (BOOL)OE_performImportWithFileURL:(NSURL *)url error:(NSError *__autoreleasing*)outError
{
    NSError *strongError;
    
    @try
    {
        @autoreleasepool
        {
            // TODO: check if path has readable suffix
            BOOL hasReadableSuffix = YES;
            if(!hasReadableSuffix) return YES;
            
            OEDBGame *game = [OEDBGame gameWithURL:url createIfNecessary:YES inDatabase:self.database error:&strongError];
            if(game != nil)
            {
                BOOL lookupGameInfo = [[NSUserDefaults standardUserDefaults] boolForKey:UDAutmaticallyGetInfoKey];
                if(lookupGameInfo)
                {
                    [game setNeedsFullSyncWithArchiveVG];
                    // TODO: decide if we are interesed in success of sync operation
                }
                
                BOOL organizeLibrary = [[NSUserDefaults standardUserDefaults] boolForKey:UDOrganizeLibraryKey];
                if(organizeLibrary)
                {
                    // TODO: initiate lib organization if requested
                    NSURL *romsFolderURL = [[self database] romsFolderURL];
                    NSURL *systemsRomFolderURL = [[self database] romsFolderURLForSystem:[game system]];
                    [[game roms] enumerateObjectsUsingBlock:^(OEDBRom *obj, BOOL *stop) {
                            if([[obj URL] isSubpathOfURL:romsFolderURL])
                            {
                                NSURL *targetURL = [systemsRomFolderURL URLByAppendingPathComponent:[[obj URL] lastPathComponent]];
                                if([[NSFileManager defaultManager] moveItemAtURL:[obj URL] toURL:targetURL error:nil])
                                    [obj setURL:targetURL];
                            }
                    }];
                }
            }
            
            return game != nil;
        }
    }
    @finally
    {
        if(outError != NULL) *outError = strongError;
    }
    
    strongError = nil;
}

- (NSArray *)importedRoms
{
    return importedRoms;
}

#pragma mark - Controlling Import
- (void)pause{}
- (void)start{}
- (void)cancel{}
- (void)removeFinished{}

- (NSUInteger)items{
    return 0;
}
- (NSUInteger)finishedItems{
    return 0;
}

#pragma mark -
#pragma mark -
#pragma mark Spotlight Importing
@synthesize searchResults;
- (void)discoverRoms:(NSArray*)volumes
{
    // TODO: limit searching or results to the volume URLs only.
    
    NSMutableArray *supportedFileExtensions = [[OESystemPlugin supportedTypeExtensions] mutableCopy];
    
    // We skip common types by default.
    NSArray *commonTypes = [NSArray arrayWithObjects:@"bin", @"zip", @"elf", nil];
    
    [supportedFileExtensions removeObjectsInArray:commonTypes];
    
    //NSLog(@"Supported search Extensions are: %@", supportedFileExtensions);
    
    NSString *searchString = @"";
    for(NSString *extension in supportedFileExtensions)
    {
        searchString = [searchString stringByAppendingFormat:@"(kMDItemDisplayName == *.%@)", extension];
        searchString = [searchString stringByAppendingString:@" || "];
    }
    
    searchString = [searchString substringWithRange:NSMakeRange(0, [searchString length] - 4)];
    
    DLog(@"SearchString: %@", searchString);
    
    MDQueryRef searchQuery = MDQueryCreate(kCFAllocatorDefault, (__bridge CFStringRef)searchString, NULL, NULL);
    
    if(searchQuery)
    {
        // Limit Scope to selected volumes / URLs only
        MDQuerySetSearchScope(searchQuery, (__bridge CFArrayRef) volumes, 0);
        
        DLog(@"Valid search query ref");
        
        [[self searchResults] removeAllObjects];
        
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(finalizeSearchResults:)
                                                     name:(NSString*)kMDQueryDidFinishNotification
                                                   object:(__bridge id)searchQuery];
        
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(updateSearchResults:)
                                                     name:(NSString*)kMDQueryProgressNotification
                                                   object:(__bridge id)searchQuery];
        
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(updateSearchResults:)
                                                     name:(NSString*)kMDQueryDidUpdateNotification
                                                   object:(__bridge id)searchQuery];
        
        if(MDQueryExecute(searchQuery, kMDQueryWantsUpdates))
            NSLog(@"Searching for importable roms");
        else
        {
            CFRelease(searchQuery);
            searchQuery = nil;
            // leave this log message in...
            NSLog(@"MDQuery failed to start.");
        }
        
    }
    else
        NSLog(@"Invalid Search Query");
}

- (void)updateSearchResults:(NSNotification *)notification
{
    DLog(@"updateSearchResults:");
    
    MDQueryRef searchQuery = (__bridge MDQueryRef)[notification object];
    
    
    // If you're going to have the same array for every iteration,
    // don't allocate it inside the loop !
    NSArray *excludedPaths = [NSArray arrayWithObjects:
                              @"System",
                              @"Library",
                              @"Developer",
                              @"Volumes",
                              @"Applications",
                              @"bin",
                              @"cores",
                              @"dev",
                              @"etc",
                              @"home",
                              @"net",
                              @"sbin",
                              @"private",
                              @"tmp",
                              @"usr",
                              @"var",
                              @"ReadMe", // markdown
                              @"readme", // markdown
                              @"README", // markdown
                              @"Readme", // markdown
                              
                              nil];
    
    // assume the latest result is the last index?
    for(CFIndex index = 0, limit = MDQueryGetResultCount(searchQuery); index < limit; index++)
    {
        MDItemRef resultItem = (MDItemRef)MDQueryGetResultAtIndex(searchQuery, index);
        NSString *resultPath = (__bridge_transfer NSString *)MDItemCopyAttribute(resultItem, kMDItemPath);
        
        // Nothing in common
        if([[resultPath pathComponents] firstObjectCommonWithArray:excludedPaths] == nil)
        {
            NSDictionary *resultDict = [[NSDictionary alloc] initWithObjectsAndKeys:
                                        resultPath, @"Path",
                                        [[resultPath lastPathComponent] stringByDeletingPathExtension], @"Name",
                                        nil];
            
            if(![[self searchResults] containsObject:resultDict])
            {
                [[self searchResults] addObject:resultDict];
                
                //                NSLog(@"Result Path: %@", resultPath);
            }
        }
    }
}

- (void)finalizeSearchResults:(NSNotification *)notification
{
    MDQueryRef searchQuery = (__bridge_retained MDQueryRef)[notification object];    
    NSLog(@"Finished searching, found: %lu items", MDQueryGetResultCount(searchQuery));
    
    if(MDQueryGetResultCount(searchQuery))
    {
        [self importInBackground];
        
        MDQueryStop(searchQuery);
    }
    
    CFRelease(searchQuery);
}

- (void)importInBackground;
{
    NSLog(@"importInBackground");
    
    [self importROMsAtPaths:[[self searchResults] valueForKey:@"Path"] inBackground:YES error:nil];;
}

@end
