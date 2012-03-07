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
#import "ArchiveVG.h"

#import "NSFileManager+OEHashingAdditions.h"
@interface OEDBRom (Private)
+ (id)_createRomWithoutChecksWithFilePath:(NSString*)filePath md5:(NSString*)md5 crc:(NSString*)crc inDatabase:(OELibraryDatabase*)database error:(NSError**)outError; 

@end
@implementation OEDBRom
#pragma mark -
#pragma mark Creating and Obtaining OEDBRoms
+ (id)createRomWithFilePath:(NSString *)filePath error:(NSError **)outError
{
    return [self createRomWithFilePath:filePath inDatabase:[OELibraryDatabase defaultDatabase] error:outError];
}
+ (id)createRomWithFilePath:(NSString*)filePath inDatabase:(OELibraryDatabase*)database error:(NSError**)outError
{
    return [self _createRomWithoutChecksWithFilePath:filePath md5:nil crc:nil inDatabase:database error:outError];
}
+ (id)romWithFilePath:(NSString*)path createIfNecessary:(BOOL)createFlag error:(NSError**)outError
{
    return [self romWithFilePath:path createIfNecessary:createFlag inDatabase:[OELibraryDatabase defaultDatabase] error:outError];
}
+ (id)createRomWithFilePath:(NSString*)filePath md5:(NSString*)md5 crc:(NSString*)crc error:(NSError**)outError
{
    return [self createRomWithFilePath:filePath md5:md5 crc:crc inDatabase:[OELibraryDatabase defaultDatabase] error:outError];
}
+ (id)createRomWithFilePath:(NSString*)filePath md5:(NSString*)md5 crc:(NSString*)crc inDatabase:(OELibraryDatabase*)database error:(NSError**)outError
{
    return [self _createRomWithoutChecksWithFilePath:filePath md5:md5 crc:crc inDatabase:database error:outError];
}

+ (id)romWithFilePath:(NSString*)path createIfNecessary:(BOOL)createFlag inDatabase:(OELibraryDatabase*)database error:(NSError**)outError
{
    if(path==nil) return nil;
    
    OEDBRom *rom = nil;
    NSManagedObjectContext *context = [database managedObjectContext];
    
    NSPredicate *predicate = [NSPredicate predicateWithFormat:@"path == %@", path];    
    NSFetchRequest *fetchRequest = [NSFetchRequest fetchRequestWithEntityName:[self entityName]];
    [fetchRequest setIncludesPendingChanges:YES];
    [fetchRequest setPredicate:predicate];
    [fetchRequest setFetchLimit:1];
    NSArray *roms = [context executeFetchRequest:fetchRequest error:outError];
    if(roms)
    {
        rom = [roms lastObject];
    }
    
    if(!rom && createFlag)
    {
        return [self _createRomWithoutChecksWithFilePath:path md5:nil crc:nil inDatabase:database error:outError];
    }
    return rom;
}

