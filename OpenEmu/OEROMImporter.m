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
#import "OEImportItem.h"

#import "OELibraryDatabase.h"
#import "OEDBRom.h"
#import "OEDBGame.h"
#import "OEDBSystem.h"
#import "OESystemPlugin.h"

#import "NSURL+OELibraryAdditions.h"
#import "NSFileManager+OEHashingAdditions.h"
#import "NSArray+OEAdditions.h"

#import "ArchiveVGThrottling.h"
@interface OEROMImporter ()
{
    dispatch_queue_t dispatchQueue;
}
@property (weak) OELibraryDatabase *database;

- (void)startQueueIfNeeded;
@property BOOL isBusy;
@property(readwrite, retain) NSMutableArray *spotlightSearchResults;
@property int activeImports;
#pragma mark - Import Steps
- (void)performImportStepCheckDirectory:(OEImportItem*)item;
- (void)performImportStepHash:(OEImportItem*)item;
- (void)performImportStepCheckHash:(OEImportItem*)item;
- (void)performImportStepDetermineSystem:(OEImportItem*)item;
- (void)performImportStepSyncArchive:(OEImportItem*)item;
- (void)performImportStepOrganize:(OEImportItem*)item;
- (void)performImportStepCreateRom:(OEImportItem*)item;
- (void)performImportStepCreateGame:(OEImportItem*)item;

- (void)scheduleItemForNextStep:(OEImportItem*)item;
- (void)finishImportForItem:(OEImportItem*)item withError:(NSError*)error;
- (void)cleanupImportForItem:(OEImportItem*)item;
@end

@implementation OEROMImporter
@synthesize database, isBusy, delegate;

- (id)initWithDatabase:(OELibraryDatabase *)aDatabase
{
    self = [super init];
    if (self) {
        [self setDatabase:aDatabase];
        [self setQueue:[NSMutableArray array]];
        [self setSpotlightSearchResults:[NSMutableArray arrayWithCapacity:1]];
        
        dispatchQueue = dispatch_queue_create("org.openemu.importqueue", DISPATCH_QUEUE_SERIAL);
        dispatch_queue_t priority = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_LOW, 0);
        dispatch_set_target_queue(dispatchQueue, priority);
    }
    return self;
}

- (void)startQueueIfNeeded
{
    if([self activeImports] < MaxSimulatenousImports) [self processNextItem];
}

- (void)processNextItem
{
    self.activeImports ++;

    // TODO: we might need a lock here if simultaneous imports are allowed
    OEImportItem * nextItem = [[self queue] firstObjectMatchingBlock:^BOOL(id evaluatedObject) {
        return [evaluatedObject importState]==OEImportItemStatusIdle;
    }];
    if(nextItem)
    {
        [nextItem setImportState:OEImportItemStatusActive];
        dispatch_async(dispatchQueue, ^{
            importBlock(self, nextItem);
        });

        [self startQueueIfNeeded];
    }
    else
        self.activeImports --;
}

- (void)dealloc
{
    dispatch_release(dispatchQueue);
}

const static void (^importBlock)(OEROMImporter *importer, OEImportItem* item) = ^(OEROMImporter *importer, OEImportItem* item){   
    switch ([item importStep]){
        case OEImportStepCheckDirectory: [importer performImportStepCheckDirectory:item]; break;
        case OEImportStepHash: [importer performImportStepHash:item]; break;
        case OEImportStepCheckHash: [importer performImportStepCheckHash:item]; break;
        case OEImportStepDetermineSystem: [importer performImportStepDetermineSystem:item]; break;
        case OEImportStepSyncArchive: [importer performImportStepSyncArchive:item]; break;
        case OEImportStepOrganize: [importer performImportStepOrganize:item]; break;
        case OEImportStepCreateRom: [importer performImportStepCreateRom:item]; break;
        case OEImportStepCreateGame: [importer performImportStepCreateGame:item]; break;
        default: return;
    }
    
    if([item importState]==OEImportItemStatusActive)
        [importer scheduleItemForNextStep:item];
};
#pragma mark - Import Steps
- (void)performImportStepCheckDirectory:(OEImportItem*)item
{
    NSURL *url = [item url];
    if([url isDirectory])
    {
        NSError *error = nil;
        NSArray *contents = [[NSFileManager defaultManager] contentsOfDirectoryAtURL:url includingPropertiesForKeys:nil options:0 error:&error];
        if(!contents)
        {
            [self finishImportForItem:item withError:error];
        }
        else
        {
            NSMutableArray *importItems = [NSMutableArray arrayWithCapacity:[contents count]];
            [contents enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
                OEImportItem *item = [OEImportItem itemWithURL:obj andCompletionHandler:[item completionHandler]];
                if(item)
                    [importItems addObject:item];
            }];
            
            NSUInteger index = [[self queue] indexOfObjectIdenticalTo:item];
            if(index == NSNotFound) // Should never happen
            {
                NSError *error = [NSError errorWithDomain:OEImportErrorDomain code:0 userInfo:nil];
                [self finishImportForItem:item withError:error];
                return;
            }
            
            // TODO: add items after index, not at the end
            [[self queue] addObjectsFromArray:importItems];
            [[self queue] removeObjectIdenticalTo:item];
            [item setImportState:OEImportItemStatusFinished];
            [self processNextItem];
            self.activeImports --;
        }
    }
}

