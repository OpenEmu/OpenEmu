/*
 Copyright (c) 2020, OpenEmu Team

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
@import OpenEmuKit;

#import "OEROMImporter.h"
#import <XADMaster/XADArchive.h>

#import "NSFileManager+OEHashingAdditions.h"

#import "OELibraryDatabase.h"
#import "OEDBGame.h"
#import "OEDBRom.h"
#import "OEDBCollection.h"
#import "OEDBSaveState.h"
#import "OEDBSystem+CoreDataProperties.h"

#import "OELogging.h"

#import "OpenEmu-Swift.h"

NS_ASSUME_NONNULL_BEGIN

NSString * const OEImportManualSystems = @"OEImportManualSystems";

@interface OEImportOperation ()

@property (readonly, nonatomic) BOOL shouldExit;
@property (nullable) OEFile *file;

@property NSString *fileName;
@property NSInteger archiveFileIndex;

@property (copy) NSString *md5Hash;

@property (nullable) OEDBRom *rom;

@end

@implementation OEImportOperation
@synthesize checked;

#pragma mark - Creating Import Operations

+ (nullable instancetype)operationWithURL:(NSURL*)url inImporter:(OEROMImporter*)importer
{
    NSError *error = nil;
    if(![url checkResourceIsReachableAndReturnError:&error])
    {
        DLog(@"Could not reach url at %@", url);
        DLog(@"%@", error);
        return nil;
    }

    // Ignore hidden or package files
    NSDictionary *resourceValues = [url resourceValuesForKeys:@[ NSURLIsPackageKey, NSURLIsHiddenKey ] error:nil];
    if([[resourceValues objectForKey:NSURLIsHiddenKey] boolValue] || [[resourceValues objectForKey:NSURLIsPackageKey] boolValue])
    {
        DLog(@"Item is hidden file or package directory at %@", url);
        // Check for .oesavestate files and copy them directly (not going through importer queue)
        [self OE_tryImportSaveStateAtURL:url];

        return nil;
    }

    // Import .oeshaderplugin
    if ([self OE_isShaderAtURL:url]) {
        return nil;
    }

    // Check for PlayStation .sbi subchannel data files and copy them directly (not going through importer queue)
    if([self OE_isSBIFileAtURL:url])
        return nil;

    // Ignore text files that are .md
    if([self OE_isTextFileAtURL:url])
        return nil;

    if([self OE_isInvalidExtensionAtURL:url])
        return nil;

    OEImportOperation *item = [[OEImportOperation alloc] init];
    item.importer = importer;
    item.URL = url;
    item.sourceURL = url;

    return item;
}

+ (BOOL)OE_isShaderAtURL:(NSURL *)url
{
    NSString *pathExtension = url.pathExtension.lowercaseString;
    if([pathExtension isEqualToString:@"oeshaderplugin"])
    {
        os_log_info(OE_LOG_IMPORT, "File seems to be a shader plugin at %{public}@", url.path);
        
        NSFileManager *fileManager = [NSFileManager defaultManager];
        NSError       *error       = nil;
        NSString      *filename    = url.lastPathComponent.stringByDeletingPathExtension;
        NSString      *shadersPath = [NSString pathWithComponents:@[NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES).lastObject, @"OpenEmu", @"Shaders"]];
        NSString      *destination = [shadersPath stringByAppendingPathComponent:filename];
        
        if ([OEShadersModel.shared.systemShaderNames containsObject:filename]) {
            // ignore customer shaders with the same name
            os_log_error(OE_LOG_IMPORT, "Custom shader name '%{public}@' collides with system shader", filename);
            return NO;
        }
        
        if ([fileManager fileExistsAtPath:destination]) {
            // lets remove it first
            if (![fileManager removeItemAtPath:destination error:&error])
            {
                os_log_error(OE_LOG_IMPORT, "Could not remove existing directory '%{public}@' before copying shader: %{public}@", destination, error.localizedDescription);
                return NO;
            }
        }
        
        if(![fileManager createDirectoryAtURL:[NSURL fileURLWithPath:destination] withIntermediateDirectories:YES attributes:nil error:&error])
        {
            os_log_error(OE_LOG_IMPORT, "Could not create directory '%{public}@' before copying shader: %{public}@", destination, error.localizedDescription);
            return NO;
        }
        
        @try {
            XADArchive *archive = [XADArchive archiveForFile:url.path];
            [archive extractTo:destination];
            [OEShadersModel.shared reload];
        } @catch (NSException *e) {
            os_log_error(OE_LOG_IMPORT, "Error extracting shader plugin: %{public}@", e.reason);
            return NO;
        }
        
        return YES;
    }
    return NO;
    
}

+ (BOOL)OE_isSBIFileAtURL:(NSURL*)url
{
    NSUserDefaults *standardUserDefaults = NSUserDefaults.standardUserDefaults;
    BOOL copyToLibrary = [standardUserDefaults boolForKey:OECopyToLibraryKey];

    NSString *pathExtension = url.pathExtension.lowercaseString;
    if([pathExtension isEqualToString:@"sbi"] && copyToLibrary)
    {
        // Check 4-byte SBI header
        NSFileHandle *header = [NSFileHandle fileHandleForReadingFromURL:url error:nil];
        [header seekToFileOffset: 0x0];
        NSData *sbiHeaderBuffer = [header readDataOfLength: 4];
        NSData *expectedSBIHeader = [NSData dataWithBytes: "SBI\0" length: 4];
        [header closeFile];
        BOOL bytesFound = [sbiHeaderBuffer isEqualToData:expectedSBIHeader];

        if(!bytesFound)
            return NO;

        DLog(@"File seems to be a SBI file at %@", url);

        NSFileManager *fileManager = NSFileManager.defaultManager;
        NSError       *error       = nil;
        NSString      *sbiFilename = url.lastPathComponent;
        NSString      *sbiExtensionlessFilename = sbiFilename.stringByDeletingPathExtension;
        // RegEx pattern match the parentheses e.g. " (Disc 1)" and trim sbiExtensionlessFilename string.
        NSString      *sbiFolderName = [sbiExtensionlessFilename stringByReplacingOccurrencesOfString:@"\\ \\(Disc.*\\)" withString:@"" options:NSRegularExpressionSearch range:NSMakeRange(0, sbiExtensionlessFilename.length)];
        // Get the PlayStation system's Game Library folder URL.
        OELibraryDatabase      *database = OELibraryDatabase.defaultDatabase;
        NSManagedObjectContext *context  = database.mainThreadContext;
        OEDBSystem *system = [OEDBSystem systemForPluginIdentifier:@"openemu.system.psx" inContext:context];
        NSURL *systemFolderURL = [database romsFolderURLForSystem:system];
        // Copy SBI to subfolder where (presumably) the user also imported their game files - SBI files must be in the same folder as the game.
        // NOTE: This is for convenience and cannot be perfect now that games for disc-supporting systems are copied into a unique subfolder in the system's folder - users may have to manually import.
        // Does not handle SBI-requiring M3U games well (FF VIII, FF IX, Galerians and Parasite Eve II), unless the M3U is named as the basename SBI (e.g. Final Fantasy IX (Europe).m3u and Final Fantasy IX (Europe) (Disc 1).sbi).
        NSURL *sbiSubFolderURL = [systemFolderURL URLByAppendingPathComponent:sbiFolderName isDirectory:YES];
        NSURL *destination = [sbiSubFolderURL URLByAppendingPathComponent:sbiFilename];

        if(![fileManager createDirectoryAtURL:sbiSubFolderURL withIntermediateDirectories:YES attributes:nil error:&error])
        {
            DLog(@"Could not create directory before copying SBI file at %@", url);
            DLog(@"%@", error);
            error = nil;
        }

        if(![fileManager copyItemAtURL:url toURL:destination error:&error])
        {
            DLog(@"Could not copy SBI file %@ to %@", url, destination);
            DLog(@"%@", error);
        }

        return YES;
    }
    return NO;
}

+ (void)OE_tryImportSaveStateAtURL:(NSURL*)url
{
    NSString *pathExtension = url.pathExtension.lowercaseString;

    if([pathExtension isEqualToString:OESaveStateSuffix])
    {
        OELibraryDatabase *db = [OELibraryDatabase defaultDatabase];
        [OEDBSaveState createSaveStateByImportingBundleURL:url intoContext:db.mainThreadContext copy:YES];
    }
}

+ (BOOL)OE_isTextFileAtURL:(NSURL*)url;
{
    NSString *pathExtension = url.pathExtension.lowercaseString;

    if([pathExtension isEqualToString:@"md"])
    {
        // Read 1k of the file an looks for null bytes
        const int sampleSize = 1024;
        char sampleBuffer[sampleSize];
        const char *path = url.path.fileSystemRepresentation;
        FILE *f = fopen(path, "r");
        if(f)
        {
            size_t bytesRead = fread(sampleBuffer, sizeof(char), sampleSize, f);
            fclose(f);
            if(memchr(sampleBuffer, '\0', bytesRead) == NULL)
                return YES;
        }
    }
    return NO;
}

+ (BOOL)OE_isInvalidExtensionAtURL:(NSURL *)url
{
    NSString *pathExtension = url.pathExtension.lowercaseString;

    // Ignore unsupported file extensions
    NSMutableSet *validExtensions = [NSMutableSet setWithArray:[OESystemPlugin supportedTypeExtensions]];

    // Hack fix for #2031
    // TODO: Build set for extensions from all BIOS file types?
    [validExtensions addObject:@"img"];

    // TODO:
    // The Archived Game document type lists all supported archive extensions, e.g. zip
    NSDictionary *bundleInfo      = [NSBundle mainBundle].infoDictionary;
    NSArray      *docTypes        = bundleInfo[@"CFBundleDocumentTypes"];
    for(NSDictionary *docType in docTypes)
    {
        if([docType[@"CFBundleTypeName"] isEqualToString:@"Archived Game"])
        {
            [validExtensions addObjectsFromArray:docType[@"CFBundleTypeExtensions"]];
            break;
        }
    }

    if(!url.isDirectory)
    {
        if(pathExtension.length > 0 && ![validExtensions containsObject:pathExtension])
        {
            DLog(@"File has unsupported extension (%@) at %@", pathExtension, url);
            return YES;
        }
    }

    return NO;
}

- (instancetype)init
{
    self = [super init];
    if (self)
    {
        _shouldExit = NO;
        _archiveFileIndex = NSNotFound;
        _exploreArchives  = YES;
        _isDisallowedArchiveWithMultipleFiles = NO;
    }
    return self;
}

#pragma mark - NSSecureCoding Protocol

- (nullable instancetype)initWithCoder:(NSCoder *)decoder
{
    self = [self init];
    if (self)
    {
        self.URL = [decoder decodeObjectOfClass:NSURL.class forKey:@"URL"];
        self.sourceURL = [decoder decodeObjectOfClass:NSURL.class forKey:@"sourceURL"];
        self.exitStatus = OEImportExitNone;
    }
    return self;
}

- (void)encodeWithCoder:(NSCoder *)encoder
{
    [encoder encodeObject:self.URL forKey:@"URL"];
    [encoder encodeObject:self.sourceURL forKey:@"sourceURL"];
}

+ (BOOL)supportsSecureCoding
{
    return YES;
}

- (instancetype)copyWithZone:(nullable NSZone *)zone
{
    OEImportOperation *copy = [[OEImportOperation allocWithZone:zone] init];
    copy.exitStatus = OEImportExitNone;
    copy.error = nil;

    copy.exploreArchives = self.exploreArchives;
    copy.URL = self.URL;
    copy.sourceURL = self.sourceURL;
    copy.collectionID = self.collectionID;
    copy.rom = self.rom;

    copy.systemIdentifiers = self.systemIdentifiers;
    copy.completionHandler = self.completionHandler;
    copy.importer = self.importer;

    copy.checked = self.checked;

    copy.fileName = self.fileName;
    copy.extractedFileURL = self.extractedFileURL;
    copy.archiveFileIndex = self.archiveFileIndex;
    copy.md5Hash = self.md5Hash;
    copy.romLocation = self.romLocation;

    copy.isDisallowedArchiveWithMultipleFiles = self.isDisallowedArchiveWithMultipleFiles;

    return copy;
}

#pragma mark -

- (void)exitWithStatus:(OEImportExitStatus)status error:(nullable NSError *)error
{
    NSManagedObjectContext *context = self.importer.context;

    if(status == OEImportExitSuccess)
    {
        OEDBRom *rom = self.rom;
        if(rom != nil)
        {
            if(self.collectionID != nil && rom.game != nil)
            {
                OEDBCollection *collection = [OEDBCollection objectWithID:self.collectionID inContext:context];
                if(collection != nil && collection.isDeleted == NO)
                {
                    [rom.game.mutableCollections addObject:collection];
                }
            }

            // start sync thread
            if(rom.game.status.intValue == OEDBGameStatusProcessing)
            {
                OELibraryDatabase *database = self.importer.database;
                dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(1.0 * NSEC_PER_SEC)),dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_BACKGROUND, 0), ^{
                    [database startOpenVGDBSync];
                });
            }
        }
    }

    if(status != OEImportExitErrorFatal)
    {
        [context save:nil];
        NSManagedObjectContext *parentContext = context.parentContext;
        [parentContext performBlock:^{
            [parentContext save:nil];
        }];
    }

    if(status != OEImportExitErrorResolvable)
    {
        [[NSFileManager defaultManager] removeItemAtURL:self.extractedFileURL error:nil];
    }

    _shouldExit = YES;
    self.error = error;
    self.exitStatus = status;
}

- (BOOL)shouldExit
{
    return _shouldExit || self.isCancelled;
}

@synthesize shouldExit=_shouldExit;

- (NSManagedObjectID*)romObjectID
{
    return self.rom.permanentID;
}

- (BOOL)shouldDisallowArchive:(XADArchive *)archive
{
    NSManagedObjectContext *context = self.importer.context;
    NSArray <OEDBSystem *> *enabledSystems = [OEDBSystem enabledSystemsinContext:context];
    NSMutableSet <NSString *> *enabledExtensions = [NSMutableSet set];
    OEDBSystem *arcadeSystem = [OEDBSystem systemForPluginIdentifier:@"openemu.system.arcade" inContext:context];
    BOOL isArcadeEnabled = arcadeSystem.enabled.boolValue;

    // Get extensions from all enabled systems.
    for (OEDBSystem *system in enabledSystems) {
        // Ignore Arcade file extensions (zip, 7z, chd).
        if ([system.systemIdentifier isEqualToString:@"openemu.system.arcade"])
            continue;

        [enabledExtensions addObjectsFromArray:system.plugin.supportedTypeExtensions];
    }

    // When Arcade is enabled, remove conflicting extensions found in Arcade ROMs.
    if (isArcadeEnabled)
        [enabledExtensions minusSet:[NSSet setWithArray:@[@"bin", @"rom", @"a26", @"a52", @"cas", @"col", @"com", @"int", @"p00", @"prg"]]];

    // Get extensions from files in archive.
    NSMutableSet <NSString *> *archiveExtensions = [NSMutableSet set];
    for (int i = 0; i < archive.numberOfEntries; i++) {
        NSString *name = [archive nameOfEntry:i];
        NSString *extension = name.pathExtension.lowercaseString;
        [archiveExtensions addObject:extension];
    }

    // Check if extensions in archive are found.
    [enabledExtensions intersectSet:archiveExtensions];
    NSArray *result = [enabledExtensions allObjects];
    if (result.count) {
        return YES;
    }

    return NO;
}

#pragma mark - NSOperation Overrides

- (void)main
{
    @autoreleasepool {
        [self.importer.context performBlockAndWait:^{
            if(self.shouldExit) return;

            [self OE_performImportStepCheckDirectory];
            if(self.shouldExit) return;

            [self OE_performImportStepCheckArchiveFile];
            if(self.shouldExit) return;

            [self OE_performImportStepHash];
            if(self.shouldExit) return;

            [self OE_performImportStepCheckHash];
            if(self.shouldExit) return;

            [self OE_performImportStepParseFile];
            if(self.shouldExit) return;

            [self OE_performImportStepDetermineSystem];
            if(self.shouldExit) return;

            [self OE_performImportStepOrganize];
            if(self.shouldExit) return;

            [self OE_performImportStepCreateCoreDataObjects];
        }];

    }
}

#pragma mark - Importing

- (void)OE_performImportStepCheckDirectory
{
    // Check if file at URL is a directory
    // if so, add new items for each item in the directory
    NSURL *url = self.URL;
    if(url.isDirectory)
    {
        NSArray *propertyKeys = @[NSURLIsPackageKey, NSURLIsHiddenKey];
        NSUInteger    options = NSDirectoryEnumerationSkipsSubdirectoryDescendants|NSDirectoryEnumerationSkipsPackageDescendants|NSDirectoryEnumerationSkipsHiddenFiles;

        NSDirectoryEnumerator *directoryEnumerator = [[NSFileManager defaultManager] enumeratorAtURL:url includingPropertiesForKeys:propertyKeys options:options errorHandler:NULL];

        NSURL *subURL = nil;
        while(self.isCancelled == NO && (subURL = directoryEnumerator.nextObject))
        {
            OEROMImporter    *importer = self.importer;
            OEImportOperation *subItem = [OEImportOperation operationWithURL:subURL inImporter:importer];
            if(subItem)
            {
                subItem.completionHandler = self.completionHandler;
                subItem.collectionID = self.collectionID;
                [importer addOperation:subItem];
            }
        }

        [self exitWithStatus:OEImportExitSuccess error:nil];
    }
}

- (void)OE_performImportStepCheckArchiveFile
{
    if(self.exploreArchives == NO) return;
    DLog();
    NSURL *url = self.URL;
    NSString *path = url.path;
    NSString *extension = path.pathExtension.lowercaseString;

    // nds and some isos might be recognized as compressed archives by XADArchive
    // but we don't ever want to extract anything from those files
    // Exclusions added here also need added to OEGameInfoHelper and OpenEmuHelperApp
    if([extension isEqualToString:@"nds"] || [extension isEqualToString:@"iso"])
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
        NSString    *formatName = archive.formatName;

        // XADArchive file detection is not exactly the best
        // ignore some formats
        if ([formatName isEqualToString:@"MacBinary"])
            return;
        
        if( [formatName isEqualToString:@"LZMA_Alone"])
            return;
        
        // disable multi-rom archives
        if (archive.numberOfEntries > 1) {
            // Check if archive contains known extensions, otherwise is assumed Arcade.
            self.isDisallowedArchiveWithMultipleFiles = [self shouldDisallowArchive:archive];

            return;
        }

        for(int i = 0; i < archive.numberOfEntries; i++)
        {
            if(([archive entryHasSize:i] && [archive sizeOfEntry:i] == 0) || [archive entryIsEncrypted:i] || [archive entryIsDirectory:i] || [archive entryIsArchive:i])
            {
                DLog(@"Entry %d is either empty, or a directory or encrypted or iteself an archive", i);
                continue;
            }

            NSString *folder = temporaryDirectoryForDecompressionOfPath(path);
            NSString *name   = [archive nameOfEntry:i];
            if (name.pathExtension.length == 0 && path.pathExtension.length > 0) {
                // this won't do. Re-add the archive's extension in case it's .smc or the like
                name = [name stringByAppendingPathExtension:path.pathExtension];
            }

            self.fileName = name;
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
                    NSLog(@"exception handeled");
                }
                
                // exception is caught but handler does not execute, so check if extraction worked
                NSNumber *fileSize = tmpURL.fileSize;
                if(fileSize.integerValue == 0)
                {
                    [fm removeItemAtPath:folder error:nil];
                    tmpURL = nil;
                    DLog(@"unpack failed");
                }
            }

            if(tmpURL)
            {
                self.extractedFileURL = tmpURL;
                self.archiveFileIndex = i;
                self.exploreArchives = NO;
            }
        }
    }
}

- (void)OE_performImportStepHash
{
    if(self.md5Hash) return;

    DLog();
    NSURL         *url = self.extractedFileURL ?: self.URL;
    NSString      *md5;
    NSError       *error = nil;
    NSFileManager *fileManager = [NSFileManager defaultManager];

    if(![fileManager hashFileAtURL:url md5:&md5 error:&error])
    {
        DLog(@"unable to hash file, this is probably a fatal error");
        DLog(@"%@", error);

        NSError *error = [NSError errorWithDomain:OEImportErrorDomainFatal code:OEImportErrorCodeNoHash userInfo:nil];
        [self exitWithStatus:OEImportExitErrorFatal error:error];
    }
    else
    {
        self.md5Hash = md5.lowercaseString;

        OEBIOSFile *biosFile = [[OEBIOSFile alloc] init];
        if([biosFile checkIfBIOSFileAndImportAtURL:url withMD5:self.md5Hash])
        {
            DLog(@"File seems to be a BIOS at %@", url);
            [self exitWithStatus:OEImportExitNone error:nil];
        }
    }

}

- (void)OE_performImportStepCheckHash
{
    DLog();
    NSError  *error = nil;
    NSString *md5   = self.md5Hash;
    NSManagedObjectContext *context = self.importer.context;

    OEDBRom *rom = nil;

    if(rom == nil)
        rom = [OEDBRom romWithMD5HashString:md5 inContext:context error:&error];

    if(rom != nil)
    {
        NSURL *romURL = rom.URL;
        self.rom = rom;
        self.romLocation = rom.game.system.lastLocalizedName;

        if(![romURL checkResourceIsReachableAndReturnError:nil])
        {
            DLog(@"rom file not available");
            error = [NSError errorWithDomain:OEImportErrorDomainFatal code:OEImportErrorCodeAlreadyInDatabaseFileUnreachable userInfo:nil];
        }
        else
        {
            error = [NSError errorWithDomain:OEImportErrorDomainFatal code:OEImportErrorCodeAlreadyInDatabase userInfo:nil];
        }

        [self exitWithStatus:OEImportExitErrorFatal error:error];
    }
}

- (void)OE_performImportStepParseFile
{
    /* Parsing M3U+CUE/BIN code walk

    1. +[OEFile fileWithURL:error:]
    2. -[OEDiscDescriptor initWithFileURL:error:]
    3. -[OEFile initWithFileURL:error:]
    4. -[OEM3UFile _setUpFileReferencesWithError:]
    5. -[OEDiscDescriptor _fileContentWithError:]
    5a. After -[OEDiscDescriptor _fileContentWithError:] returns, -[OEM3UFile _setUpFileReferencesWithError:] loops through the fileContent string and calls +[OEFile fileWithURL:error:] for each line, thus repeating steps 1-5 but for class OECUESheet. -[OEDiscDescriptor _validateFileURLs:withError:] will be called for the referenced BIN file during *each* CUE iteration
    6. Finally, -[OEDiscDescriptor _validateFileURLs:withError:] called to check the referenced CUE's array
     */
    NSURL *url = self.extractedFileURL ?: self.URL;

    NSError *error;
    OEFile *file = [OEFile fileWithURL:url error:&error];
    if (file != nil) {
        if (file.fileSize == 0) {
            error = [NSError errorWithDomain:OEImportErrorDomainFatal code:OEImportErrorCodeEmptyFile userInfo:nil];
            [self exitWithStatus:OEImportExitErrorFatal error:error];
        }
        self.file = file;
        return;
    }

    DLog(@"Error while parsing file: %@", error);
    [self exitWithStatus:OEImportExitErrorFatal error:error];
}

