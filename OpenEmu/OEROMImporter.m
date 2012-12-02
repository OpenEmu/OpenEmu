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
#import "OESystemController.h"

#import "NSURL+OELibraryAdditions.h"
#import "NSFileManager+OEHashingAdditions.h"
#import "NSArray+OEAdditions.h"

#import "ArchiveVGThrottling.h"

#import "OECUESheet.h"

#import <CommonCrypto/CommonDigest.h>
#import <XADMaster/XADArchive.h>
#import <objc/runtime.h>

const int MaxSimulatenousImports = 1; // imports can't really be simulatenous because access to queue is not ready for multithreadding right now

#pragma mark User Default Keys -
NSString *const OEOrganizeLibraryKey       = @"organizeLibrary";
NSString *const OECopyToLibraryKey         = @"copyToLibrary";
NSString *const OEAutomaticallyGetInfoKey  = @"automaticallyGetInfo";

#pragma mark Error Codes -
NSString *const OEImportErrorDomainFatal      = @"OEImportFatalDomain";
NSString *const OEImportErrorDomainResolvable = @"OEImportResolvableDomain";
NSString *const OEImportErrorDomainSuccess    = @"OEImportSuccessDomain";

#pragma mark Import Info Keys -
NSString *const OEImportInfoMD5         = @"md5";
NSString *const OEImportInfoCRC         = @"crc";
NSString *const OEImportInfoROMObjectID = @"RomObjectID";
NSString *const OEImportInfoSystemID    = @"systemID";
NSString *const OEImportInfoArchiveSync = @"archiveSync";
NSString *const OEImportInfoArchivedFileURL = @"archivedFileURL";

@interface OEROMImporter ()
{
    dispatch_queue_t dispatchQueue;
}

@property(readwrite)            NSInteger          status;
@property(readwrite)            NSInteger          activeImports;
@property(readwrite)            NSInteger          numberOfProcessedItems;
@property(readwrite, nonatomic) NSInteger          totalNumberOfItems;
@property(readwrite)            NSMutableArray    *queue;
@property(readwrite, strong)    NSMutableArray    *spotlightSearchResults;
@property(weak)                 OELibraryDatabase *database;

- (void)startQueueIfNeeded;

@property(readwrite, retain) NSMutableSet *unsavedMD5Hashes;
@property(readwrite, retain) NSMutableSet *unsavedCRCHashes;

#pragma mark - Import Steps
- (void)performImportStepCheckDirectory:(OEImportItem *)item;
- (void)performImportStepCheckArchiveFile:(OEImportItem *)item;
- (void)performImportStepHash:(OEImportItem *)item;
- (void)performImportStepCheckHash:(OEImportItem *)item;
- (void)performImportStepDetermineSystem:(OEImportItem *)item;
- (void)performImportStepSyncArchive:(OEImportItem *)item;
- (void)performImportStepOrganize:(OEImportItem *)item;
- (void)performImportStepOrganizeAdditionalFiles:(OEImportItem *)item;
- (void)performImportStepCreateRom:(OEImportItem *)item;
- (void)performImportStepCreateGame:(OEImportItem *)item;

- (void)scheduleItemForNextStep:(OEImportItem *)item;
- (void)stopImportForItem:(OEImportItem *)item withError:(NSError *)error;
- (void)cleanupImportForItem:(OEImportItem *)item;

- (void)OE_performSelectorOnDelegate:(SEL)selector withObject:(id)object;
@end

@implementation OEROMImporter
@synthesize database, delegate;
@synthesize spotlightSearchResults;

+ (void)initialize
{
    if(self != [OEROMImporter class]) return;

    [[NSUserDefaults standardUserDefaults] registerDefaults:(@{
                                                             OEOrganizeLibraryKey      : @(YES),
                                                             OECopyToLibraryKey        : @(YES),
                                                             OEAutomaticallyGetInfoKey : @(YES),
                                                             })];
}

- (id)initWithDatabase:(OELibraryDatabase *)aDatabase
{
    self = [super init];
    if (self) {
        [self setDatabase:aDatabase];
        [self setQueue:[NSMutableArray array]];
        [self setSpotlightSearchResults:[NSMutableArray arrayWithCapacity:1]];
        [self setNumberOfProcessedItems:0];
        
        [self setUnsavedCRCHashes:[NSMutableSet set]];
        [self setUnsavedMD5Hashes:[NSMutableSet set]];
        /*
        dispatchQueue = dispatch_get_main_queue();
         */

        dispatchQueue = dispatch_queue_create("org.openemu.importqueue", DISPATCH_QUEUE_SERIAL);
        dispatch_queue_t priority = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_LOW, 0);
        dispatch_set_target_queue(dispatchQueue, priority);
        [self setStatus:OEImporterStatusStopped];
    }
    return self;
}

- (void)startQueueIfNeeded
{
    if([self status] == OEImporterStatusRunning && [self activeImports] < MaxSimulatenousImports)
    {
        [self processNextItem];
    }
}