- (void)performImportStepHash:(OEImportItem*)item
{
    NSURL           *url = [item url];
    NSString        *md5, *crc;
    NSError         *error = nil;
    NSFileManager   *fileManager = [NSFileManager defaultManager];
    
    if(![fileManager hashFileAtURL:url md5:&md5 crc32:&crc error:&error])
    {
        DLog(@"unable to hash file, this is probably a fatal error");
        DLog(@"%@", error);
        
        [self finishImportForItem:item withError:error];
    }
    else
    {
        [[item importInfo] setValue:md5 forKey:OEImportInfoMD5];
        [[item importInfo] setValue:crc forKey:OEImportInfoCRC];
        
        // TODO: get file size
    }
}

- (void)performImportStepCheckHash:(OEImportItem*)item
{
    NSError  *error = nil;
    NSString *md5 = [[item importInfo] valueForKey:OEImportInfoMD5];
    NSString *crc = [[item importInfo] valueForKey:OEImportInfoCRC];
    
    OEDBRom *rom = [OEDBRom romWithMD5HashString:md5 inDatabase:[self database] error:&error];
    if(!rom) rom = [OEDBRom romWithCRC32HashString:crc inDatabase:[self database] error:&error];
    
    if(rom)
    {
        NSURL *romURL = [rom URL];
        if(![romURL checkResourceIsReachableAndReturnError:&error])
        {
            DLog(@"rom file not available");
            DLog(@"%@", error);
            // TODO: depending on error finish here with 'already present' success
            // if the error says something like volume could not be found we might want to skip import because the file is probably on an external HD that is currently not connected
            // but if it just says the file was deleted we should replace the rom's url with the new one and continue importing
            [[item importInfo] setValue:[[rom objectID] URIRepresentation] forKey:OEImportInfoROMObjectID];
        }
        else
        {
            DLog(@"rom in db, skipping file");
            // TODO: create an 'error' saying that the file was skipped
            [self finishImportForItem:item withError:nil];
        }
    }
}

- (void)performImportStepDetermineSystem:(OEImportItem*)item
{
    if([[item importInfo] valueForKey:OEImportInfoROMObjectID]) return;
    
    NSError *error        = nil;
    NSURL   *url          = [item url];
    NSArray *validSystems = [OEDBSystem systemsForFileWithURL:url inDatabase:[self database] error:&error];
    if(!validSystems)
    {
        DLog(@"Could not get valid systems");
        DLog(@"%@", error);
        [self finishImportForItem:item withError:error];
    }
    else
    {
        if([validSystems count]==0)
        {
            DLog(@"No valid system found for item at url %@", [item url]);
            // TODO: create unresolvable error
            [self finishImportForItem:item withError:error];
        }
        else
        {
            NSMutableArray *systemIDs = [NSMutableArray arrayWithCapacity:[validSystems count]];
            [validSystems enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
                [systemIDs addObject:[obj systemIdentifier]];
            }];
            [[item importInfo] setValue:systemIDs forKey:OEImportInfoSystemID];
        }
    }
}