- (void)OE_performImportStepDetermineSystem
{
    DLog(@"URL: %@", self.sourceURL);
    if(self.rom != nil) return;

    NSError *error = nil;
    OEROMImporter *importer = self.importer;
    NSManagedObjectContext *context = importer.context;

    // see if systemidentifiers are set already (meaning that user determined system)
    NSArray <OEDBSystem *> *validSystems = nil;
    if(self.systemIdentifiers)
    {
        NSMutableArray *systems = [NSMutableArray arrayWithCapacity:self.systemIdentifiers.count];
        for(NSString *obj in self.systemIdentifiers)
        {
            OEDBSystem *system = [OEDBSystem systemForPluginIdentifier:obj inContext:context];
            if(system != nil)
                [systems addObject:system];
        }
        validSystems = systems;
    }
    else if([[NSUserDefaults standardUserDefaults] boolForKey:OEImportManualSystems])
        validSystems = [OEDBSystem allSystemsInContext:context];
    else
        validSystems = [OEDBSystem systemsForFile:self.file inContext:context error:&error];

    if (validSystems == nil) {
        DLog(@"Error while looking for a valid system: %@", error);
        [self exitWithStatus:OEImportExitErrorFatal error:error];
        return;
    }

    if(validSystems.count == 0)
    {
        DLog(@"Could not get valid systems");
        DLog(@"%@", error);
        if (self.extractedFileURL) {
            DLog(@"Try again with zip itself");
            self.extractedFileURL = nil;
            self.file = nil;
            [self OE_performImportStepParseFile];
            if(self.shouldExit) return;

            [self OE_performImportStepDetermineSystem];
        }
        else {
            // Unless for Arcade, compressed archives must not contain multiple files.
            if (self.isDisallowedArchiveWithMultipleFiles) {
                error = [NSError errorWithDomain:OEImportErrorDomainFatal code:OEImportErrorCodeDisallowArchivedFile userInfo:nil];
            } else {
                error = [NSError errorWithDomain:OEImportErrorDomainFatal code:OEImportErrorCodeNoSystem userInfo:nil];
            }
            [self exitWithStatus:OEImportExitErrorFatal error:error];
        }
        return;
    }
    else if(validSystems.count == 1)
    {
        self.systemIdentifiers = @[validSystems.lastObject.systemIdentifier];

        // Optical disc media or non-Arcade archives with multiple files must not import compressed.
        // Stops false positives importing into Arcade.
        OEDBSystem *system = [OEDBSystem systemForPluginIdentifier:self.systemIdentifiers.lastObject inContext:context];
        if ((self.extractedFileURL && [system.plugin.systemMedia containsObject:@"OESystemMediaOpticalDisc"]) || self.isDisallowedArchiveWithMultipleFiles)
        {
            error = [NSError errorWithDomain:OEImportErrorDomainFatal code:OEImportErrorCodeDisallowArchivedFile userInfo:nil];
            [self exitWithStatus:OEImportExitErrorFatal error:error];
        }
    }
    else // Found multiple valid systems after checking extension and system specific canHandleFile:
    {
        NSMutableArray <NSString *> *systemIDs = [NSMutableArray arrayWithCapacity:validSystems.count];
        for(OEDBSystem *system in validSystems)
        {
            NSString *systemIdentifier = system.systemIdentifier;
            [systemIDs addObject:systemIdentifier];
        }
        self.systemIdentifiers = systemIDs;

        NSDictionary *userInfo = @{ NSLocalizedDescriptionKey : @"Aaargh, too many systems. You need to choose one!" };
        error = [NSError errorWithDomain:OEImportErrorDomainResolvable code:OEImportErrorCodeMultipleSystems userInfo:userInfo];
        [self exitWithStatus:OEImportExitErrorResolvable error:error];
    }
}