- (void)processNextItem
{
    self.activeImports++;
    // TODO: we need a lock here if simultaneous imports are allowed
    
    OEImportItem *nextItem = [[self queue] firstObjectMatchingBlock:^BOOL (id evaluatedObject)
                              {
                                  return [evaluatedObject importState] == OEImportItemStatusIdle;
                              }];
    if(nextItem != nil)
    {
        [nextItem setImportState:OEImportItemStatusActive];
        dispatch_async(dispatchQueue, ^{
            importBlock(self, nextItem);
            [self OE_performSelectorOnDelegate:@selector(romImporter:startedProcessingItem:) withObject:nextItem];
        });
        
        if(MaxSimulatenousImports > 1) dispatch_async(dispatchQueue, ^{
            [self startQueueIfNeeded];
        });
    }
    else
    {
        self.activeImports--;
        if([self numberOfProcessedItems] == [self totalNumberOfItems])
        {
            dispatch_async(dispatchQueue, ^{
                if([[self queue] count] == 0)
                {
                    [self setQueue:[NSMutableArray array]];
                    [self setNumberOfProcessedItems:0];
                    [self setTotalNumberOfItems:0];
                    
                    [self OE_performSelectorOnDelegate:@selector(romImporterDidFinish:) withObject:nil];
                    [self setStatus:OEImporterStatusStopped];
                }
                else
                    [self startQueueIfNeeded];
            });
        }
    }
}

- (void)dealloc
{
    dispatch_release(dispatchQueue);
}

#pragma mark - Import Block

static void importBlock(OEROMImporter *importer, OEImportItem *item)
{
    if([importer status] == OEImporterStatusPausing || [importer status] == OEImporterStatusPaused)
    {
        DLog(@"skipping item!");
        importer.activeImports--;
        if([item importState] == OEImportItemStatusActive)
            [item setImportState:OEImportItemStatusIdle];
    }
    else if([importer status] == OEImporterStatusStopping || [importer status] == OEImporterStatusStopped)
    {
        importer.activeImports--;
        [item setError:nil];
        [item setImportState:OEImportItemStatusCancelled];
        [importer cleanupImportForItem:item];
        DLog(@"deleting item!");
    }
    else
    {
        [importer OE_performSelectorOnDelegate:@selector(romImporter:changedProcessingPhaseOfItem:) withObject:item];
        switch([item importStep])
        {
            case OEImportStepCheckDirectory  : [importer performImportStepCheckDirectory:item];  break;
            case OEImportStepCheckArchiveFile : [importer performImportStepCheckArchiveFile:item]; break;
            case OEImportStepHash            : [importer performImportStepHash:item];            break;
            case OEImportStepCheckHash       : [importer performImportStepCheckHash:item];       break;
            case OEImportStepDetermineSystem : [importer performImportStepDetermineSystem:item]; break;
            case OEImportStepSyncArchive     : [importer performImportStepSyncArchive:item];     break;
            case OEImportStepOrganize        : [importer performImportStepOrganize:item];        break;
            case OEImportStepOrganizeAdditionalFiles : [importer performImportStepOrganizeAdditionalFiles:item]; break;
            case OEImportStepCreateRom       : [importer performImportStepCreateRom:item];       break;
            case OEImportStepCreateGame      : [importer performImportStepCreateGame:item];      break;
            default : return;
        }
        
        if([item importState] == OEImportItemStatusActive)
            [importer scheduleItemForNextStep:item];
    }
}

#pragma mark - Import Steps
// Checks if item.url points to a directory and adds its contents to the queue (by replacing item)
- (void)performImportStepCheckDirectory:(OEImportItem *)item
{
    NSURL *url = [item URL];
    if([url isDirectory])
    {
        NSError *error = nil;
        NSArray *contents = [[NSFileManager defaultManager] contentsOfDirectoryAtURL:url includingPropertiesForKeys:nil options:0 error:&error];
        if(contents == nil)
        {
            [self stopImportForItem:item withError:error];
        }
        else
        {
            NSMutableArray *importItems = [NSMutableArray arrayWithCapacity:[contents count]];
            [contents enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
                OEImportItem *subItem = [OEImportItem itemWithURL:obj andCompletionHandler:[item completionHandler]];
                if(subItem)
                    [importItems addObject:subItem];
            }];
            
            NSUInteger index = [[self queue] indexOfObjectIdenticalTo:item];
            if(index == NSNotFound) // Should never happen
            {
                // TODO: set proper error code
                NSError *error = [NSError errorWithDomain:OEImportErrorDomainFatal code:0 userInfo:nil];
                [self stopImportForItem:item withError:error];
            }
            else
            {
                // TODO: add items after index, not at the end
                [item setImportState:OEImportItemStatusFinished];
                [[self queue] addObjectsFromArray:importItems];
                [[self queue] removeObjectIdenticalTo:item];
                self.totalNumberOfItems += ([importItems count]-1);
                [self processNextItem];
                self.activeImports--;
            }
        }
    }
}