- (void)performImportStepSyncArchive:(OEImportItem*)item
{
    if([[item importInfo] valueForKey:OEImportInfoROMObjectID])
    {
        OEDBRom *rom = [OEDBRom romWithURIURL:[[item importInfo] valueForKey:OEImportInfoROMObjectID] inDatabase:[self database]];
        if([rom game] && [[[rom game] archiveID] intValue] != 0) return;
    }
    
    NSUserDefaults  *standardUserDefaults = [NSUserDefaults standardUserDefaults];
    BOOL lookupInfo = [standardUserDefaults boolForKey:UDAutomaticallyGetInfoKey];
    if(!lookupInfo)
        return;

    NSString *md5 = [[item importInfo] valueForKey:OEImportInfoMD5];
    NSString *crc = [[item importInfo] valueForKey:OEImportInfoCRC];
    
    // TODO: set user info in error
    NSError *error = [NSError errorWithDomain:OEImportErrorDomain code:OEImportErrorCodeWaitingForArchiveSync userInfo:nil];
    [self finishImportForItem:item withError:error];
    [[ArchiveVGThrottling throttled] gameInfoByMD5:md5 andCRC:crc withCallback:^(NSDictionary *result, NSError *error) {
        if(error)
            [[item importInfo] setValue:error forKey:OEImportInfoArchiveSync];
        else if(result)
            [[item importInfo] setValue:result forKey:OEImportInfoArchiveSync];
        else
        {
            result = [NSDictionary dictionary];
            [[item importInfo] setValue:result forKey:OEImportInfoArchiveSync];
        }
        
        if([item error] && [[item error] code] == OEImportErrorCodeWaitingForArchiveSync)
        {
            [item setError:nil];
            [item setImportState:OEImportItemStatusIdle];
            item.importStep ++;
            [self startQueueIfNeeded];
        }
    }];
}

- (void)performImportStepOrganize:(OEImportItem*)item
{
    NSUserDefaults *standardUserDefaults = [NSUserDefaults standardUserDefaults];
    BOOL copyToLibrary   = [standardUserDefaults boolForKey:UDCopyToLibraryKey];
    BOOL organizeLibrary = [standardUserDefaults boolForKey:UDOrganizeLibraryKey];
    BOOL lookupInfo      = [standardUserDefaults boolForKey:UDAutomaticallyGetInfoKey];
    
    NSURL *url = [item url];
    
    if(copyToLibrary && ![url isSubpathOfURL:[[self database] romsFolderURL]])
    {
        NSString *fullName  = [url lastPathComponent];
        NSString *extension    = [fullName pathExtension];
        NSString *baseName  = [fullName stringByDeletingPathExtension];
        
        NSURL *unsortedFolder = [[self database] unsortedRomsFolderURL];
        NSURL *romURL       = [unsortedFolder URLByAppendingPathComponent:fullName];
        romURL = [romURL uniqueURLUsingBlock:^NSURL *(NSInteger triesCount) {
            NSString *newName = [NSString stringWithFormat:@"%@ %ld.%@", baseName, triesCount, extension];
            return [unsortedFolder URLByAppendingPathComponent:newName];
        }];
    }
    
    NSMutableDictionary *importInfo = [item importInfo];
    if(organizeLibrary && [url isSubpathOfURL:[[self database] romsFolderURL]])
    {
        OEDBSystem *system = nil;
        if([importInfo valueForKey:OEImportInfoROMObjectID])
        {
            DLog(@"using rom object");

        }
        else if([importInfo valueForKey:OEImportInfoSystemID] && [[importInfo valueForKey:OEImportInfoSystemID] count]==1)
        {
            DLog(@"using system");

        }
        else if([importInfo valueForKey:OEImportInfoArchiveSync])
        {
            DLog(@"using archive info");
        }
        else if(lookupInfo && ![importInfo valueForKey:OEImportInfoArchiveSync])
        {
            DLog(@"waiting for archive info");

            // TODO: set user info in error
            NSError *error = [NSError errorWithDomain:OEImportErrorDomain code:OEImportErrorCodeWaitingForArchiveSync userInfo:nil];
            [self finishImportForItem:item withError:error];
            return;
        }
        
        if(system)
        {
            DLog(@"got system");

            NSString *fullName  = [url lastPathComponent];
            NSString *extension    = [fullName pathExtension];
            NSString *baseName  = [fullName stringByDeletingPathExtension];
            
            NSURL *systemFolder = [[self database] romsFolderURLForSystem:system];
            NSURL *romURL       = [systemFolder URLByAppendingPathComponent:fullName];
            romURL = [romURL uniqueURLUsingBlock:^NSURL *(NSInteger triesCount) {
                NSString *newName = [NSString stringWithFormat:@"%@ %ld.%@", baseName, triesCount, extension];
                return [systemFolder URLByAppendingPathComponent:newName];
            }];
            
            NSError *error = nil;
            if(![[NSFileManager defaultManager] moveItemAtURL:url toURL:romURL error:&error])
            {
                [self finishImportForItem:item withError:error];
            }
            else
                [item setUrl:url];
            }
        else
        {
            DLog(@"no system");
            // TODO: set user info in error
            NSError *error = [NSError errorWithDomain:OEImportErrorDomain code:OEImportErrorCodeMultipleSystems userInfo:nil];
            [self finishImportForItem:item withError:error];
        }
    }
}

