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
        NSURL   *url   = [[NSBundle mainBundle] URLForResource:@"db sample" withExtension:@"sqlite"];
        NSURL *applicationSupport = [[NSFileManager defaultManager] URLForDirectory:NSApplicationSupportDirectory inDomain:NSUserDomainMask appropriateForURL:nil create:YES error:nil];
        NSURL *alternativeDBUrl = [applicationSupport URLByAppendingPathComponent:@"OpenEmu/db sample.sqlite"];
        if([alternativeDBUrl checkResourceIsReachableAndReturnError:nil])
            url = alternativeDBUrl;
        
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

    NSString *sql = [NSString stringWithFormat:@"SELECT DISTINCT releaseTitleName as 'name', releaseCoverFront as 'boxImageURL', releaseDescription as 'gameDescription'\
                     FROM ROMs rom LEFT JOIN RELEASES release USING (romID) LEFT JOIN REGIONS region on (regionLocalizedID=region.regionID)\
                     WHERE %@ = '%@'", key, [value uppercaseString]];

    NSArray *result = [[self database] executeQuery:sql error:error];
    return [result lastObject];
}

- (int)sizeOfROMHeaderForSystem:(OEDBSystem*)system
{
    NSString *sql = [NSString stringWithFormat:@"select systemheadersizebytes as 'size' from systems where systemoeid = '%@'", [system systemIdentifier]];
    NSArray *result = [[self database] executeQuery:sql error:nil];
    return [[[result lastObject] objectForKey:@"size"] intValue];
}
@end