- (void)performImportStepCheckArchiveFile:(OEImportItem *)item
{
    //Short circuit this?
    NSString *path = [[item URL] path];
    XADArchive *archive = [XADArchive archiveForFile:path];
    if (archive && [archive numberOfEntries] == 1)
    {
        NSString *formatName = [archive formatName];
        if ([formatName isEqualToString:@"MacBinary"])
            return;
        
        if (![archive entryHasSize:0] || [archive entryIsEncrypted:0] || [archive entryIsDirectory:0] || [archive entryIsArchive:0])
            return;
        
        NSString *folder = temporaryDirectoryForDecompressionOfPath(path);
        NSString *name = [archive nameOfEntry:0];
        NSString *tmpPath = [folder stringByAppendingPathComponent:name];

        BOOL isdir;
        NSURL *tmpURL = [NSURL fileURLWithPath:tmpPath];
        NSFileManager *fm = [NSFileManager new];
        if ([fm fileExistsAtPath:tmpPath isDirectory:&isdir] && !isdir) {
            DLog(@"Found existing decompressed ROM for path %@", path);
            [[item importInfo] setValue:tmpURL forKey:OEImportInfoArchivedFileURL];
            return;
        }

        BOOL success = [archive extractEntry:0 to:folder];
        if (success)
            [[item importInfo] setValue:tmpURL forKey:OEImportInfoArchivedFileURL];
        else
            [fm removeItemAtPath:folder error:nil];
    }
}

// Calculates md5 and crc32 hash strings
- (void)performImportStepHash:(OEImportItem *)item
{
    NSURL         *url = [[item importInfo] valueForKey:OEImportInfoArchivedFileURL] ?: [item URL];
    NSString      *md5, *crc;
    NSError       *error = nil;
    NSFileManager *fileManager = [NSFileManager defaultManager];
    
    
    if(![fileManager hashFileAtURL:url md5:&md5 crc32:&crc error:&error])
    {
        DLog(@"unable to hash file, this is probably a fatal error");
        DLog(@"%@", error);
        
        [self stopImportForItem:item withError:error];
    }
    else
    {
        [[item importInfo] setValue:md5 forKey:OEImportInfoMD5];
        [[item importInfo] setValue:crc forKey:OEImportInfoCRC];
    }
}

// Checks if hash is already known, if so skips the file
- (void)performImportStepCheckHash:(OEImportItem *)item
{
    NSError  *error = nil;
    NSString *md5 = [[item importInfo] valueForKey:OEImportInfoMD5];
    NSString *crc = [[item importInfo] valueForKey:OEImportInfoCRC];
    
    OEDBRom *rom = [OEDBRom romWithMD5HashString:md5 inDatabase:[self database] error:&error];
    if(rom == nil) rom = [OEDBRom romWithCRC32HashString:crc inDatabase:[self database] error:&error];
    
    if(rom != nil)
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
            // TODO: set user info for error
            NSError *error = [NSError errorWithDomain:OEImportErrorDomainSuccess code:OEImportErrorCodeAlreadyInDatabase userInfo:nil];
            [self stopImportForItem:item withError:error];
        }
    }
    else
    {
        if([[self unsavedCRCHashes] containsObject:crc] || [[self unsavedMD5Hashes] containsObject:md5])
        {
            NSError *error = [NSError errorWithDomain:OEImportErrorDomainSuccess code:OEImportErrorCodeAlreadyInDatabase userInfo:nil];
            [self stopImportForItem:item withError:error];
        }
        
        [[self unsavedCRCHashes] addObject:crc];
        [[self unsavedMD5Hashes] addObject:md5];
    }
}

// Tries to find out which system the file belongs to
- (void)performImportStepDetermineSystem:(OEImportItem *)item
{
    if([[item importInfo] valueForKey:OEImportInfoROMObjectID]) return;
    
    NSString *systemIdentifier = nil;
    id archiveResult = [[item importInfo] valueForKey:OEImportInfoArchiveSync];
    if(archiveResult != nil) // If archive sync has finished and returned to determine system
    {
        if([archiveResult isKindOfClass:[NSDictionary class]] && [archiveResult valueForKey:AVGGameSystemNameKey])
        {
            // Try to find a system for archvie name
            NSString *archiveSystemName = [archiveResult valueForKey:AVGGameSystemNameKey];
            OEDBSystem *system = [OEDBSystem systemForArchiveName:archiveSystemName inDatabase:[self database]];
            
            if(system)
            {
                systemIdentifier = [system systemIdentifier];
                [[item importInfo] setValue:[NSArray arrayWithObject:systemIdentifier] forKey:OEImportInfoSystemID];
                return;
            }
        }
    }
    else
    {
        NSError *error        = nil;
        NSURL   *url          = [[item importInfo] valueForKey:OEImportInfoArchivedFileURL] ?: [item URL];
        NSArray *validSystems = [OEDBSystem systemsForFileWithURL:url inDatabase:[self database] error:&error];
        
        if(validSystems == nil)
        {
            DLog(@"Could not get valid systems");
            DLog(@"%@", error);
            [self stopImportForItem:item withError:error];
            return;
        }
        else if([validSystems count] == 0)
        {
            DLog(@"No valid system found for item at url %@", url);
            // TODO: create unresolvable error
            error = [NSError errorWithDomain:OEImportErrorDomainFatal code:OEImportErrorCodeNoSystem userInfo:nil];
            [self stopImportForItem:item withError:error];
            return;
        }
        else if([validSystems count] == 1)
        {
            systemIdentifier = [[validSystems lastObject] systemIdentifier];
        }
        else
        {
            OECUESheet* cueSheet = [[OECUESheet alloc] initWithPath:[[item sourceURL] path]];
            NSString *dataTrackPath = [cueSheet dataTrackPath];
            
            NSMutableArray *verifiedSystemIDs = [NSMutableArray arrayWithCapacity:[validSystems count]];
            NSMutableArray *systemIDs = [NSMutableArray arrayWithCapacity:[validSystems count]];
            [validSystems enumerateObjectsUsingBlock:^(OEDBSystem *system, NSUInteger idx, BOOL *stop){
                NSString *systemIdentifier = [system systemIdentifier];
                if([[[system plugin] controller] canHandleFile:dataTrackPath])
                    [verifiedSystemIDs addObject:systemIdentifier];
                [systemIDs addObject:systemIdentifier];
            }];
            
            if([verifiedSystemIDs count]==1)
            {
                systemIdentifier = [verifiedSystemIDs lastObject];
            }
            else
            {
                [[item importInfo] setValue:systemIDs forKey:OEImportInfoSystemID];
                return;
            }
        }
    }
    
    if(systemIdentifier == nil)
    {
        NSDictionary *userInfo = [NSDictionary dictionaryWithObject:@"Aaargh, too many systems. You need to choose one!" forKey:NSLocalizedDescriptionKey];
        NSError *error = [NSError errorWithDomain:OEImportErrorDomainResolvable code:OEImportErrorCodeMultipleSystems userInfo:userInfo];
        [self stopImportForItem:item withError:error];
    }
    else
        [[item importInfo] setValue:[NSArray arrayWithObject:systemIdentifier] forKey:OEImportInfoSystemID];
}

