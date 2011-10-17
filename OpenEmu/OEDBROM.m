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
@implementation OEDBRom

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
#pragma mark -
- (void)doInitialSetupWithDatabase:(OELibraryDatabase*)db{
    NSString* filePath = [self valueForKey:@"path"];
    NSLog(@"calculateChecksum: %@", filePath);
    
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
    
    
    if(rom){
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
    
    OEDBGame* game = [db gameWithArchiveID:[gameDictionary valueForKey:AVGGameIDKey]];
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
- (BOOL)calculateChecksumInDatabase:(OELibraryDatabase*)db
{
}

- (void)doArchiveSyncInDatabase:(OELibraryDatabase*)db
{
}

- (void)organizeInLibraryOfDatabase:(OELibraryDatabase *)db
{    
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

@end