+ (id)_createRomWithoutChecksWithFilePath:(NSString*)filePath md5:(NSString*)md5 crc:(NSString*)crc inDatabase:(OELibraryDatabase*)database error:(NSError**)outError; 
{
    if(filePath == nil)
    {
        if(outError!=NULL)
        *outError = [NSError errorWithDomain:@"OEErrorDomain" code:0 userInfo:[NSDictionary dictionaryWithObject:@"_createRomWithoutChecksWithFilePath called without filepath" forKey:NSLocalizedDescriptionKey]];
        return nil;   
    }    
    NSManagedObjectContext *context = [database managedObjectContext];
    NSEntityDescription *description = [self entityDescriptionInContext:context];
    OEDBRom *rom = [[OEDBRom alloc] initWithEntity:description insertIntoManagedObjectContext:context];
    
    NSUserDefaults *standardUserDefaults = [NSUserDefaults standardUserDefaults];
    BOOL copyToDatabase = [standardUserDefaults boolForKey:UDCopyToLibraryKey];
    BOOL useMD5 = [standardUserDefaults boolForKey:UDUseMD5HashingKey];
    
    NSString *newFilePath = [filePath copy];
    if(copyToDatabase && ![newFilePath hasPrefix:[database databaseFolderPath]])
    {
        NSString *databaseUnsortedFolder = [database databaseUnsortedRomsPath];
        NSFileManager *defaultManager = [NSFileManager defaultManager];
        
        NSInteger i = 0;
        NSString *fileName = [[newFilePath lastPathComponent] stringByDeletingPathExtension];
        NSString *fileSuffix = [newFilePath pathExtension];
        
        newFilePath = [databaseUnsortedFolder stringByAppendingPathComponent:[newFilePath lastPathComponent]];
        while([defaultManager fileExistsAtPath:newFilePath])
        {
            i++;
            newFilePath = [NSString stringWithFormat:@"%@/%@ %d.%@", databaseUnsortedFolder, fileName, i, fileSuffix];            
        }
        
        if(![defaultManager copyItemAtPath:filePath toPath:newFilePath error:outError])
        {
            [context deleteObject:rom];
            if(outError!=NULL)
            *outError = [NSError errorWithDomain:@"OEErrorDomain" code:1 userInfo:[NSDictionary dictionaryWithObject:@"Copying ROM-File failed!" forKey:NSLocalizedDescriptionKey]];
            return nil;
        }
    }
    
    [rom setPath:newFilePath];
    
    if(md5!=nil) [rom setMd5:md5];
    if(crc!=nil) [rom setCrc32:crc];
    
    BOOL calculateHash = (useMD5 && md5==nil) || (!useMD5 && crc==nil);
    if(calculateHash)
    {
        NSData *data = [NSData dataWithContentsOfFile:newFilePath options:NSDataReadingUncached error:outError];
        if(!data)
        {
            [[NSFileManager defaultManager] removeItemAtPath:newFilePath error:nil];
            
            [context deleteObject:rom];
            if(outError!=NULL)
                *outError = [NSError errorWithDomain:@"OEErrorDomain" code:2 userInfo:[NSDictionary dictionaryWithObject:@"Calculating Hash for ROM-File failed!" forKey:NSLocalizedDescriptionKey]];
            return nil;
        }
        
        NSFileManager* defaultFileManager = [NSFileManager defaultManager];
        NSString* hash;
        if(useMD5)
        {
            hash = [defaultFileManager md5DigestForFileAtPath:filePath error:outError];
            if(!hash)
                return nil;
            [rom setMd5:hash];
        }
        else
        {
            hash = [defaultFileManager crc32ForFileAtPath:filePath error:outError];
            if(!hash)
                return nil;
            [rom setCrc32:hash];
        }
    }
    
    return rom;
}

+ (id)romWithFileName:(NSString*)filename error:(NSError**)outError
{
    return [self romWithFileName:filename inDatabase:[OELibraryDatabase defaultDatabase] error:outError];
}
+ (id)romWithFileName:(NSString*)filename inDatabase:(OELibraryDatabase*)database error:(NSError**)outError
{
    if(filename==nil) return nil;
    
    NSManagedObjectContext *context = [database managedObjectContext];
    
    NSPredicate *predicate = [NSPredicate predicateWithFormat:@"path ENDSWITH %@", filename];    
    NSFetchRequest *fetchRequest = [NSFetchRequest fetchRequestWithEntityName:[self entityName]];
    [fetchRequest setIncludesPendingChanges:YES];
    [fetchRequest setPredicate:predicate];
    [fetchRequest setFetchLimit:1];
    
    NSArray *roms = [context executeFetchRequest:fetchRequest error:outError];
    if(!roms)
    {
        return nil;
    }
    return [roms lastObject];
}
+ (id)romWithCRC32HashString:(NSString*)crcHash error:(NSError**)outError
{
    return [self romWithCRC32HashString:crcHash inDatabase:[OELibraryDatabase defaultDatabase] error:outError];
}