- (void)performImportStepSyncArchive:(OEImportItem *)item
{
    if([[item importInfo] valueForKey:OEImportInfoROMObjectID] != nil)
    {
        OEDBRom *rom = [OEDBRom romWithURIURL:[[item importInfo] valueForKey:OEImportInfoROMObjectID] inDatabase:[self database]];
        if([rom game] && [[[rom game] archiveID] intValue] != 0) return;
    }
    
    if([[item importInfo] valueForKey:OEImportInfoArchiveSync] != nil)
        return;
    
    if(![[NSUserDefaults standardUserDefaults] boolForKey:OEAutomaticallyGetInfoKey]) return;
    
    NSString *md5 = [[item importInfo] valueForKey:OEImportInfoMD5];
    NSString *crc = [[item importInfo] valueForKey:OEImportInfoCRC];
    
    // TODO: localize user info in error
    NSDictionary *userInfo = [NSDictionary dictionaryWithObject:@"Waiting for Archive.VG sync" forKey:NSLocalizedDescriptionKey];
    NSError *error = [NSError errorWithDomain:OEImportErrorDomainResolvable code:OEImportErrorCodeWaitingForArchiveSync userInfo:userInfo];
    [self stopImportForItem:item withError:error];
    
    [[ArchiveVGThrottling throttled] gameInfoByMD5:md5 andCRC:crc withCallback:
     ^(NSDictionary *result, NSError *error)
     {
         if(error != nil)
             [[item importInfo] setValue:error forKey:OEImportInfoArchiveSync];
         else if(result != nil)
             [[item importInfo] setValue:result forKey:OEImportInfoArchiveSync];
         else
         {
             result = [NSDictionary dictionary];
             [[item importInfo] setValue:result forKey:OEImportInfoArchiveSync];
         }
         
         if([[[item error] domain] isEqualTo:OEImportErrorDomainResolvable] && [[item error] code] == OEImportErrorCodeWaitingForArchiveSync)
         {
             [item setError:nil];
             
             if([[[item importInfo] valueForKey:OEImportInfoSystemID] count] != 1)
                 [item setImportStep:OEImportStepDetermineSystem];
             else
                 item.importStep++;
             
             [item setImportState:OEImportItemStatusIdle];
             [self startQueueIfNeeded];
         }
     }];
}

