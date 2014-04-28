/*
 Copyright (c) 2012, OpenEmu Team

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

#import "OEImportOperation.h"
#import "OESystemPlugin.h"

#import "OEROMImporter.h"
#import <MagicKit/MagicKit.h>
#import <XADMaster/XADArchive.h>

#import "NSFileManager+OEHashingAdditions.h"
#import "NSURL+OELibraryAdditions.h"

#import "OELibraryDatabase.h"
#import "OEDBGame.h"
#import "OEDBRom.h"
#import "OEDBCollection.h"

@interface OEImportOperation ()
- (void)OE_performImportStepCheckDirectory;
- (void)OE_performImportStepCheckArchiveFile;
- (void)OE_performImportStepHash;
- (void)OE_performImportStepCheckHash;
- (void)OE_performImportStepDetermineSystem;
- (void)OE_performImportStepOrganize;
- (void)OE_performImportStepOrganizeAdditionalFiles;
- (void)OE_performImportStepCreateCoreDataObjects;
@end

@interface OEImportOperation ()
@property (readonly, nonatomic) BOOL shouldExit;

@property (strong) NSString  *fileName;
@property (strong) NSURL     *extractedFileURL;
@property          NSInteger  archiveFileIndex;

@property (strong) NSString *md5Hash;
@property (strong) NSString *crcHash;

@property (strong) NSArray *systemIdentifiers;

@property (strong) OEDBRom *rom;
@end

@implementation OEImportOperation
#pragma mark - Creating Import Operations
+ (instancetype)operationWithURL:(NSURL*)url inImporter:(OEROMImporter*)importer
{
    NSError *error = nil;
    if(![url checkResourceIsReachableAndReturnError:&error])
    {
        IMPORTDLog(@"Could not reach url at %@", url);
        IMPORTDLog(@"%@", error);
        return nil;
    }

    // Ignore hidden or package files
    NSDictionary *resourceValues = [url resourceValuesForKeys:@[ NSURLIsPackageKey, NSURLIsHiddenKey ] error:nil];
    if([[resourceValues objectForKey:NSURLIsHiddenKey] boolValue] || [[resourceValues objectForKey:NSURLIsPackageKey] boolValue])
    {
        IMPORTDLog(@"Item is hidden file or package directory at %@", url);
        return nil;
    }

    // Check for .cg filters and copy them directly (not going through importer queue)
    if([self OE_isFilterAtURL:url])
        return nil;

    // Ignore text files that are .md
    if([self OE_isTextFileAtURL:url])
        return nil;

    if([self OE_isBiosFileAtURL:url])
        return nil;

    if([self OE_isInvalidExtensionAtURL:url])
        return nil;

    OEImportOperation *item = [[OEImportOperation alloc] init];
    [item setImporter:importer];
    [item setURL:url];
    [item setSourceURL:url];

    return item;
}

+ (BOOL)OE_isFilterAtURL:(NSURL*)url
{
    NSString *pathExtension = [[url pathExtension] lowercaseString];
    if([pathExtension isEqualToString:@"cg"])
    {
        IMPORTDLog(@"File seems to be a filter at %@", url);

        NSFileManager *fileManager = [NSFileManager defaultManager];
        NSError       *error       = nil;
        NSString      *cgFilename  = [url lastPathComponent];
        NSArray       *components  = @[[NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES) lastObject], @"OpenEmu", @"Filters"];
        NSURL         *filtersURL  = [NSURL fileURLWithPathComponents:components];
        NSURL         *destination = [NSURL URLWithString:cgFilename relativeToURL:filtersURL];


        if(![fileManager createDirectoryAtURL:filtersURL withIntermediateDirectories:YES attributes:nil error:&error])
        {
            IMPORTDLog(@"Could not create directory before copying filter at %@", url);
            IMPORTDLog(@"%@", error);
            error = nil;
        }

        if(![fileManager copyItemAtURL:url toURL:destination error:&error])
        {
            IMPORTDLog(@"Could not copy filter %@ to %@", url, destination);
            IMPORTDLog(@"%@", error);
        }

        return YES;
    }
    return NO;
}

+ (BOOL)OE_isTextFileAtURL:(NSURL*)url;
{
    NSString *pathExtension = [[url pathExtension] lowercaseString];
    if([pathExtension isEqualToString:@"md"] && [[[GEMagicKit magicForFileAtURL:url] uniformTypeHierarchy] containsObject:(id)kUTTypeText])
    {
        IMPORTDLog(@"Text file found at %@", url);
        return YES;
    }
    return NO;
}

+ (BOOL)OE_isBiosFileAtURL:(NSURL*)url
{
    // Copy known BIOS / System Files to BIOS folder
    for(NSDictionary *validFile in [OESystemPlugin requiredFiles])
    {
        NSString *biosSystemFileName = [validFile objectForKey:@"Name"];
        NSString *biosFilename       = [url lastPathComponent];
        NSError  *error              = nil;

        if([biosFilename caseInsensitiveCompare:biosSystemFileName] == NSOrderedSame)
        {
            IMPORTDLog(@"File seems to be a filter at %@", url);

            NSFileManager *fileManager = [NSFileManager defaultManager];
            NSArray       *components  = @[[NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES) lastObject], @"OpenEmu", @"BIOS"];
            NSURL         *biosURL     = [NSURL fileURLWithPathComponents:components];
            NSURL         *destination = [NSURL URLWithString:biosFilename relativeToURL:biosURL];

            if(![fileManager createDirectoryAtURL:biosURL withIntermediateDirectories:YES attributes:nil error:&error])
            {
                IMPORTDLog(@"Could not create directory before copying filter at %@", url);
                IMPORTDLog(@"%@", error);
                error = nil;
            }

            if(![fileManager copyItemAtURL:url toURL:destination error:&error])
            {
                IMPORTDLog(@"Could not copy bios file %@ to %@", url, destination);
                IMPORTDLog(@"%@", error);
            }

            return YES;
        }
    }
    return NO;
}

+ (BOOL)OE_isInvalidExtensionAtURL:(NSURL *)url
{
    NSString *pathExtension = [[url pathExtension] lowercaseString];

    // Ignore unsupported file extensions
    NSMutableSet *validExtensions = [NSMutableSet setWithArray:[OESystemPlugin supportedTypeExtensions]];

    // TODO:
    // The Archived Game document type lists all supported archive extensions, e.g. zip
    NSDictionary *bundleInfo      = [[NSBundle mainBundle] infoDictionary];
    NSArray      *docTypes        = [bundleInfo objectForKey:@"CFBundleDocumentTypes"];
    for(NSDictionary *docType in docTypes)
    {
        if([[docType objectForKey:@"CFBundleTypeName"] isEqualToString:@"Archived Game"])
        {
            [validExtensions addObjectsFromArray:[docType objectForKey:@"CFBundleTypeExtensions"]];
            break;
        }
    }

    if(![url isDirectory])
    {
        if([pathExtension length] > 0 && ![validExtensions containsObject:pathExtension])
        {
            IMPORTDLog(@"File has unsupported extension (%@) at %@", pathExtension, url);
            return YES;
        }
    }

    return NO;
}

#pragma mark - NSCoding Protocol
- (instancetype)init
{
    self = [super init];
    if (self)
    {
        _shouldExit = NO;
        _archiveFileIndex = NSNotFound;
        _exploreArchives  = YES;
    }
    return self;
}

- (instancetype)initWithCoder:(NSCoder *)decoder
{
    self = [self init];
    if (self)
    {
        [self setURL:[decoder decodeObjectForKey:@"URL"]];
        [self setSourceURL:[decoder decodeObjectForKey:@"sourceURL"]];
        [self setImportState:OEImportItemStatusIdle];
    }
    return self;
}

- (void)encodeWithCoder:(NSCoder *)encoder
{
    [encoder encodeObject:[self URL] forKey:@"URL"];
    [encoder encodeObject:[self sourceURL] forKey:@"sourceURL"];
}
#pragma mark -
- (void)setURL:(NSURL *)URL
{
    if([self archive])
        [[self archive] setURL:URL];
    else
        _url = URL;
}

- (NSURL*)URL
{
    if([self archive])
        return [[self archive] URL];
    return _url;
}
@synthesize URL=_url;
#pragma mark -
- (void)exitWithStatus:(OEImportItemState)status error:(NSError*)error
{
    DLog(@"%@ | %@", [[self URL] lastPathComponent], [[self extractedFileURL] lastPathComponent]);
    DLog(@"Success: %s", BOOL_STR(status == OEImportItemStatusFinished));
    _shouldExit = YES;
    [self setError:error];
    [self setImportState:status];
}

- (BOOL)shouldExit
{
    return _shouldExit || [self isCancelled];
}
@synthesize shouldExit=_shouldExit;
#pragma mark - NSOperation Overrides
- (void)main
{
    if([self shouldExit]) return;

    [self OE_performImportStepCheckDirectory];
    if([self shouldExit]) return;

    [self OE_performImportStepCheckArchiveFile];
    if([self shouldExit]) return;

    [self OE_performImportStepDetermineSystem];
    if([self shouldExit]) return;

    [self OE_performImportStepHash];
    if([self shouldExit]) return;

    [self OE_performImportStepCheckHash];
    if([self shouldExit]) return;

    [self OE_performImportStepOrganize];
    if([self shouldExit]) return;

    [self OE_performImportStepOrganizeAdditionalFiles];
    if([self shouldExit]) return;

    [self OE_performImportStepCreateCoreDataObjects];
}
#pragma mark - Importing
- (void)OE_performImportStepCheckDirectory
{
    // Check if file at URL is a directory
    // if so, add new items for each item in the directory
    NSURL *url = [self URL];
    if([url isDirectory])
    {
        NSArray *propertyKeys = @[NSURLIsPackageKey, NSURLIsHiddenKey];
        NSUInteger    options = NSDirectoryEnumerationSkipsSubdirectoryDescendants|NSDirectoryEnumerationSkipsPackageDescendants|NSDirectoryEnumerationSkipsHiddenFiles;

        NSDirectoryEnumerator *directoryEnumerator = [[NSFileManager defaultManager] enumeratorAtURL:url includingPropertiesForKeys:propertyKeys options:options errorHandler:NULL];

        NSURL *subURL = nil;
        while([self isCancelled] == NO && (subURL = [directoryEnumerator nextObject]))
        {
            OEROMImporter    *importer = [self importer];
            OEImportOperation *subItem = [OEImportOperation operationWithURL:subURL inImporter:importer];
            if(subItem)
            {
                [subItem setCompletionHandler:[self completionHandler]];
                [subItem setCollectionID:[self collectionID]];
                [importer addOperation:subItem];
            }
        }

        [self exitWithStatus:OEImportItemStatusFinished error:nil];
    }
}
- (void)OE_performImportStepCheckArchiveFile
{
    if([self exploreArchives] == NO) return;
    IMPORTDLog();
    NSURL *url = [self URL];
    NSString *path = [url path];
    NSString *extension = [[path pathExtension] lowercaseString];

    if([extension isEqualToString:@"nds"])
        return;

    XADArchive *archive = nil;
    @try
    {
        archive = [XADArchive archiveForFile:path];
    }
    @catch(NSException *e)
    {
        archive = nil;
    }

    if(archive != nil)
    {
        BOOL hasSubItem = NO;
        OEROMImporter *importer = [self importer];
        NSString    *formatName = [archive formatName];
        OEImportOperation *duplicateItem = [OEImportOperation operationWithURL:[self URL] inImporter:importer];

        if ([formatName isEqualToString:@"MacBinary"])
            return;

        for(int i=0; i<[archive numberOfEntries]; i++)
        {
            if(![archive entryHasSize:i] || [archive entryIsEncrypted:i] || [archive entryIsDirectory:i] || [archive entryIsArchive:i])
            {
                DLog(@"Entry %d is either empty, or a directory or encrypted or iteself an archive", i);
                continue;
            }

            NSString *folder = temporaryDirectoryForDecompressionOfPath(path);
            NSString *name   = [archive nameOfEntry:i];
            if ([[name pathExtension] length] == 0 && [[path pathExtension] length] > 0) {
                // this won't do. Re-add the archive's extension in case it's .smc or the like
                name = [name stringByAppendingPathExtension:[path pathExtension]];
            }

            [self setFileName:name];
            NSString *extractionPath = [folder stringByAppendingPathComponent:name];

            BOOL isdir;
            NSURL *tmpURL = [NSURL fileURLWithPath:extractionPath];
            NSFileManager *fm = [NSFileManager defaultManager];
            if (![fm fileExistsAtPath:extractionPath isDirectory:&isdir]) {
                @try
                {
                    [archive _extractEntry:i as:extractionPath deferDirectories:YES dataFork:YES resourceFork:NO];
                }
                @catch (NSException *exception) {
                    [fm removeItemAtPath:folder error:nil];
                    tmpURL = nil;
                    DLog(@"unpack failed");
                }
            }

            if(tmpURL)
            {
                OEImportOperation *subItem = [OEImportOperation operationWithURL:tmpURL inImporter:importer];
                if(subItem)
                {
                    [subItem setArchive:duplicateItem];
                    [subItem setURL:[self URL]];
                    [subItem setExtractedFileURL:tmpURL];
                    [subItem setArchiveFileIndex:i];
                    [subItem setExploreArchives:NO];
                    [subItem setCollectionID:[self collectionID]];

                    // TODO: insert operation at front of queue
                    [importer addOperation:subItem];
                    hasSubItem = YES;
                }
            }
        }

        if(hasSubItem)
        {
            [duplicateItem setExploreArchives:NO];
            [duplicateItem setCollectionID:[self collectionID]];

            // TODO: insert operation at front of queue (after other subitems)
            [importer addOperation:duplicateItem];
            [duplicateItem setCollectionID:[self collectionID]];
            [self exitWithStatus:OEImportItemStatusFinished error:nil];
        }
    }
}

- (void)OE_performImportStepHash
{
    IMPORTDLog();
    NSURL         *url = [self extractedFileURL] ?: [self URL];
    NSString      *md5, *crc;
    NSError       *error = nil;
    NSFileManager *fileManager = [NSFileManager defaultManager];

    if(![fileManager hashFileAtURL:url md5:&md5 crc32:&crc error:&error])
    {
        DLog(@"unable to hash file, this is probably a fatal error");
        DLog(@"%@", error);

        NSError *error = [NSError errorWithDomain:OEImportErrorDomainFatal code:OEImportErrorCodeNoHash userInfo:nil];
        [self exitWithStatus:OEImportItemStatusFatalError error:error];
    }
    else
    {
        [self setMd5Hash:md5];
        [self setCrcHash:crc];
    }

}

- (void)OE_performImportStepCheckHash
{
    IMPORTDLog();
    NSError  *error = nil;
    NSString *md5   = [self md5Hash];
    NSString *crc   = [self crcHash];
    NSManagedObjectContext *context = [[self importer] context];

    OEDBRom *rom = nil;

    if(rom == nil)
        rom = [OEDBRom romWithMD5HashString:md5 inContext:context error:&error];
    if(rom == nil)
        rom = [OEDBRom romWithCRC32HashString:crc inContext:context error:&error];

    if(rom != nil)
    {
        NSURL *romURL = [rom URL];
        [self setRom:rom];
        if(![romURL checkResourceIsReachableAndReturnError:&error])
        {
            DLog(@"rom file not available");
            DLog(@"%@", error);
            // TODO: depending on error finish here with 'already present' success
            // if the error says something like volume could not be found we might want to skip import because the file is probably on an external HD that is currently not connected
            // but if it just says the file was deleted we should replace the rom's url with the new one and continue importing
        }
        else
        {
            [self exitWithStatus:OEImportItemStatusFinished error:nil];
        }
    }
}

- (void)OE_performImportStepDetermineSystem
{
    IMPORTDLog(@"URL: %@", [self sourceURL]);
    if([self rom] != nil) return;

    NSError *error = nil;
    OEROMImporter *importer = [self importer];
    NSManagedObjectContext *context = [importer context];
    NSURL *url = [self extractedFileURL] ?: [self URL];

    NSArray *validSystems = [OEDBSystem systemsForFileWithURL:url inContext:context error:&error];
    if(validSystems == nil)
    {
        DLog(@"Could not get valid systems");
        DLog(@"%@", error);

        error = [NSError errorWithDomain:OEImportErrorDomainFatal code:OEImportErrorCodeNoSystem userInfo:nil];
        [self exitWithStatus:OEImportItemStatusFatalError error:error];
        return;
    }
    else if([validSystems count] == 0)
    {
        // Try again with the zip itself
        DLog(@"No valid system found for item at url %@", url);

        error = [NSError errorWithDomain:OEImportErrorDomainFatal code:OEImportErrorCodeNoSystem userInfo:nil];
        [self exitWithStatus:OEImportItemStatusFatalError error:error];
        return;
    }
    else if([validSystems count] == 1)
    {
        [self setSystemIdentifiers:@[[[validSystems lastObject] systemIdentifier]]];
    }
    else // Found multiple valid systems after checking extension and system specific canHandleFile:
    {
        NSMutableArray *systemIDs = [NSMutableArray arrayWithCapacity:[validSystems count]];
        [validSystems enumerateObjectsUsingBlock:^(OEDBSystem *system, NSUInteger idx, BOOL *stop){
            NSString *systemIdentifier = [system systemIdentifier];
            [systemIDs addObject:systemIdentifier];
        }];
        [self setSystemIdentifiers:systemIDs];

        NSDictionary *userInfo = [NSDictionary dictionaryWithObject:@"Aaargh, too many systems. You need to choose one!" forKey:NSLocalizedDescriptionKey];
        error = [NSError errorWithDomain:OEImportErrorDomainResolvable code:OEImportErrorCodeMultipleSystems userInfo:userInfo];
        [self exitWithStatus:OEImportItemStatusResolvableError error:error];
    }
}

- (void)OE_performImportStepOrganize
{
    NSUserDefaults *standardUserDefaults = [NSUserDefaults standardUserDefaults];
    BOOL copyToLibrary   = [standardUserDefaults boolForKey:OECopyToLibraryKey];
    BOOL organizeLibrary = [standardUserDefaults boolForKey:OEOrganizeLibraryKey];

    NSError *error       = nil;
    NSURL   *url         = [self URL];

    NSManagedObjectContext *context = [[self importer] context];

    // Unlock rom file so we can rename the copy directly
    BOOL romFileLocked = NO;
    if([[[[NSFileManager defaultManager] attributesOfItemAtPath:[url path] error:&error] objectForKey:NSFileImmutable] boolValue])
    {
        romFileLocked = YES;
        [[NSFileManager defaultManager] setAttributes:@{ NSFileImmutable: @(FALSE) } ofItemAtPath:[url path] error:nil];
    }

    // Copy to library folder if it's not already there
    OELibraryDatabase *database = [[self importer] database];

    if(copyToLibrary && ![url isSubpathOfURL:[database romsFolderURL]])
    {
        NSString *fullName  = [url lastPathComponent];
        NSString *extension = [fullName pathExtension];
        NSString *baseName  = [fullName stringByDeletingPathExtension];

        NSURL *unsortedFolder = [database unsortedRomsFolderURL];
        NSURL *romURL         = [unsortedFolder URLByAppendingPathComponent:fullName];
        romURL = [romURL uniqueURLUsingBlock:^NSURL *(NSInteger triesCount) {
            NSString *newName = [NSString stringWithFormat:@"%@ %ld.%@", baseName, triesCount, extension];
            return [unsortedFolder URLByAppendingPathComponent:newName];
        }];

        if([[NSFileManager defaultManager] copyItemAtURL:url toURL:romURL error:&error])
            // Lock original file again
            if(romFileLocked)
                [[NSFileManager defaultManager] setAttributes:@{ NSFileImmutable: @(YES) } ofItemAtPath:[url path] error:&error];

        if(error != nil)
        {
            return;
        }

        url = [romURL copy];
        [self setURL:url];
    }

    // Move items in library folder to system sub-folder
    if(organizeLibrary && [url isSubpathOfURL:[database romsFolderURL]])
    {
        // unlock file so we can move and rename it
        if([[[NSFileManager defaultManager] attributesOfItemAtPath:[url path] error:&error] objectForKey:NSFileImmutable])
            [[NSFileManager defaultManager] setAttributes:@{ NSFileImmutable: @(FALSE) } ofItemAtPath:[url path] error:nil];

        __block OEDBSystem *system = nil;
        if([self rom] != nil)
        {
            DLog(@"using rom object");
            system = [[[self rom] game] system];
        }
        else
        {
            NSAssert([[self systemIdentifiers] count] == 1, @"System should have been detected at an earlier import stage");
            NSString *systemIdentifier = [[self systemIdentifiers] lastObject];
            system = [OEDBSystem systemForPluginIdentifier:systemIdentifier inContext:context];
        }

        NSString *fullName  = [url lastPathComponent];
        NSString *extension = [fullName pathExtension];
        NSString *baseName  = [fullName stringByDeletingPathExtension];

        NSURL *systemFolder = [database romsFolderURLForSystem:system];
        NSURL *romURL       = [systemFolder URLByAppendingPathComponent:fullName];

        if([romURL isEqualTo:url])
        {
            [self setURL:romURL];
            return;
        }

        romURL = [romURL uniqueURLUsingBlock:^NSURL*(NSInteger triesCount) {
            NSString *newName = [NSString stringWithFormat:@"%@ %ld.%@", baseName, triesCount, extension];
            return [systemFolder URLByAppendingPathComponent:newName];
        }];

        NSError *error = nil;
        if(![[NSFileManager defaultManager] moveItemAtURL:url toURL:romURL error:&error])
        {
            [self exitWithStatus:OEImportItemStatusFatalError error:error];
        }
        else
        {
            [self setURL:romURL];
        }
    }
}

- (void)OE_performImportStepOrganizeAdditionalFiles
{
    IMPORTDLog();
    OELibraryDatabase *database = [[self importer] database];
    NSURL    *url  = [self URL];
    NSString *path = [url path];

    NSUserDefaults *standardUserDefaults = [NSUserDefaults standardUserDefaults];
    BOOL copyToLibrary   = [standardUserDefaults boolForKey:OECopyToLibraryKey];
    BOOL organizeLibrary = [standardUserDefaults boolForKey:OEOrganizeLibraryKey];
    if((copyToLibrary || organizeLibrary) && [[[url pathExtension] lastPathComponent] isEqualToString:@"cue"])
    {
        NSString *referencedFilesDirectory = [path stringByDeletingLastPathComponent];
        OECUESheet *cue = [[OECUESheet alloc] initWithPath:path andReferencedFilesDirectory:referencedFilesDirectory];
        if(cue == nil)
        {
            // TODO: Create user info
            NSError *error = [NSError errorWithDomain:OEImportErrorDomainFatal code:OEImportErrorCodeInvalidFile userInfo:nil];
            [self exitWithStatus:OEImportItemStatusFatalError error:error];
            return;
        }

        if(![cue allFilesAvailable])
        {
            // TODO: Create user info
            NSError *error = [NSError errorWithDomain:OEImportErrorDomainFatal code:OEImportErrorCodeAdditionalFiles userInfo:nil];
            [self exitWithStatus:OEImportItemStatusFatalError error:error];
            return;
        }

        NSURL *sourceURL = [self sourceURL];
        NSString *targetDirectory = [path stringByDeletingLastPathComponent];
        if(copyToLibrary && ![sourceURL isSubpathOfURL:[database romsFolderURL]])
        {
            DLog(@"copy to '%@'", targetDirectory);
            NSError *error = nil;
            if(![cue copyReferencedFilesToPath:targetDirectory withError:&error])
            {
                DLog(@"%@", error);
                [self exitWithStatus:OEImportItemStatusFatalError error:error];
                return;
            }
        }
        else if(organizeLibrary && [sourceURL isSubpathOfURL:[database romsFolderURL]])
        {
            DLog(@"move to '%@'", targetDirectory);
            NSError *error = nil;
            if(![cue moveReferencedFilesToPath:targetDirectory withError:&error])
            {
                DLog(@"%@", error);
                [self exitWithStatus:OEImportItemStatusFatalError error:error];
                return;
            }
        }
    }

}

- (void)OE_performImportStepCreateCoreDataObjects
{
    IMPORTDLog();

    NSManagedObjectContext *context = [[self importer] context];
    NSError *error = nil;
    OEDBRom *rom = [self rom];

    if(rom == nil)
    {
        rom = [OEDBRom createObjectInContext:context];
    }

    if(rom == nil)
    {
        NSError *error = [NSError errorWithDomain:OEImportErrorDomainFatal code:OEImportErrorCodeInvalidFile userInfo:nil];
        [self exitWithStatus:OEImportItemStatusFatalError error:error];
        return;
    }

    NSString *md5 = [self md5Hash];
    NSString *crc = [self crcHash];

    [rom setURL:[self URL]];
    if([self fileName] != nil) [rom setFileName:[self fileName]];
    if(crc != nil) [rom setCrc32:crc];
    if(md5 != nil) [rom setMd5:md5];

    // Check if system plugin for ROM implemented headerLookupForFile: and serialLookupForFile:
    NSAssert([[self systemIdentifiers] count] == 1, @"System should have been detected at an earlier import stage");
    NSString *systemIdentifier = [[self systemIdentifiers] lastObject];

    NSURL *lookupURL = [self extractedFileURL] ?: [rom URL];
    NSString *headerFound = [OEDBSystem headerForFileWithURL:lookupURL forSystem:systemIdentifier];
    NSString *serialFound = [OEDBSystem serialForFileWithURL:lookupURL forSystem:systemIdentifier];

    if(headerFound != nil)
    {
        [rom setHeader:headerFound];
    }

    if(serialFound != nil)
    {
        [rom setSerial:serialFound];
    }

    if([self archiveFileIndex] != NSNotFound)
        [rom setArchiveFileIndex:@([self archiveFileIndex])];

    OEDBGame *game = [rom game];
    if(game == nil)
    {
        NSString *systemIdentifier = [[self systemIdentifiers] lastObject];
        OEDBSystem *system = [OEDBSystem systemForPluginIdentifier:systemIdentifier inContext:context];
        if(system != nil)
        {
            NSURL *url = [self extractedFileURL] ?: [rom URL];
            NSString *gameTitleWithSuffix = [url lastPathComponent];
            NSString *gameTitleWithoutSuffix = [gameTitleWithSuffix stringByDeletingPathExtension];

            game = [OEDBGame createObjectInContext:context];
            [game setName:gameTitleWithoutSuffix];
            [game setSystem:system];
        }
        else
        {
            NSDictionary *userInfo = [NSDictionary dictionaryWithObject:@"No system! Someone must have deleted or disabled it!" forKey:NSLocalizedDescriptionKey];
            error = [NSError errorWithDomain:OEImportErrorDomainFatal code:OEImportErrorCodeNoSystem userInfo:userInfo];
            [self exitWithStatus:OEImportItemStatusFatalError error:error];
            return;
        }
    }
    
    if(game != nil)
    {
        [rom setGame:game];

        if([[NSUserDefaults standardUserDefaults] boolForKey:OEAutomaticallyGetInfoKey])
        {
            [game setStatus:@(OEDBGameStatusProcessing)];
        }

        if([self collectionID])
        {
            OEDBCollection *collection = [OEDBCollection objectWithID:[self collectionID] inContext:context];
            if(collection != nil && [collection isDeleted] == NO)
            {
                [[game mutableCollections] addObject:collection];
                DLog(@"add to collection: %@", collection);
            }
        }

        [rom save];

        [self exitWithStatus:OEImportItemStatusFinished error:nil];
    }
    else
    {
        [rom delete];
        [game delete];
        [rom save];

        NSError *error = [NSError errorWithDomain:OEImportErrorDomainFatal code:OEImportErrorCodeNoGame userInfo:nil];
        [self exitWithStatus:OEImportItemStatusFatalError error:error];
    }
}
@end
