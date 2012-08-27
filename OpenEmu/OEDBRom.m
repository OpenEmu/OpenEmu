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

#import "OEDBRom.h"
#import "OEDBGame.h"
#import "OEDBSaveState.h"

#import "OELibraryDatabase.h"

#import "NSFileManager+OEHashingAdditions.h"
#import "NSURL+OELibraryAdditions.h"

@interface OEDBRom ()
+ (id)OE_createRomWithoutChecksWithURL:(NSURL *)url md5:(NSString *)md5 crc:(NSString *)crc inDatabase:(OELibraryDatabase *)database error:(NSError **)outError;
- (void)OE_calculateHashes;
@end

@implementation OEDBRom
@dynamic URL;
// Data Model Properties
@dynamic bookmarkData, favorite, crc32, md5, lastPlayed, fileSize;
// Data Model Relationships
@dynamic game, saveStates, tosec;

#pragma mark -
#pragma mark Creating and Obtaining OEDBRoms
+ (id)romWithID:(NSManagedObjectID *)objID
{
    return [self romWithID:objID inDatabase:[OELibraryDatabase defaultDatabase]];
}

+ (id)romWithID:(NSManagedObjectID *)objID inDatabase:(OELibraryDatabase *)database
{
    return [[database managedObjectContext] objectWithID:objID];
}

+ (id)romWithURIURL:(NSURL *)objIDUrl
{
    return [self romWithURIURL:objIDUrl inDatabase:[OELibraryDatabase defaultDatabase]];
}

+ (id)romWithURIURL:(NSURL*)objIDUrl inDatabase:(OELibraryDatabase *)database
{
    NSManagedObjectID *objID = [database managedObjectIDForURIRepresentation:objIDUrl];
    return [self romWithID:objID inDatabase:database];
}

+ (id)createRomWithURL:(NSURL *)url error:(NSError **)outError
{
    return [self createRomWithURL:url inDatabase:[OELibraryDatabase defaultDatabase] error:outError];
}

+ (id)createRomWithURL:(NSURL *)url inDatabase:(OELibraryDatabase *)database error:(NSError **)outError
{
    return [self OE_createRomWithoutChecksWithURL:url md5:nil crc:nil inDatabase:database error:outError];
} 

+ (id)romWithURL:(NSURL *)url error:(NSError **)outError
{
    return [self romWithURL:url inDatabase:[OELibraryDatabase defaultDatabase] error:outError];
}

+ (id)createRomWithURL:(NSURL *)url md5:(NSString *)md5 crc:(NSString *)crc error:(NSError **)outError
{
    return [self createRomWithURL:url md5:md5 crc:crc inDatabase:[OELibraryDatabase defaultDatabase] error:outError];
}

+ (id)createRomWithURL:(NSURL *)url md5:(NSString *)md5 crc:(NSString *)crc inDatabase:(OELibraryDatabase *)database error:(NSError **)outError
{
    return [self OE_createRomWithoutChecksWithURL:url md5:md5 crc:crc inDatabase:database error:outError];
}

+ (id)romWithURL:(NSURL *)url inDatabase:(OELibraryDatabase *)database error:(NSError **)outError
{
    if(url == nil) return nil;
    

    NSManagedObjectContext *context = [database managedObjectContext];
    NSPredicate *predicate = [NSPredicate predicateWithFormat:@"URL == %@", url];
    NSFetchRequest *fetchRequest = [NSFetchRequest fetchRequestWithEntityName:[self entityName]];
    
    [fetchRequest setFetchLimit:1];
    [fetchRequest setIncludesPendingChanges:YES];
    [fetchRequest setPredicate:predicate];
    
    return [[context executeFetchRequest:fetchRequest error:outError] lastObject];
}