- (void)performImportStepOrganize:(OEImportItem *)item
{
    NSUserDefaults *standardUserDefaults = [NSUserDefaults standardUserDefaults];
    BOOL copyToLibrary   = [standardUserDefaults boolForKey:OECopyToLibraryKey];
    BOOL organizeLibrary = [standardUserDefaults boolForKey:OEOrganizeLibraryKey];
    BOOL lookupInfo      = [standardUserDefaults boolForKey:OEAutomaticallyGetInfoKey];
    
    NSURL *url = [item URL];
    
    NSError *error        = nil;
    BOOL    romFileLocked = NO;
    if([[[[NSFileManager defaultManager] attributesOfItemAtPath:[url path] error:&error] objectForKey:NSFileImmutable] boolValue])
    {
        romFileLocked = YES;
        [[NSFileManager defaultManager] setAttributes:@{ NSFileImmutable: @(FALSE) } ofItemAtPath:[url path] error:nil];
    }

    if(copyToLibrary && ![url isSubpathOfURL:[[self database] romsFolderURL]])
    {
        NSString *fullName  = [url lastPathComponent];
        NSString *extension = [fullName pathExtension];
        NSString *baseName  = [fullName stringByDeletingPathExtension];
        
        NSURL *unsortedFolder = [[self database] unsortedRomsFolderURL];
        NSURL *romURL       = [unsortedFolder URLByAppendingPathComponent:fullName];
        romURL = [romURL uniqueURLUsingBlock:^NSURL *(NSInteger triesCount) {
            NSString *newName = [NSString stringWithFormat:@"%@ %ld.%@", baseName, triesCount, extension];
            return [unsortedFolder URLByAppendingPathComponent:newName];
        }];
    
        [[NSFileManager defaultManager] copyItemAtURL:url toURL:romURL error:&error];
        if(romFileLocked)
            [[NSFileManager defaultManager] setAttributes:@{ NSFileImmutable: @(YES) } ofItemAtPath:[url path] error:&error];

        if(error != nil)
        {
            [self stopImportForItem:item withError:error];
            return;
        }
        
        url = [romURL copy];
        [item setURL:url];
    }
    
    NSMutableDictionary *importInfo = [item importInfo];
    if(organizeLibrary && [url isSubpathOfURL:[[self database] romsFolderURL]])
    {
        if([[[NSFileManager defaultManager] attributesOfItemAtPath:[url path] error:&error] objectForKey:NSFileImmutable])
        {
            [[NSFileManager defaultManager] setAttributes:@{ NSFileImmutable: @(FALSE) } ofItemAtPath:[url path] error:nil];
        }
        
        OEDBSystem *system = nil;
        if([importInfo valueForKey:OEImportInfoROMObjectID] != nil)
        {
            DLog(@"using rom object");
            NSURL *objectID = [importInfo valueForKey:OEImportInfoROMObjectID];
            OEDBRom *rom = [OEDBRom romWithURIURL:objectID inDatabase:[self database]];
            system = [[rom game] system];
        }
        else if([[importInfo valueForKey:OEImportInfoSystemID] count] == 1)
        {
            NSString *systemIdentifier = [[importInfo valueForKey:OEImportInfoSystemID] lastObject];
            system = [OEDBSystem systemForPluginIdentifier:systemIdentifier inDatabase:[self database]];
            
        }
        else if([importInfo valueForKey:OEImportInfoArchiveSync] != nil)
        {
            DLog(@"using archive info – Not Implemented Yet");
        }
        else if(lookupInfo && [importInfo valueForKey:OEImportInfoArchiveSync] == nil)
        {
            DLog(@"waiting for archive info");
            
            // TODO: localize user info in error
            NSDictionary *userInfo = [NSDictionary dictionaryWithObject:@"Waiting for Archive.VG sync" forKey:NSLocalizedDescriptionKey];
            NSError *error = [NSError errorWithDomain:OEImportErrorDomainResolvable code:OEImportErrorCodeWaitingForArchiveSync userInfo:userInfo];
            [self stopImportForItem:item withError:error];
            return;
        }
        
        if(system != nil)
        {
            NSString *fullName  = [url lastPathComponent];
            NSString *extension = [fullName pathExtension];
            NSString *baseName  = [fullName stringByDeletingPathExtension];
            
            NSURL *systemFolder = [[self database] romsFolderURLForSystem:system];
            // FIXME: The initialization is pointless here.
            NSURL *romURL       = [systemFolder URLByAppendingPathComponent:fullName];
            romURL = [romURL uniqueURLUsingBlock:
                      ^ NSURL *(NSInteger triesCount)
                      {
                          NSString *newName = [NSString stringWithFormat:@"%@ %ld.%@", baseName, triesCount, extension];
                          return [systemFolder URLByAppendingPathComponent:newName];
                      }];
            
            NSError *error = nil;
            if(![[NSFileManager defaultManager] moveItemAtURL:url toURL:romURL error:&error])
                [self stopImportForItem:item withError:error];
            else
                [item setURL:romURL];
        }
        else
        {
            DLog(@"no system");
            // TODO: localize user info in error
            NSDictionary *userInfo = [NSDictionary dictionaryWithObject:@"Could not find a valid system" forKey:NSLocalizedDescriptionKey];
            NSError *error = [NSError errorWithDomain:OEImportErrorDomainResolvable code:OEImportErrorCodeMultipleSystems userInfo:userInfo];
            [self stopImportForItem:item withError:error];
        }
    }
}