+ (id)romWithCRC32HashString:(NSString*)crcHash inDatabase:(OELibraryDatabase*)database error:(NSError**)outError
{
    if(crcHash==nil) return nil;
    
    NSManagedObjectContext *context = [database managedObjectContext];
    NSPredicate *predicate = [NSPredicate predicateWithFormat:@"crc32 == %@", crcHash];    
    NSFetchRequest *fetchRequest = [NSFetchRequest fetchRequestWithEntityName:[self entityName]];
    [fetchRequest setFetchLimit:1];
    [fetchRequest setIncludesPendingChanges:YES];
    [fetchRequest setPredicate:predicate];

    NSArray *roms = [context executeFetchRequest:fetchRequest error:outError];
    if(!roms)
    {
        return nil;
    }

    return [roms lastObject];
}
+ (id)romWithMD5HashString:(NSString*)md5Hash error:(NSError**)outError
{
    return [self romWithMD5HashString:md5Hash inDatabase:[OELibraryDatabase defaultDatabase] error:outError];
}
+ (id)romWithMD5HashString:(NSString*)md5Hash inDatabase:(OELibraryDatabase*)database error:(NSError**)outError
{
    if(md5Hash==nil) return nil;
    
    NSManagedObjectContext *context = [database managedObjectContext];
    NSPredicate *predicate = [NSPredicate predicateWithFormat:@"md5 == %@", md5Hash];    
    NSFetchRequest *fetchRequest = [NSFetchRequest fetchRequestWithEntityName:[self entityName]];
    [fetchRequest setFetchLimit:1];
    [fetchRequest setIncludesPendingChanges:YES];
    [fetchRequest setPredicate:predicate];
    
    NSArray *roms = [context executeFetchRequest:fetchRequest error:outError];
    if(!roms)
    {
        return nil;
    }
    
    return [roms lastObject];
}

#pragma mark -
#pragma mark Accessors
- (NSURL*)url
{
    return [NSURL fileURLWithPath:[self path]];
}

- (NSString*)md5Hash
{
    NSString *hash = [self md5];
    if(!hash)
    {
        NSError *error = nil;
        NSString *filePath = [self path];
        if(![[NSFileManager defaultManager] fileExistsAtPath:filePath])
        {
            // TODO: mark self as file missing
            return nil;
        }
        
        hash = [[NSFileManager defaultManager] md5DigestForFileAtPath:filePath error:&error];
        if(!hash)
        {
            DLog(@"%@", error);
            // TODO: mark self as file missing
            return nil;
        }
        [self setMd5:hash];
    }
    return hash;
}

- (NSString*)md5HashIfAvailable
{
    return [self md5];
}

- (NSString*)crcHash
{
    NSString *hash = [self crc32];
    if(!hash)
    {
        NSError *error = nil;
        NSString *filePath = [self path];
        if(![[NSFileManager defaultManager] fileExistsAtPath:filePath])
        {
            // TODO: mark self as file missing
            return nil;
        }
        
        hash = [[NSFileManager defaultManager] crc32ForFileAtPath:filePath error:&error];
        if(!hash)
        {
            DLog(@"%@", error);
            // TODO: mark self as file missing
            return nil;
        }
        [self setCrc32:hash];
    }
    return hash;    
}

- (NSString*)crcHashIfAvailable
{
    return [self crc32];
}


- (NSArray*)saveStatesByTimestampAscending:(BOOL)ascFlag
{
    NSSet *set = [self saveStates];
    return [[set allObjects] sortedArrayUsingComparator:^NSComparisonResult(OEDBSaveState *obj1, OEDBSaveState *obj2) 
            {
                NSDate *d1 = [obj1 timestamp], *d2=[obj2 timestamp];
                if(ascFlag)
                    return [d2 compare:d1];
                return [d1 compare:d2];
            }];
}

- (NSInteger)saveStateCount
{
    return [[self saveStates] count];
}
#pragma mark -
#pragma mark Mainpulating a rom
- (void)markAsPlayedNow
{
    [self setLastPlayed:[NSDate date]];
}
#pragma mark -
#pragma mark Core Data utilities
+ (NSString *)entityName
{
    return @"ROM";
}

