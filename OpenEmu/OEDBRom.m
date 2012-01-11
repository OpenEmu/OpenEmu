//
//  OEDBRom.m
//  OpenEmu
//
//  Created by Carl Leimbrock on 10.10.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "OEDBRom.h"
#import "OEDBGame.h"
#import "OEDBSaveState.h"
#import "NSData+HashingAdditions.h"

#import "OELibraryDatabase.h"
#import "ArchiveVG.h"
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
    
    OEDBRom* rom = nil;
    NSManagedObjectContext* context = [database managedObjectContext];
    
    NSPredicate* predicate = [NSPredicate predicateWithFormat:@"path == %@", path];    
    NSFetchRequest* fetchRequest = [NSFetchRequest fetchRequestWithEntityName:[self entityName]];
    [fetchRequest setIncludesPendingChanges:YES];
    [fetchRequest setPredicate:predicate];
    [fetchRequest setFetchLimit:1];
    NSArray* roms = [context executeFetchRequest:fetchRequest error:outError];
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
    NSManagedObjectContext* context = [database managedObjectContext];
    NSEntityDescription* description = [self entityDescriptionInContext:context];
    OEDBRom* rom = [[OEDBRom alloc] initWithEntity:description insertIntoManagedObjectContext:context];
    
    NSUserDefaults* standardUserDefaults = [NSUserDefaults standardUserDefaults];
    BOOL copyToDatabase = [standardUserDefaults boolForKey:UDCopyToLibraryKey];
    BOOL useMD5 = [standardUserDefaults boolForKey:UDUseMD5HashingKey];
    
    NSString* newFilePath = [[filePath copy] autorelease];
    if(copyToDatabase && ![newFilePath hasPrefix:[database databaseFolderPath]])
    {
        NSString* databaseUnsortedFolder = [database databaseUnsortedRomsPath];
        NSFileManager* defaultManager = [NSFileManager defaultManager];
        
        NSInteger i = 0;
        NSString* fileName = [[newFilePath lastPathComponent] stringByDeletingPathExtension];
        NSString* fileSuffix = [newFilePath pathExtension];
        
        newFilePath = [databaseUnsortedFolder stringByAppendingPathComponent:[newFilePath lastPathComponent]];
        while([defaultManager fileExistsAtPath:newFilePath])
        {
            i++;
            newFilePath = [NSString stringWithFormat:@"%@/%@ %d.%@", databaseUnsortedFolder, fileName, i, fileSuffix];            
        }
        
        if(![defaultManager copyItemAtPath:filePath toPath:newFilePath error:outError])
        {
            [context deleteObject:rom];
            [rom release];
            if(outError!=NULL)
            *outError = [NSError errorWithDomain:@"OEErrorDomain" code:1 userInfo:[NSDictionary dictionaryWithObject:@"Copying ROM-File failed!" forKey:NSLocalizedDescriptionKey]];
            return nil;
        }
    }
    
    [rom setValue:newFilePath forKey:@"path"];
    
    if(md5!=nil) [rom setValue:md5 forKey:@"md5"];
    if(crc!=nil) [rom setValue:md5 forKey:@"crc32"];
    
    BOOL calculateHash = (useMD5 && md5==nil) || (!useMD5 && crc==nil);
    if(calculateHash)
    {
        NSData* data = [NSData dataWithContentsOfFile:newFilePath options:NSDataReadingUncached error:outError];
        if(!data)
        {
            [[NSFileManager defaultManager] removeItemAtPath:newFilePath error:nil];
            
            [context deleteObject:rom];
            [rom release];
            if(outError!=NULL)
                *outError = [NSError errorWithDomain:@"OEErrorDomain" code:2 userInfo:[NSDictionary dictionaryWithObject:@"Calculating Hash for ROM-File failed!" forKey:NSLocalizedDescriptionKey]];
            return nil;
        }
        
        if(useMD5)
        {
            NSString* hash = [data MD5HashString];
            [rom setValue:hash forKey:@"md5"];
        }
        else
        {
            NSString* hash = [data CRC32HashString];
            [rom setValue:hash forKey:@"crc32"];        
        }
    }
    
    return [rom autorelease];
}