- (void)performImportStepOrganizeAdditionalFiles:(OEImportItem *)item
{
    NSUserDefaults *standardUserDefaults = [NSUserDefaults standardUserDefaults];
    BOOL copyToLibrary   = [standardUserDefaults boolForKey:OECopyToLibraryKey];
    BOOL organizeLibrary = [standardUserDefaults boolForKey:OEOrganizeLibraryKey];
    if((copyToLibrary || organizeLibrary) && [[[[item URL] pathExtension] lastPathComponent] isEqualToString:@"cue"])
    {
        NSString *referencedFilesDirectory = [[[item sourceURL] path] stringByDeletingLastPathComponent];
        OECUESheet *cue = [[OECUESheet alloc] initWithPath:[[item URL] path] andReferencedFilesDirectory:referencedFilesDirectory];
        if(cue == nil)
        {
            // TODO: Create user info
            NSError *error = [NSError errorWithDomain:OEImportErrorDomainFatal code:OEImportErrorCodeInvalidFile userInfo:nil];
            [self stopImportForItem:item withError:error];
            return;
        }
        
        if(![cue allFilesAvailable])
        {
            // TODO: Create user info
            NSError *error = [NSError errorWithDomain:OEImportErrorDomainFatal code:OEImportErrorCodeAdditionalFiles userInfo:nil];
            [self stopImportForItem:item withError:error];
            return;
        }
        
        NSURL *sourceURL = [item sourceURL], *url = [item URL];
        NSString *targetDirectory = [[url path] stringByDeletingLastPathComponent];
        if(copyToLibrary && ![sourceURL isSubpathOfURL:[[self database] romsFolderURL]])
        {
            DLog(@"copy to '%@'", targetDirectory);
            NSError *error = nil;
            if(![cue copyReferencedFilesToPath:targetDirectory withError:&error])
            {
                DLog(@"%@", error);
                [self stopImportForItem:item withError:error];
                return;
            }
        }
        else if(organizeLibrary && [sourceURL isSubpathOfURL:[[self database] romsFolderURL]])
        {
            DLog(@"move to '%@'", targetDirectory);
            NSError *error = nil;
            if(![cue moveReferencedFilesToPath:targetDirectory withError:&error])
            {
                DLog(@"%@", error);
                [self stopImportForItem:item withError:error];
                return;
            }
        }
    }
}

- (void)performImportStepCreateRom:(OEImportItem *)item
{
    NSMutableDictionary *importInfo = [item importInfo];
    if([importInfo valueForKey:OEImportInfoROMObjectID] != nil)
    {
        OEDBRom *rom = [OEDBRom romWithURIURL:[importInfo valueForKey:OEImportInfoROMObjectID] inDatabase:[self database]];
        [rom setURL:[item URL]];
        [self stopImportForItem:item withError:nil];
        return;
    }
    
    NSError *error = nil;
    NSString *md5 = [importInfo valueForKey:OEImportInfoMD5];
    NSString *crc = [importInfo valueForKey:OEImportInfoCRC];
    OEDBRom *rom = [OEDBRom createRomWithURL:[item URL] md5:md5 crc:crc inDatabase:[self database] error:&error];
    if(rom == nil)
    {
        [self stopImportForItem:item withError:error];
        return;
    }
    
    NSURL *objectIDURIRep = [[rom objectID] URIRepresentation];
    [importInfo setValue:objectIDURIRep forKey:OEImportInfoROMObjectID];
}

- (void)performImportStepCreateGame:(OEImportItem *)item
{
    NSMutableDictionary *importInfo = [item importInfo];
    
    NSError *error = nil;
    OEDBGame *game = nil;
    OEDBRom  *rom  = [OEDBRom romWithURIURL:[importInfo valueForKey:OEImportInfoROMObjectID]];
    if(rom == nil || [rom game] != nil) return;
    
    id archiveResult = [importInfo valueForKey:OEImportInfoArchiveSync];
    NSDictionary *archiveDict = nil;

    if([archiveResult isKindOfClass:[NSError class]])
        DLog(@"archiveError: %@", archiveResult);
    else if(archiveResult != nil)
    {
        archiveDict = archiveResult;
        //game = [OEDBGame gameWithArchiveID:[archiveResult valueForKey:AVGGameIDKey] error:&error];
    }
    
    if(game == nil)
    {
        if([[importInfo valueForKey:OEImportInfoSystemID] count] > 1)
        {
            if([archiveDict valueForKey:AVGGameSystemNameKey] == nil)
            {
                [[game managedObjectContext] deleteObject:game];
                
                // TODO: localize user info in error
                NSDictionary *userInfo = [NSDictionary dictionaryWithObject:@"Aaargh, too many systems. You need to choose one!" forKey:NSLocalizedDescriptionKey];
                error = [NSError errorWithDomain:OEImportErrorDomainResolvable code:OEImportErrorCodeMultipleSystems userInfo:userInfo];
                [self stopImportForItem:item withError:error];
            }
            else
            {
                DLog(@"trying to get system with AVGGameSystemNameKey: %@", [archiveDict valueForKey:AVGGameSystemNameKey]);
                OEDBSystem *system = [OEDBSystem systemForArchiveName:[archiveDict valueForKey:AVGGameSystemNameKey]];
                if(system == nil)
                {
                    DLog(@"got an archive system we don't know");
                    [[game managedObjectContext] deleteObject:game];
                    
                    // TODO: localize user info in error
                    NSDictionary *userInfo = [NSDictionary dictionaryWithObject:@"Aaargh, too many systems. You need to choose one!" forKey:NSLocalizedDescriptionKey];
                    error = [NSError errorWithDomain:OEImportErrorDomainResolvable code:OEImportErrorCodeMultipleSystems userInfo:userInfo];
                    [self stopImportForItem:item withError:error];
                    return;
                }
                else
                {
                    [importInfo setValue:[NSArray arrayWithObject:[system systemIdentifier]] forKey:OEImportInfoSystemID];
                }
            }
        }
        
        // try again
        if([[importInfo valueForKey:OEImportInfoSystemID] count] == 1)
        {
            NSString *systemIdentifier = [[importInfo valueForKey:OEImportInfoSystemID] lastObject];
            OEDBSystem *system = [OEDBSystem systemForPluginIdentifier:systemIdentifier inDatabase:[self database]];
            if(system == nil)
            {
                //TODO: create proper error
                NSDictionary *userInfo = [NSDictionary dictionaryWithObject:@"No system again!" forKey:NSLocalizedDescriptionKey];
                error = [NSError errorWithDomain:OEImportErrorDomainFatal code:51 userInfo:userInfo];
                [self stopImportForItem:item withError:error];
            }
            else
            {
                NSURL *url = [rom URL];
                NSString *gameTitleWithSuffix = [url lastPathComponent];
                NSString *gameTitleWithoutSuffix = [gameTitleWithSuffix stringByDeletingPathExtension];
                game = [OEDBGame createGameWithName:gameTitleWithoutSuffix andSystem:system inDatabase:[self database]];
                if(archiveDict)
                    [game setArchiveVGInfo:archiveDict];
            }
        }
    }
    
    if(game != nil)
    {
        [rom setGame:game];
        NSAssert([[game mutableRoms] count] != 0, @"THIS IS BAD!!!");
        [self stopImportForItem:item withError:nil];
    }
}