- (void)OE_performImportStepOrganize
{
    NSUserDefaults *standardUserDefaults = [NSUserDefaults standardUserDefaults];
    BOOL copyToLibrary   = [standardUserDefaults boolForKey:OECopyToLibraryKey];

    if (!copyToLibrary) {
        // There is nothing to do in this method if we do not have to copy or move the file.
        return;
    }

    NSError *error       = nil;
    NSURL   *url         = self.URL;

    if (self.extractedFileURL) {
        OEFile *originalFile = [OEFile fileWithURL:url error:&error];
        if (!originalFile) {
            DLog(@"Failed to create file from original archive.");
            [self exitWithStatus:OEImportExitErrorFatal error:error];
            return;
        }

        self.file = originalFile;
    }

    NSManagedObjectContext *context = self.importer.context;

    // Unlock rom file so we can rename the copy directly
    BOOL romFileLocked = NO;
    NSNumber *isImmutable;
    if ([url getResourceValue:&isImmutable forKey:NSURLIsUserImmutableKey error:&error] && isImmutable.boolValue) {
        romFileLocked = YES;
        [url setResourceValue:@NO forKey:NSURLIsUserImmutableKey error:nil];
    }

    // Copy to system sub-folder in library folder if it's not already there
    OELibraryDatabase *database = self.importer.database;

    if(copyToLibrary && ![url isSubpathOfURL:database.romsFolderURL])
    {
        __block OEDBSystem *system = nil;
        if(self.rom != nil)
        {
            DLog(@"using rom object");
            system = self.rom.game.system;
        }
        else
        {
            NSAssert(self.systemIdentifiers.count == 1, @"System should have been detected at an earlier import stage");
            NSString *systemIdentifier = self.systemIdentifiers.lastObject;
            system = [OEDBSystem systemForPluginIdentifier:systemIdentifier inContext:context];
        }

        NSString *fullName  = url.lastPathComponent;
        NSString *extension = fullName.pathExtension;
        NSString *baseName  = fullName.stringByDeletingPathExtension;

        NSURL *systemFolder = [database romsFolderURLForSystem:system];
        // Copy game to subfolder in system's folder if system supports discs with descriptor file
        if (system.plugin.supportsDiscsWithDescriptorFile) {
            systemFolder = [systemFolder URLByAppendingPathComponent:baseName isDirectory:YES];

            systemFolder = [systemFolder uniqueURLUsingBlock:^NSURL *(NSInteger triesCount) {
                NSString *newName = [NSString stringWithFormat:@"%@ %ld", baseName, triesCount];
                return [systemFolder.URLByDeletingLastPathComponent URLByAppendingPathComponent:newName isDirectory:YES];
            }];

            [NSFileManager.defaultManager createDirectoryAtURL:systemFolder withIntermediateDirectories:YES attributes:nil error:nil];
        }
        NSURL *romURL       = [systemFolder URLByAppendingPathComponent:fullName];

        if([romURL isEqualTo:url])
        {
            self.URL = romURL;
            return;
        }

        romURL = [romURL uniqueURLUsingBlock:^NSURL *(NSInteger triesCount) {
            NSString *newName = [NSString stringWithFormat:@"%@ %ld.%@", baseName, triesCount, extension];
            return [systemFolder URLByAppendingPathComponent:newName];
        }];

        OEFile *copiedFile = [self.file fileByCopyingFileToURL:romURL error:&error];
        if (copiedFile != nil) {
            // Lock original file again
            if(romFileLocked)
                [url setResourceValue:@YES forKey:NSURLIsUserImmutableKey error:nil];

            url = [romURL copy];
            self.URL = url;
            self.file = copiedFile;
        } else {
            DLog(@"Could not copy rom to library");
            [self exitWithStatus:OEImportExitErrorFatal error:error];
            return;
        }
    }
}