+ (id)romWithFileName:(NSString*)filename error:(NSError**)outError
{
    return [self romWithFileName:filename inDatabase:[OELibraryDatabase defaultDatabase] error:outError];
}
+ (id)romWithFileName:(NSString*)filename inDatabase:(OELibraryDatabase*)database error:(NSError**)outError
{
    if(filename==nil) return nil;
    
    NSManagedObjectContext* context = [database managedObjectContext];
    
    NSPredicate* predicate = [NSPredicate predicateWithFormat:@"path ENDSWITH %@", filename];    
    NSFetchRequest* fetchRequest = [NSFetchRequest fetchRequestWithEntityName:[self entityName]];
    [fetchRequest setIncludesPendingChanges:YES];
    [fetchRequest setPredicate:predicate];
    [fetchRequest setFetchLimit:1];
    
    NSArray* roms = [context executeFetchRequest:fetchRequest error:outError];
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
    
    NSManagedObjectContext* context = [database managedObjectContext];
    NSPredicate* predicate = [NSPredicate predicateWithFormat:@"crc32 == %@", crcHash];    
    NSFetchRequest* fetchRequest = [NSFetchRequest fetchRequestWithEntityName:[self entityName]];
    [fetchRequest setFetchLimit:1];
    [fetchRequest setIncludesPendingChanges:YES];
    [fetchRequest setPredicate:predicate];
    
    NSArray* roms = [context executeFetchRequest:fetchRequest error:outError];
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
    
    NSManagedObjectContext* context = [database managedObjectContext];
    NSPredicate* predicate = [NSPredicate predicateWithFormat:@"md5 == %@", md5Hash];    
    NSFetchRequest* fetchRequest = [NSFetchRequest fetchRequestWithEntityName:[self entityName]];
    [fetchRequest setFetchLimit:1];
    [fetchRequest setIncludesPendingChanges:YES];
    [fetchRequest setPredicate:predicate];
    
    NSArray* roms = [context executeFetchRequest:fetchRequest error:outError];
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
    return [NSURL fileURLWithPath:[self valueForKey:@"path"]];
}

- (NSString*)md5Hash
{
    NSString* hash = [self valueForKey:@"md5"];
    if(!hash)
    {
        NSError* error = nil;
        NSString* filePath = [self valueForKey:@"path"];
        if(![[NSFileManager defaultManager] fileExistsAtPath:filePath])
        {
            // TODO: mark self as file missing
            return nil;
        }
        
        NSData* fileData = [NSData dataWithContentsOfFile:filePath options:NSDataReadingUncached error:&error];
        if(!fileData)
        {
            // TODO: mark self as file missing
        }
        hash = [fileData MD5HashString];
        [self setValue:hash forKey:@"md5"];
    }
    return hash;
}

- (NSString*)md5HashIfAvailable
{
    return [self valueForKey:@"md5"];
}

- (NSString*)crcHash
{
    NSString* hash = [self valueForKey:@"crc32"];
    if(!hash)
    {
        NSError* error = nil;
        NSString* filePath = [self valueForKey:@"path"];
        if(![[NSFileManager defaultManager] fileExistsAtPath:filePath])
        {
            // TODO: mark self as file missing
            return nil;
        }
        
        NSData* fileData = [NSData dataWithContentsOfFile:filePath options:NSDataReadingUncached error:&error];
        if(!fileData)
        {
            // TODO: mark self as file missing
        }
        hash = [fileData CRC32HashString];
        [self setValue:hash forKey:@"crc32"];
    }
    return hash;    
}

- (NSString*)crcHashIfAvailable
{
    return [self valueForKey:@"crc32"];
}


- (NSArray*)saveStatesByTimestampAscending:(BOOL)ascFlag
{
    NSSet* set = [self valueForKey:@"saveStates"];
    return [[set allObjects] sortedArrayUsingComparator:^NSComparisonResult(id obj1, id obj2) 
            {
                NSDate* d1 = [obj1 valueForKey:@"timestamp"], *d2=[obj2 valueForKey:@"timestamp"];
                if(ascFlag)
                    return [d2 compare:d1];
                return [d1 compare:d2];
            }];
}