- (void)performImportStepCreateRom:(OEImportItem*)item
{
    NSMutableDictionary *importInfo = [item importInfo];
    if([importInfo valueForKey:OEImportInfoROMObjectID])
    {
        OEDBRom *rom = [OEDBRom romWithURIURL:[importInfo valueForKey:OEImportInfoROMObjectID] inDatabase:[self database]];
        [rom setURL:[item url]];
        [self finishImportForItem:item withError:nil];
        return;
    }
    
    NSError *error = nil;
    NSString *md5 = [importInfo valueForKey:OEImportInfoMD5];
    NSString *crc = [importInfo valueForKey:OEImportInfoCRC];
    OEDBRom *rom = [OEDBRom createRomWithURL:[item url] md5:md5 crc:crc inDatabase:[self database] error:&error];
    if(!rom)
    {
        [self finishImportForItem:item withError:error];
        return;
    }
    
    // TODO: set file size

    NSURL *objectIDURIRep = [[rom objectID] URIRepresentation];
    [importInfo setValue:objectIDURIRep forKey:OEImportInfoROMObjectID];
}

- (void)performImportStepCreateGame:(OEImportItem*)item
{    
    NSMutableDictionary *importInfo = [item importInfo];
    
    NSError *error = nil;
    OEDBGame *game = nil;
    OEDBRom  *rom  = [OEDBRom romWithURIURL:[importInfo valueForKey:OEImportInfoROMObjectID]];
    if(!rom || [rom game]) return;
    
    id archiveResult = [importInfo valueForKey:OEImportInfoArchiveSync];
    if([archiveResult isKindOfClass:[NSError class]])
    {
        DLog(@"archiveError: %@", archiveResult);
    }
    else if(archiveResult)
    {
        game = [OEDBGame gameWithArchiveDictionary:archiveResult inDatabase:[self database]];
    }
    
    if(!game)
    {
        if([[importInfo valueForKey:OEImportInfoSystemID] count]>1)
        {
            // TODO: set user info in error
            error = [NSError errorWithDomain:OEImportErrorDomain code:OEImportErrorCodeMultipleSystems userInfo:nil];
            [self finishImportForItem:item withError:error];            
        }
        else
        {
            NSString *systemIdentifier = [[importInfo valueForKey:OEImportInfoSystemID] lastObject];
            OEDBSystem *system = [OEDBSystem systemForPluginIdentifier:systemIdentifier inDatabase:[self database]];
            if(!system)
            {
                //TODO: create proper error
                error = [NSError errorWithDomain:OEImportErrorDomain code:51 userInfo:nil];
                 [self finishImportForItem:item withError:error];
            }
            else
            {
                NSURL *url = [rom URL];
                NSString *gameTitleWithSuffix = [url lastPathComponent];
                NSString *gameTitleWithoutSuffix = [gameTitleWithSuffix stringByDeletingPathExtension];

                game = [OEDBGame createGameWithName:gameTitleWithoutSuffix andSystem:system inDatabase:[self database]];
            }
        }
    }
    
    if(game)
    {
        [rom setGame:game];
        [self finishImportForItem:item withError:nil];
    }
}

- (void)scheduleItemForNextStep:(OEImportItem*)item
{
    item.importStep ++;
    dispatch_async(dispatchQueue, ^{
        importBlock(self, item);

    });
}