- (void)scheduleItemForNextStep:(OEImportItem *)item
{
    item.importStep++;
    if([self status] == OEImporterStatusRunning)
    {
        dispatch_async(dispatchQueue, ^{
            importBlock(self, item);
        });
    }
    else
        self.activeImports--;
}

- (void)stopImportForItem:(OEImportItem *)item withError:(NSError *)error
{
    if([[error domain] isEqualTo:OEImportErrorDomainResolvable])
        [item setImportState:OEImportItemStatusResolvableError];
    else if(error == nil || [[error domain] isEqualTo:OEImportErrorDomainSuccess])
        [item setImportState:OEImportItemStatusFinished];
    else
        [item setImportState:OEImportItemStatusFatalError];
    
    [item setError:error];
    self.activeImports--;
    
    if(([item importState] == OEImportItemStatusFinished || [item importState] == OEImportItemStatusFatalError || [item importState] == OEImportItemStatusCancelled))
    {
        if([item completionHandler] != nil)
        {
            dispatch_async(dispatch_get_main_queue(), ^{
                [item completionHandler]();
            });
        }
        
        if([item error]) DLog(@"%@", [item error]);
        self.numberOfProcessedItems++;
        
        [self cleanupImportForItem:item];
    }
    
    [self startQueueIfNeeded];
    
    [self OE_performSelectorOnDelegate:@selector(romImporter:stoppedProcessingItem:) withObject:item];
}

- (void)cleanupImportForItem:(OEImportItem *)item
{
    NSError *error = [item error];
    if(error && [[error domain] isEqualTo:OEImportErrorDomainResolvable])
        return;
    
    if([item importState] == OEImportItemStatusFinished)
        [[self database] save:nil];
    
    NSString *md5 = [[item importInfo] valueForKey:OEImportInfoMD5];
    NSString *crc = [[item importInfo] valueForKey:OEImportInfoCRC];
    [[self unsavedMD5Hashes] removeObject:md5];
    [[self unsavedCRCHashes] removeObject:crc];

    [[self queue] removeObjectIdenticalTo:item];
}

#pragma mark - Importing Items with completion handler

- (BOOL)importItemAtPath:(NSString *)path withCompletionHandler:(OEImportItemCompletionBlock)handler
{
    NSURL *url = [NSURL fileURLWithPath:path];
    return [self importItemAtURL:url withCompletionHandler:handler];
}

- (BOOL)importItemsAtPaths:(NSArray *)paths withCompletionHandler:(OEImportItemCompletionBlock)handler
{
    __block BOOL success = NO;
    [paths enumerateObjectsUsingBlock:
     ^(id obj, NSUInteger idx, BOOL *stop)
     {
         success = [self importItemAtPath:obj withCompletionHandler:handler] || success;
     }];
    return success;
}

- (BOOL)importItemAtURL:(NSURL *)url withCompletionHandler:(OEImportItemCompletionBlock)handler
{
    id item = [[self queue] firstObjectMatchingBlock:
               ^ BOOL (id item)
               {
                   return [[item URL] isEqualTo:url];
               }];
    
    if(item == nil)
    {
        OEImportItem *item = [OEImportItem itemWithURL:url andCompletionHandler:handler];
        if(item)
        {
            [[self queue] addObject:item];
            self.totalNumberOfItems++;
            [self start];
            return YES;
        }
    }
    return NO;
}

- (BOOL)importItemsAtURLs:(NSArray *)urls withCompletionHandler:(OEImportItemCompletionBlock)handler
{
    __block BOOL success = NO;
    [urls enumerateObjectsUsingBlock:
     ^(id obj, NSUInteger idx, BOOL *stop)
     {
         success = [self importItemAtURL:obj withCompletionHandler:handler] || success;
     }];
    return success;
}

#pragma mark - Importing Items without completion handler
- (BOOL)importItemAtPath:(NSString *)path
{
    return [self importItemAtPath:path withCompletionHandler:nil];
}

- (BOOL)importItemsAtPaths:(NSArray *)paths
{
    return [self importItemsAtPaths:paths withCompletionHandler:nil];
}