- (NSInteger)saveStateCount
{
    return [[self valueForKey:@"saveStates"] count];
}
#pragma mark -
#pragma mark Mainpulating a rom
- (void)markAsPlayedNow
{
    [self setValue:[NSDate date] forKey:@"lastPlayed"];
}
#pragma mark -
#pragma mark Core Data utilities
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
#pragma mark -
#pragma mark -
#pragma mark -
#pragma mark -
#pragma mark -
#pragma mark -
#pragma mark -
#pragma mark -
#pragma mark -
#pragma mark -
#pragma mark -
#pragma mark -









#pragma mark -
- (void)doInitialSetupWithDatabase:(OELibraryDatabase*)db
{
    NSString* filePath = [self valueForKey:@"path"];
    
    DLog(@"doInitialSetupWithDatabase: %@", filePath);
    
    BOOL useMD5 = [[NSUserDefaults standardUserDefaults] boolForKey:UDUseMD5HashingKey];
    NSError* error = nil;
    NSData* data = [[NSData alloc] initWithContentsOfFile:filePath options:NSDataReadingUncached error:&error];
    if(!data)
    {
        NSLog(@"ROM cannot calculate checksum");
        NSLog(@"%@", error);
        [data release];
        return;    
    }
    
    NSString* hash;
    OEDBRom* rom;
    if(useMD5)
    {
        hash = [data MD5HashString];
        rom = [db romForMD5Hash:hash];
    }
    else 
    {
        hash = [data CRC32HashString];
        rom = [db romForCRC32Hash:hash];
    }
    [data release];
    
    
    if(rom)
    {
        NSSet* romsInGame = [self valueForKeyPath:@"game.roms"];
        if([romsInGame count]==1)
        {
            NSString* path = [self valueForKey:@"path"];
            if([path hasPrefix:[[NSUserDefaults standardUserDefaults] stringForKey:UDDatabasePathKey]])
            {
                [[NSFileManager defaultManager] removeItemAtPath:path error:nil];
            }
            
            [self.managedObjectContext deleteObject:[self valueForKey:@"game"]];
            [self.managedObjectContext deleteObject:self];
        }
    }
    
    [self setValue:hash forKey:useMD5?@"md5":@"crc32"];
    
    NSDictionary* gameDictionary;
    if(useMD5)
    {
        gameDictionary = [ArchiveVG gameInfoByMD5:hash];
    } 
    else
    {
        gameDictionary = [ArchiveVG gameInfoByCRC:hash];
    }
    
    OEDBGame* game = [db gameWithArchiveID:(NSNumber*)[gameDictionary valueForKey:(NSString*)AVGGameIDKey]];
    if(game)
    {
        NSLog(@"Game is aleady present");
        [self.managedObjectContext deleteObject:[self valueForKey:@"game"]];
        [[game mutableSetValueForKey:@"roms"] addObject:self];
    } 
    else
    {
        game = [self valueForKey:@"game"];
    }
    
    [game mergeWithGameInfo:gameDictionary];
    
    BOOL organize = [[NSUserDefaults standardUserDefaults] boolForKey:UDOrganizeLibraryKey];
    NSString* path = [self valueForKey:@"path"];
    NSString* libraryPath = [[NSUserDefaults standardUserDefaults] stringForKey:UDDatabasePathKey];
    if(organize && [path hasPrefix:libraryPath])
    {
        NSString* systemName = [self valueForKeyPath:@"game.system.systemIdentifier"];
        NSString* folderPath = [libraryPath stringByAppendingPathComponent:systemName];
        
        if(![[NSFileManager defaultManager] createDirectoryAtPath:folderPath withIntermediateDirectories:YES attributes:nil error:&error])
        {
            NSLog(@"could not create system folder");
            NSLog(@"%@", error);
            return;
        }
        
        NSString* romName = [[self valueForKey:@"path"] lastPathComponent];
        
        // TODO: use tosec for new rom path if available
        NSString* newRomPath = [folderPath stringByAppendingPathComponent:romName];
        if(![[NSFileManager defaultManager] moveItemAtPath:path toPath:newRomPath error:&error])
        {
            NSLog(@"could not move rom to new path");
            NSLog(@"%@", error);
            return;
        }
        
        [self setValue:newRomPath forKey:@"path"];
    }
    
}

@end
