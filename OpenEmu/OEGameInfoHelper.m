//
//  OEGameInfoHelper.m
//  OpenEmu
//
//  Created by Christoph Leimbrock on 07/12/13.
//
//

#import "OEGameInfoHelper.h"
#import "OESQLiteDatabase.h"
#import "NSURL+OELibraryAdditions.h"
#import "NSFileManager+OEHashingAdditions.h"

#import <OpenEmuSystem/OpenEmuSystem.h> // we only need OELocalizationHelper

@interface OEGameInfoHelper ()
@property OESQLiteDatabase *database;
@end
@implementation OEGameInfoHelper
+ (id)sharedHelper
{
    static OEGameInfoHelper *sharedHelper = nil;
    if(!sharedHelper)
    {
        sharedHelper = [[OEGameInfoHelper alloc] init];
    
        NSError *error = nil;
        NSURL   *url   = [NSURL fileURLWithPath:[@"~/Downloads/Database/db sample.sqlite" stringByExpandingTildeInPath]];
        OESQLiteDatabase *database = [[OESQLiteDatabase alloc] initWithURL:url error:&error];
        if(database)
        {
            [sharedHelper setDatabase:database];
        }
        else
        {
            [NSApp presentError:error];
        }
    };
    return sharedHelper;
}

- (NSDictionary*)gameInfoForROM:(OEDBRom*)rom error:(NSError *__autoreleasing*)error
{
    NSString *key, *value;
    
    int headerSize = [self sizeOfROMHeaderForSystem:[[rom game] system]];
    if(headerSize == 0) // rom has no header, that means we can use the hash we calculated at import
    {
        if((value = [rom md5HashIfAvailable]) != nil)
        {
            key = @"romHashMD5";
        }
        else if((value = [rom crcHashIfAvailable]) != nil)
        {
            key = @"romHashCRC";
        }
        /*
         else if((value = [rom sha1HashIfAvailable]) != nil)
         {
         key = @"romHashSHA1";
         }
         */
    }
    else
    {
        key = @"romHashMD5";
        if(![[NSFileManager defaultManager] hashFileAtURL:[rom URL] headerSize:headerSize md5:&value crc32:nil error:error])
            return nil;
    }
    
    /* SQL to get name, gameDescription, gameID and boxImageURL from local database:
     *
     *  SELECT DISTINCT releaseTitleName as 'name', releaseCoverFront as 'boxImageURL', gameDescription as 'gameDescription', gameInfoID as 'archiveID'
     *  FROM ROMs rom LEFT JOIN RELEASES release USING (romID) LEFT JOIN GAMESINFO info USING (gameInfoID) LEFT JOIN REGIONS region on (regionLocalizedID=region.regionID)
     *  WHERE key = 'value'
     */
    NSString *regionFilter = [NSString stringWithFormat:@" and region.regionName = '%@'", [[OELocalizationHelper sharedHelper] regionName]];
    NSString *sql = [NSString stringWithFormat:@"SELECT DISTINCT releaseTitleName as 'name', releaseCoverFront as 'boxImageURL', gameDescription as 'gameDescription', gameInfoID as 'archiveID'\
                     FROM ROMs rom LEFT JOIN RELEASES release USING (romID) LEFT JOIN GAMESINFO info USING (gameInfoID) LEFT JOIN REGIONS region on (regionLocalizedID=region.regionID)\
                     WHERE %@ = '%@'", key, [value uppercaseString]];
    NSArray *result = [[self database] executeQuery:[sql stringByAppendingString:regionFilter] error:error];
    if([result count] == 0)
    {
        result = [[self database] executeQuery:sql error:error];
    }
    return [result lastObject];
}

- (int)sizeOfROMHeaderForSystem:(OEDBSystem*)system
{
    // TODO: Read header size from database
    if([[system systemIdentifier] isEqualToString:@"openemu.system.nes"]) return 16;
    else return 0;
}
@end