- (BOOL)importItemAtURL:(NSURL *)url
{
    return [self importItemAtURL:url withCompletionHandler:nil];
}

- (BOOL)importItemsAtURLs:(NSArray *)urls
{
    return [self importItemsAtURLs:urls withCompletionHandler:nil];
}

#pragma mark - Spotlight importing -
- (void)discoverRoms:(NSArray *)volumes
{
    DLog();
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
    
    if(searchQuery != NULL)
    {
        // Limit Scope to selected volumes / URLs only
        MDQuerySetSearchScope(searchQuery, (__bridge CFArrayRef) volumes, 0);
        
        [[self spotlightSearchResults] removeAllObjects];
        
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(finalizeSearchResults:)
                                                     name:(NSString *)kMDQueryDidFinishNotification
                                                   object:(__bridge id)searchQuery];
        
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(updateSearchResults:)
                                                     name:(NSString *)kMDQueryProgressNotification
                                                   object:(__bridge id)searchQuery];
        
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(updateSearchResults:)
                                                     name:(NSString *)kMDQueryDidUpdateNotification
                                                   object:(__bridge id)searchQuery];
        
        if(MDQueryExecute(searchQuery, kMDQueryWantsUpdates))
            DLog(@"Searching for importable roms");
        else
        {
            CFRelease(searchQuery);
            searchQuery = nil;
            // leave this log message in...
            DLog(@"MDQuery failed to start.");
        }
        
    }
    else
        DLog(@"Invalid Search Query");
}

- (void)updateSearchResults:(NSNotification *)notification
{
    DLog();
    
    MDQueryRef searchQuery = (__bridge MDQueryRef)[notification object];
    
    
    // If you're going to have the same array for every iteration,
    // don't allocate it inside the loop !
    NSArray *excludedPaths = @[
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
                              ];
    
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
    DLog(@"Finished searching, found: %lu items", MDQueryGetResultCount(searchQuery));
    
    if(MDQueryGetResultCount(searchQuery))
    {
        [self importSpotlightResultsInBackground];
        
        MDQueryStop(searchQuery);
    }
    
    CFRelease(searchQuery);
}

- (void)importSpotlightResultsInBackground;
{
    DLog();
    [self importItemsAtPaths:[[self spotlightSearchResults] valueForKey:@"Path"]];
}

#pragma mark - Controlling Import -
- (void)start
{
    if([self status] == OEImporterStatusPaused || [self status] == OEImporterStatusStopped)
    {
        [self setStatus:OEImporterStatusRunning];
        [self startQueueIfNeeded];
        [self OE_performSelectorOnDelegate:@selector(romImporterDidStart:) withObject:self];
    }
}

- (void)pause
{
    if([self status] == OEImporterStatusRunning)
    {
        [self setStatus:OEImporterStatusPausing];
        dispatch_async(dispatchQueue, ^{
            [self setStatus:OEImporterStatusPaused];
            [self OE_performSelectorOnDelegate:@selector(romImporterDidPause:) withObject:self];
        });
    }
}

- (void)togglePause
{
    if([self status] == OEImporterStatusPaused)
        [self start];
    else if([self status] == OEImporterStatusRunning)
        [self pause];
}

- (void)cancel
{
    if([self status] == OEImporterStatusRunning)
    {
        [self setStatus:OEImporterStatusStopping];
        dispatch_async(dispatchQueue, ^{
            [self setStatus:OEImporterStatusStopped];
            
            [self setQueue:[NSMutableArray array]];
            [self setNumberOfProcessedItems:0];
            [self setTotalNumberOfItems:0];
            
            [self OE_performSelectorOnDelegate:@selector(romImporterDidCancel:) withObject:self];
        });
    }
}

- (void)removeFinished
{
    DLog(@"removeFinished");
    dispatch_async(dispatchQueue, ^{
        [[self queue] filterUsingPredicate:
         [NSPredicate predicateWithBlock:
          ^ BOOL (OEImportItem *evaluatedObject, NSDictionary *bindings)
          {
              return [evaluatedObject importState] != OEImportItemStatusFinished && [evaluatedObject importState] != OEImportItemStatusFatalError && [evaluatedObject importState] != OEImportItemStatusCancelled;
          }]];
    });
}

#pragma mark - Private Methods -
- (void)setTotalNumberOfItems:(NSInteger)totalNumberOfItems
{
    _totalNumberOfItems = totalNumberOfItems;
    [self OE_performSelectorOnDelegate:@selector(romImporterChangedItemCount:) withObject:nil];
}

- (void)OE_performSelectorOnDelegate:(SEL)selector withObject:(id)object
{
    if(![[self delegate] respondsToSelector:selector] ||
       ![[self delegate] respondsToSelector:@selector(performSelectorOnMainThread:withObject:waitUntilDone:)])
        return;
    
    NSAssert(protocol_getMethodDescription(@protocol(OEROMImporterDelegate), selector, NO, YES).name == selector,
             @"Unknown delegate method %@", NSStringFromSelector(selector));
    
    [(NSObject *)[self delegate] performSelectorOnMainThread:selector withObject:object waitUntilDone:NO];
}

@end