+ (id)OE_createRomWithoutChecksWithURL:(NSURL *)url md5:(NSString *)md5 crc:(NSString *)crc inDatabase:(OELibraryDatabase *)database error:(NSError **)outError
{
    if(url == nil)
    {
        if(outError != NULL)
            *outError = [NSError errorWithDomain:@"OEErrorDomain" code:0 userInfo:[NSDictionary dictionaryWithObject:@"_createRomWithoutChecksWithURL called without url" forKey:NSLocalizedDescriptionKey]];
        return nil;
    }
    
    NSManagedObjectContext *context = [database managedObjectContext];
    NSEntityDescription *description = [self entityDescriptionInContext:context];
    OEDBRom *rom = [[OEDBRom alloc] initWithEntity:description insertIntoManagedObjectContext:context];
    
    NSUserDefaults *standardUserDefaults = [NSUserDefaults standardUserDefaults];
    BOOL copyToDatabase = [standardUserDefaults boolForKey:UDCopyToLibraryKey];
    
    NSURL *newURL = nil;
    if(copyToDatabase && ![url isSubpathOfURL:[database romsFolderURL]])
    {
                       newURL                       = [url copy];
        NSURL         *databaseUnsortedFolderURL    = [database unsortedRomsFolderURL];
        NSFileManager *defaultManager               = [NSFileManager defaultManager];
        
        NSString *fileName = [[newURL lastPathComponent] stringByDeletingPathExtension];
        NSString *fileSuffix = [newURL pathExtension];
        
        newURL = [databaseUnsortedFolderURL URLByAppendingPathComponent:[newURL lastPathComponent] isDirectory:NO];
        newURL = [newURL uniqueURLUsingBlock:
                  ^ NSURL * (NSInteger triesCount)
                  {
                      NSString *newFileName = [NSString stringWithFormat:@"%@ %ld.%@", fileName, triesCount, fileSuffix];
                      return [databaseUnsortedFolderURL URLByAppendingPathComponent:newFileName isDirectory:NO];
                  }];

        if(![defaultManager copyItemAtURL:url toURL:newURL error:outError])
        {
            [context deleteObject:rom];
            if(outError!=NULL)
                *outError = [NSError errorWithDomain:@"OEErrorDomain" code:1 userInfo:[NSDictionary dictionaryWithObject:@"Copying ROM-File failed!" forKey:NSLocalizedDescriptionKey]];
            
            return nil;
        }
    }
    
    [rom setURL:newURL ? : url];
    
    if(md5 != nil) [rom setMd5:md5];
    if(crc != nil) [rom setCrc32:crc];
    
    if(md5 == nil && crc == nil)
    {
        NSString *crcHash, *md5Hash;
        if([[NSFileManager defaultManager] hashFileAtURL:url md5:&md5Hash crc32:&crcHash error:outError])
        {
            [rom setMd5:md5Hash];
            [rom setCrc32:crcHash];
        }
        else if(outError != NULL)
        {
                *outError = [NSError errorWithDomain:@"OEErrorDomain" code:2 userInfo:[NSDictionary dictionaryWithObject:@"Calculating Hash for ROM-File failed!" forKey:NSLocalizedDescriptionKey]];
                DLog(@"%@", *outError);
        }
    }
    [rom setFileSize:[url fileSize]];
    
    return rom;
}

#pragma mark -

+ (id)romWithCRC32HashString:(NSString *)crcHash error:(NSError **)outError
{
    return [self romWithCRC32HashString:crcHash inDatabase:[OELibraryDatabase defaultDatabase] error:outError];
}

+ (id)romWithCRC32HashString:(NSString *)crcHash inDatabase:(OELibraryDatabase *)database error:(NSError **)outError
{
    if(crcHash == nil) return nil;
    
    NSManagedObjectContext *context = [database managedObjectContext];
    NSPredicate *predicate = [NSPredicate predicateWithFormat:@"crc32 == %@", crcHash];    
    NSFetchRequest *fetchRequest = [NSFetchRequest fetchRequestWithEntityName:[self entityName]];
    
    [fetchRequest setFetchLimit:1];
    [fetchRequest setIncludesPendingChanges:YES];
    [fetchRequest setPredicate:predicate];
    
    return [[context executeFetchRequest:fetchRequest error:outError] lastObject];
}

+ (id)romWithMD5HashString:(NSString *)md5Hash error:(NSError **)outError
{
    return [self romWithMD5HashString:md5Hash inDatabase:[OELibraryDatabase defaultDatabase] error:outError];
}

+ (id)romWithMD5HashString:(NSString *)md5Hash inDatabase:(OELibraryDatabase *)database error:(NSError **)outError
{
    if(md5Hash == nil) return nil;
    
    NSManagedObjectContext *context = [database managedObjectContext];
    NSPredicate *predicate = [NSPredicate predicateWithFormat:@"md5 == %@", md5Hash];    
    NSFetchRequest *fetchRequest = [NSFetchRequest fetchRequestWithEntityName:[self entityName]];
    [fetchRequest setFetchLimit:1];
    [fetchRequest setIncludesPendingChanges:YES];
    [fetchRequest setPredicate:predicate];
    
    return [[context executeFetchRequest:fetchRequest error:outError] lastObject];
}

#pragma mark -
#pragma mark Accessors

- (NSURL *)URL
{
    return [NSURL URLByResolvingBookmarkData:[self bookmarkData] options:0 relativeToURL:nil bookmarkDataIsStale:NULL error:nil];
}

- (void)setURL:(NSURL *)url
{
    NSData *data = [url bookmarkDataWithOptions:NSURLBookmarkCreationMinimalBookmark includingResourceValuesForKeys:nil relativeToURL:nil error:nil];
    if(data != nil) [self setBookmarkData:data];
}

- (NSString *)md5Hash
{
    NSString *hash = [self md5];
    if(hash != nil)
    {
        [self OE_calculateHashes];
        return [self md5HashIfAvailable];
    }
    
    return hash;
}