- (void)OE_performImportStepCreateCoreDataObjects
{
    DLog();

    NSManagedObjectContext *context = self.importer.context;
    NSError *error = nil;
    OEDBRom *rom = self.rom;

    if(rom == nil)
    {
        rom = [OEDBRom createObjectInContext:context];
    }

    if(rom == nil)
    {
        NSError *error = [NSError errorWithDomain:OEImportErrorDomainFatal code:OEImportErrorCodeInvalidFile userInfo:nil];
        [self exitWithStatus:OEImportExitErrorFatal error:error];
        return;
    }

    NSString *md5 = self.md5Hash;

    rom.URL = self.URL;
    if(self.fileName != nil) rom.fileName = self.fileName;
    if(md5 != nil) rom.md5 = md5.lowercaseString;

    // Check if system plugin for ROM implemented headerLookupForFile: and serialLookupForFile:
    NSAssert(self.systemIdentifiers.count == 1, @"System should have been detected at an earlier import stage");
    NSString *systemIdentifier = self.systemIdentifiers.lastObject;

    NSURL *lookupURL = self.extractedFileURL ?: rom.URL;
    OEFile *file = [OEFile fileWithURL:lookupURL error:&error];
    if (file == nil) {
        [self exitWithStatus:OEImportExitErrorFatal error:error];
        return;
    }

    NSString *headerFound = [OEDBSystem headerForFile:file forSystem:systemIdentifier];
    NSString *serialFound = [OEDBSystem serialForFile:file forSystem:systemIdentifier];

    if(headerFound != nil)
    {
        rom.header = headerFound;
    }

    if(serialFound != nil)
    {
        rom.serial = serialFound;
    }

    if(self.archiveFileIndex != NSNotFound)
        rom.archiveFileIndex = @(self.archiveFileIndex);

    OEDBGame *game = rom.game;
    if(game == nil)
    {
        NSString *systemIdentifier = self.systemIdentifiers.lastObject;
        OEDBSystem *system = [OEDBSystem systemForPluginIdentifier:systemIdentifier inContext:context];
        if(system != nil)
        {
            // Ensure single-file Arcade archives containing a known file suffix (bin, rom) retain their archived name.
            if ([systemIdentifier isEqualToString:@"openemu.system.arcade"]) {
                self.extractedFileURL = nil;
            }

            NSURL *url = self.extractedFileURL ?: rom.URL;
            NSString *gameTitleWithSuffix = url.lastPathComponent;
            NSString *gameTitleWithoutSuffix = gameTitleWithSuffix.stringByDeletingPathExtension;

            game = [OEDBGame createObjectInContext:context];
            game.name = gameTitleWithoutSuffix;
            game.system = system;
            game.importDate = [NSDate date];
        }
        else
        {
            NSDictionary *userInfo = @{ NSLocalizedDescriptionKey : @"No system! Someone must have deleted or disabled it!" };
            error = [NSError errorWithDomain:OEImportErrorDomainFatal code:OEImportErrorCodeNoSystem userInfo:userInfo];
            [self exitWithStatus:OEImportExitErrorFatal error:error];
            return;
        }
    }
    
    if(game != nil)
    {
        rom.game = game;

        if([[NSUserDefaults standardUserDefaults] boolForKey:OEAutomaticallyGetInfoKey])
        {
            game.status = @(OEDBGameStatusProcessing);
        }

        self.rom = rom;

        [self exitWithStatus:OEImportExitSuccess error:nil];
    }
    else
    {
        [rom delete];
        [game delete];

        self.rom = nil;
        NSError *error = [NSError errorWithDomain:OEImportErrorDomainFatal code:OEImportErrorCodeNoGame userInfo:nil];
        [self exitWithStatus:OEImportExitErrorFatal error:error];
    }
}

@end

NS_ASSUME_NONNULL_END