+ (NSEntityDescription *)entityDescriptionInContext:(NSManagedObjectContext *)context
{
    return [NSEntityDescription entityForName:[self entityName] inManagedObjectContext:context];
}
#pragma mark -
- (void)doInitialSetupWithDatabase:(OELibraryDatabase*)db
{
    NSString *filePath = [self path];
    
    DLog(@"doInitialSetupWithDatabase: %@", filePath);
    
    BOOL useMD5 = [[NSUserDefaults standardUserDefaults] boolForKey:UDUseMD5HashingKey];
    NSError *error = nil;
    
    NSString *hash;
    OEDBRom *rom;
    if(useMD5)
    {
        hash = [[NSFileManager defaultManager] md5DigestForFileAtPath:filePath error:nil];
        rom = [db romForMD5Hash:hash];
    }
    else 
    {
        hash = [[NSFileManager defaultManager] crc32ForFileAtPath:filePath error:nil];
        rom = [db romForCRC32Hash:hash];
    }
    
    if(rom)
    {
        NSSet *romsInGame = [self valueForKeyPath:@"game.roms"];
        if([romsInGame count]==1)
        {
            NSString *path = [self path];
            if([path hasPrefix:[[NSUserDefaults standardUserDefaults] stringForKey:UDDatabasePathKey]])
            {
                [[NSFileManager defaultManager] removeItemAtPath:path error:nil];
            }
            
            [self.managedObjectContext deleteObject:[self game]];
            [self.managedObjectContext deleteObject:self];
        }
    }
    
    if(useMD5)
        [self setMd5:hash];
    else
        [self setCrc32:hash];
    
    NSDictionary *gameDictionary;
    if(useMD5)
    {
        gameDictionary = [ArchiveVG gameInfoByMD5:hash];
    } 
    else
    {
        gameDictionary = [ArchiveVG gameInfoByCRC:hash];
    }
    
    OEDBGame *game = [db gameWithArchiveID:(NSNumber*)[gameDictionary valueForKey:(NSString*)AVGGameIDKey]];
    if(game)
    {
        NSLog(@"Game is aleady present");
        [self.managedObjectContext deleteObject:[self game]];
        [[game mutableRoms] addObject:self];
    } 
    else
        game = [self game];

    [game mergeWithGameInfo:gameDictionary];
    
    BOOL organize = [[NSUserDefaults standardUserDefaults] boolForKey:UDOrganizeLibraryKey];
    NSString *path = [self path];
    NSString *libraryPath = [[NSUserDefaults standardUserDefaults] stringForKey:UDDatabasePathKey];
    if(organize && [path hasPrefix:libraryPath])
    {
        NSString *systemName = [self valueForKeyPath:@"game.system.systemIdentifier"];
        NSString *folderPath = [libraryPath stringByAppendingPathComponent:systemName];
        
        if(![[NSFileManager defaultManager] createDirectoryAtPath:folderPath withIntermediateDirectories:YES attributes:nil error:&error])
        {
            NSLog(@"could not create system folder");
            NSLog(@"%@", error);
            return;
        }
        
        NSString *romName = [[self path] lastPathComponent];
        
        // TODO: use tosec for new rom path if available
        NSString *newRomPath = [folderPath stringByAppendingPathComponent:romName];
        if(![[NSFileManager defaultManager] moveItemAtPath:path toPath:newRomPath error:&error])
        {
            NSLog(@"could not move rom to new path");
            NSLog(@"%@", error);
            return;
        }
        
        [self setPath:newRomPath];
    }
    
}
#pragma mark -
#pragma mark Data Model Properties
@dynamic path, favorite, crc32, md5, lastPlayed;

#pragma mark -
#pragma mark Data Model Relationships
@dynamic game, saveStates, tosec;
- (NSMutableSet*)mutableSaveStates
{
    return [self mutableSetValueForKey:@"saveStates"];
}
@end