- (void)finishImportForItem:(OEImportItem*)item withError:(NSError*)error
{
    if(error && [[error domain] isEqualTo:OEImportErrorDomain] && [error code]>=OEImportMinFatalErrorCode) [item setImportState:OEImportItemStatusFatalError];
    else if(error) [item setImportState:OEImportItemStatusResolvableError];
    else [item setImportState:OEImportItemStatusFinished];
    
    [item setError:error];
    self.activeImports --;

    [self cleanupImportForItem:item];
    [self startQueueIfNeeded];
    
    if([item completionHandler] && ([item importState] == OEImportItemStatusFinished || [item importState]==OEImportItemStatusFatalError))
        dispatch_async(dispatch_get_main_queue(), ^{
            [item completionHandler]();
        });
}

- (void)cleanupImportForItem:(OEImportItem*)item
{
    NSError *error = [item error];
    if(error && [[error domain] isEqualTo:OEImportErrorDomain] && [error code]< OEImportMinFatalErrorCode) return;
    
    if([item importState] == OEImportItemStatusFinished)
        [[self database] save:nil];
    
    [[item importInfo] removeObjectForKey:OEImportInfoArchiveSync];
    [[item importInfo] removeObjectForKey:OEImportInfoCRC];
    [[item importInfo] removeObjectForKey:OEImportInfoMD5];
    [[item importInfo] removeObjectForKey:OEImportInfoSystemID];
}
#pragma mark - Importing Items with completion handler
- (void)importItemAtPath:(NSString*)path withCompletionHandler:(OEImportItemCompletionBlock)handler
{
    NSURL *url = [NSURL fileURLWithPath:path];
    [self importItemAtURL:url withCompletionHandler:handler];
}

- (void)importItemsAtPaths:(NSArray*)paths withCompletionHandler:(OEImportItemCompletionBlock)handler
{
    [paths enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop)
     {
         [self importItemAtPath:obj withCompletionHandler:handler];
     }];
}

- (void)importItemAtURL:(NSURL*)url withCompletionHandler:(OEImportItemCompletionBlock)handler
{
    id item = [[self queue] firstObjectMatchingBlock:^BOOL(id item) {
        return [[item url] isEqualTo:url];
    }];
    
    if(!item)
    {
        OEImportItem *item = [OEImportItem itemWithURL:url andCompletionHandler:handler];
        
        [[self queue] addObject:item];
        [self startQueueIfNeeded];
    }
}

- (void)importItemsAtURLs:(NSArray*)urls withCompletionHandler:(OEImportItemCompletionBlock)handler
{
    [urls enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop)
     {
         [self importItemAtURL:obj withCompletionHandler:handler];
     }];
}

#pragma mark - Importing Items without completion handler
- (void)importItemAtPath:(NSString*)path
{
    [self importItemAtPath:path withCompletionHandler:nil];
}

- (void)importItemsAtPaths:(NSArray*)paths
{
    [self importItemsAtPaths:paths withCompletionHandler:nil];
}

- (void)importItemAtURL:(NSURL*)url
{
    [self importItemAtURL:url withCompletionHandler:nil];
}

- (void)importItemsAtURLs:(NSArray*)urls
{
    [self importItemsAtURLs:urls withCompletionHandler:nil];
}

#pragma mark - Handle Spotlight importing
@synthesize spotlightSearchResults;
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
                
        [[self spotlightSearchResults] removeAllObjects];
        
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
            
            if(![[self spotlightSearchResults] containsObject:resultDict])
            {
                [[self spotlightSearchResults] addObject:resultDict];
                
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
    [self importItemsAtPaths:[[self spotlightSearchResults] valueForKey:@"Path"]];
}

#pragma mark - Controlling Import
- (void)pause
{
    DLog(@"");
}

- (void)start
{
    DLog(@"");
}

- (void)cancel
{
    DLog(@"");
}

- (void)removeFinished
{
    DLog(@"");
    [[self queue] filterUsingPredicate:[NSPredicate predicateWithBlock:^BOOL(OEImportItem *evaluatedObject, NSDictionary *bindings)
    {
        return [evaluatedObject importState] != OEImportItemStatusFinished && [evaluatedObject importState] != OEImportItemStatusFatalError;
    }]];
}

- (NSUInteger)numberOfItems
{
    return [[self queue] count];
}

- (NSUInteger)finishedItems
{
    return [[[self queue] filteredArrayUsingPredicate:[NSPredicate predicateWithBlock:^BOOL(OEImportItem *evaluatedObject, NSDictionary *bindings)
    {
        return [evaluatedObject importState] == OEImportItemStatusFinished;
    }]] count];
}
@end