- (NSString *)md5HashIfAvailable
{
    return [self md5];
}

- (NSString *)crcHash
{
    NSString *hash = [self crc32];
    
    if(hash != nil)
    {
        [self OE_calculateHashes];
        return [self crcHashIfAvailable];
    }
    
    return hash;    
}

- (NSString *)crcHashIfAvailable
{
    return [self crc32];
}

- (void)OE_calculateHashes
{
    NSError *error = nil;
    NSURL *url = [self URL];
    
    if(![url checkResourceIsReachableAndReturnError:&error])
    {
        // TODO: mark self as file missing
        DLog(@"%@", error);
        return;
    }
    
    NSString *md5Hash, *crc32Hash;
    if(![[NSFileManager defaultManager] hashFileAtURL:url md5:&md5Hash crc32:&crc32Hash error:&error])
    {
        DLog(@"%@", error);
        // TODO: mark self as file missing
        return;
    }
    
    [self setCrc32:crc32Hash];
    [self setMd5:md5Hash];
}


- (NSArray *)normalSaveStates
{
    NSPredicate *predicate = [NSPredicate predicateWithFormat:@"NOT (name beginswith[c] %@)", @"OESpecialState"];
    NSSet *set = [self saveStates];
    set = [set filteredSetUsingPredicate:predicate];
    
    return [set allObjects];
}

- (NSArray *)normalSaveStatesByTimestampAscending:(BOOL)ascFlag
{
    return [[self normalSaveStates] sortedArrayUsingComparator:
            ^ NSComparisonResult (OEDBSaveState *obj1, OEDBSaveState *obj2)
            {
                NSDate *d1 = [obj1 timestamp], *d2 = [obj2 timestamp];
                
                return ascFlag ? [d2 compare:d1] : [d1 compare:d2];
            }];
}


- (NSInteger)saveStateCount
{
    return [[self saveStates] count];
}

- (OEDBSaveState *)autosaveState
{
    NSPredicate *predicate = [NSPredicate predicateWithFormat:@"name beginswith[c] %@", OESaveStateAutosaveName];
    NSSet *set = [self saveStates];
    set = [set filteredSetUsingPredicate:predicate];
    
    return [set anyObject];
}

- (NSArray *)quickSaveStates
{
    NSPredicate *predicate = [NSPredicate predicateWithFormat:@"name beginswith[c] %@", OESaveStateQuicksaveName];
    NSSet *set = [self saveStates];
    set = [set filteredSetUsingPredicate:predicate];
    
    return [set allObjects];
}

- (OEDBSaveState *)quickSaveStateInSlot:(int)num
{
    NSString *quickSaveName = num == 0 ? OESaveStateQuicksaveName:[NSString stringWithFormat:@"%@%d", OESaveStateQuicksaveName, num];
    
    NSPredicate *predicate = [NSPredicate predicateWithFormat:@"name beginswith[c] %@", quickSaveName];
    NSSet *set = [self saveStates];
    set = [set filteredSetUsingPredicate:predicate];
    
    return [set anyObject];
}

- (OEDBSaveState *)saveStateWithName:(NSString *)string
{
    NSPredicate *predicate = [NSPredicate predicateWithFormat:@"name == %@", string];
    NSSet *set = [self saveStates];
    set = [set filteredSetUsingPredicate:predicate];
    
    return [set anyObject];
}

- (void)removeMissingStates
{
    NSSet *set = [[self saveStates] copy];
    [set makeObjectsPerformSelector:@selector(removeIfMissing)];
}

#pragma mark -
#pragma mark Mainpulating a rom

- (void)markAsPlayedNow
{
    [self setLastPlayed:[NSDate date]];
}

#pragma mark -
#pragma mark Core Data utilities

- (void)deleteByMovingFile:(BOOL)moveToTrash keepSaveStates:(BOOL)statesFlag
{
    NSURL *url = [self URL];
    
    if([url isSubpathOfURL:[[self libraryDatabase] romsFolderURL]])
    {
        NSString *path = [url path];
        [[NSWorkspace sharedWorkspace] performFileOperation:NSWorkspaceRecycleOperation source:[path stringByDeletingLastPathComponent] destination:nil files:[NSArray arrayWithObject:[path lastPathComponent]] tag:NULL];
    }
    
    if(statesFlag)
    {
        // TODO: remove states
    }
    
    [[self managedObjectContext] deleteObject:self];
}

+ (NSString *)entityName
{
    return @"ROM";
}

+ (NSEntityDescription *)entityDescriptionInContext:(NSManagedObjectContext *)context
{
    return [NSEntityDescription entityForName:[self entityName] inManagedObjectContext:context];
}

- (NSMutableSet *)mutableSaveStates
{
    return [self mutableSetValueForKey:@"saveStates"];
}

@end
