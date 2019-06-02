/*
 Copyright (c) 2009, OpenEmu Team
 
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

#import "OEVersionMigrationController.h"
#import <Sparkle/SUStandardVersionComparator.h>
#import <objc/message.h>

#import "OEDBImageMigrationPolicy.h"
#import "OELibraryDatabase.h"
#import "OEDBGame.h"
#import "OEDBImage.h"
#import "OEDBRom.h"
#import "OEDBScreenshot+CoreDataProperties.h"

#import "OpenEmu-Swift.h"

@interface _OEMigrator : NSObject
{
    id __unsafe_unretained target;
    SEL action;
    
    BOOL hasRun;
}

@property(nonatomic, unsafe_unretained) id target;
@property(nonatomic, assign) SEL action;
@property(nonatomic, assign) BOOL hasRun;

- (BOOL)runWithError:(NSError **)err;

@end

@implementation _OEMigrator

@synthesize target, action, hasRun;

- (BOOL)runWithError:(NSError **)err
{
    if(hasRun) return YES;
    
    hasRun = YES;
    
    return ((BOOL(*)(id, SEL, NSError **))objc_msgSend)(target, action, err);
}

@end

#pragma mark -
#pragma mark Implementation

@implementation OEVersionMigrationController

@synthesize versionComparator;

static OEVersionMigrationController *sDefaultMigrationController = nil;

+ (id)defaultMigrationController
{
    if(!sDefaultMigrationController)
        sDefaultMigrationController = [[OEVersionMigrationController alloc] init];
    
    return sDefaultMigrationController;
}

- (id)init
{
    self = [super init];
    if(self != nil)
    {
        DLog();
        [self setVersionComparator:[SUStandardVersionComparator defaultComparator]];
        
        // We'll cheat here and rely on Sparkle's key
        isFirstRun  = ![[NSUserDefaults standardUserDefaults] boolForKey:@"SUHasLaunchedBefore"];
        lastVersion = [[[NSUserDefaults standardUserDefaults] objectForKey:@"OEMigrationLastVersion"] copy];
    }
    return self;
}

- (void)dealloc
{
    migrators = nil;
    lastVersion = nil;
}

- (void)runMigrationIfNeeded
{
    NSString *currentVersion = [[[NSBundle mainBundle] infoDictionary] objectForKey:(NSString *)kCFBundleVersionKey];
    NSString  *mostRecentVersion = lastVersion;
    NSUserDefaults *userDefaults = [NSUserDefaults standardUserDefaults];

    // Start thread to convert old images to new format
    if([userDefaults boolForKey:OEDBImageMigrateImageFormat])
    {
        [self OE_runImageMigration];
    }

    if([userDefaults boolForKey:OEDBScreenshotImportRequired])
    {
        [self OE_importScreenshots];
    }

    // We have to work around the fact that older versions of OpenEmu didn't stick a version key in the plist.
    // If the Sparkle key for an existing launch doesn't exist, then this is a new installation, not an upgrade.
    // Thus, we log our current version and prevent the migration. Subsequent migrations will then have the new version.
    if(isFirstRun && mostRecentVersion == nil)
    {
        [userDefaults setObject:currentVersion forKey:@"OEMigrationLastVersion"];
        return;
    }
    
    // if it's not the first run, and there's no most recent version, then it's a pre-1.0.0b5 upgrade
    // if the current version has a higher version than the most recent version, then it's an upgrade
    if(mostRecentVersion == nil || [[self versionComparator] compareVersion:mostRecentVersion toVersion:currentVersion] == NSOrderedAscending)
        [self migrateFromVersion:mostRecentVersion toVersion:currentVersion error:nil];
}

- (void)OE_runImageMigration
{
    dispatch_queue_t queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_BACKGROUND, 0);
    void (^block)(void) = ^{
        NSUserDefaults *userDefaults = [NSUserDefaults standardUserDefaults];
        OEBitmapImageFileType format = [userDefaults integerForKey:OEGameArtworkFormatKey];
        NSDictionary     *attributes = [userDefaults dictionaryForKey:OEGameArtworkPropertiesKey];

        OELibraryDatabase *database = [OELibraryDatabase defaultDatabase];
        NSFetchRequest    *request  = [NSFetchRequest fetchRequestWithEntityName:[OEDBImage entityName]];
        [request setFetchLimit:1];
        NSPredicate     *predicate  = [NSPredicate predicateWithFormat:@"format = -1"];
        [request setPredicate:predicate];

        NSManagedObjectContext *context = [database mainThreadContext];
        NSArray *images = [context executeFetchRequest:request error:nil];
        if(images != nil && [images count] == 0)
        {
            NSURL *coverFolderURL = [database coverFolderURL];

            NSArray *boxSizes = @[@"original", @"75", @"150", @"300", @"450"];
            [boxSizes enumerateObjectsUsingBlock:^(id boxSize, NSUInteger idx, BOOL *stop) {
                NSURL *url = [coverFolderURL URLByAppendingPathComponent:boxSize isDirectory:YES];

                NSArray *contents = [[NSFileManager defaultManager] contentsOfDirectoryAtURL:url includingPropertiesForKeys:@[] options:NSDirectoryEnumerationSkipsHiddenFiles error:nil];
                if([contents count] == 0)
                    [[NSFileManager defaultManager] removeItemAtURL:url error:nil];
            }];
            [userDefaults removeObjectForKey:@"BoxSizes"];
            [userDefaults removeObjectForKey:OEDBImageMigrateImageFormat];
        }
        else
        {
            [request setFetchLimit:10];
            __block NSUInteger count = 0;
            [context performBlockAndWait:^{
                count = [context countForFetchRequest:request error:nil];
            }];
            while(count != 0)
            {
                [context performBlockAndWait:^{
                    NSArray *images = [context executeFetchRequest:request error:nil];
                    [images enumerateObjectsUsingBlock:^(OEDBImage *image, NSUInteger idx, BOOL *stop) {
                        if(![image convertToFormat:format withProperties:attributes])
                        {
                            DLog(@"Failed to migrate image! Delete...");
                            [[image managedObjectContext] deleteObject:image];
                        }
                    }];
                    [context save:nil];
                }];

                [NSThread sleepForTimeInterval:1.0];

                [context performBlockAndWait:^{
                    count = [context countForFetchRequest:request error:nil];
                }];
            }
        }
    };
    dispatch_async(queue, block);
}

- (void)OE_importScreenshots
{
    OELibraryDatabase *database = [OELibraryDatabase defaultDatabase];

    NSURL *screenshotFolderURL = [database screenshotFolderURL];
    [self OE_importScreenShotsFromDirectory:screenshotFolderURL];

    [[database mainThreadContext] save:nil];

    [[NSUserDefaults standardUserDefaults] removeObjectForKey:OEDBScreenshotImportRequired];
}

- (void)OE_importScreenShotsFromDirectory:(NSURL*)directory
{
    NSFileManager *fm = [NSFileManager defaultManager];
    NSArray    *files = [fm contentsOfDirectoryAtURL:directory includingPropertiesForKeys:@[NSURLIsDirectoryKey] options:0 error:nil];

    NSDateFormatter *formatter = [[NSDateFormatter alloc] init];
    [formatter setDateFormat:@"yyyy-MM-dd HH.mm.ss"];

    for(NSURL *url in files)
    {
        if([url isDirectory])
        {
            [self OE_importScreenShotsFromDirectory:url];
        }
        else
        {
            NSManagedObjectContext *context = [[OELibraryDatabase defaultDatabase] mainThreadContext];

            NSString *file = [[url lastPathComponent] stringByDeletingPathExtension];
            NSString *extension = [url pathExtension];

            const NSUInteger dateLength = 19;
            if([file length] <= dateLength+1 || ![[extension lowercaseString] isEqualToString:@"png"])
                continue;

            NSString *dateString = [file substringFromIndex:[file length]-dateLength];
            NSDate    *timestamp = [formatter dateFromString:dateString];
            NSString   *gameName = [file substringToIndex:[file length]-dateLength-1];


            NSFetchRequest *request = [NSFetchRequest fetchRequestWithEntityName:[OEDBRom entityName]];

            NSArray *predicates = @[ [NSPredicate predicateWithFormat:@"game.name contains[cd] %@", gameName],
                                     [NSPredicate predicateWithFormat:@"game.gameTitle contains[cd] %@", gameName]];
            [request setPredicate:[NSCompoundPredicate orPredicateWithSubpredicates:predicates]];

            NSArray *fetchResult = [context executeFetchRequest:request error:nil];
            if([fetchResult count] == 0) continue;

            OEDBRom *rom = [fetchResult lastObject];

            OEDBScreenshot *screenShot = [OEDBScreenshot createObjectInContext:context];
            [screenShot setName:@"Screenshot"];
            [screenShot setTimestamp:timestamp];
            [screenShot setLocation:[url absoluteString]];
            [screenShot setRom:rom];
        }
    }
}

#pragma mark -

- (BOOL)migrateFromVersion:(NSString *)mostRecentVersion toVersion:(NSString *)currentVersion error:(NSError **)err
{
    if(mostRecentVersion == nil) mostRecentVersion = @"0.0.1";
    
    BOOL hasFailed = NO;
    
    NSMutableArray *errors = (err == nil ? nil : [NSMutableArray array]);
    NSArray *allVersions = [self allMigrationVersions];
    
    for(NSString *migratorVersion in allVersions)
    {
        // @"1.0.0b4", @"1.0.0b5", @"1.0.0"
        // Don't need to migrate, migrator is too young and has already run
        
        if([self.versionComparator compareVersion:mostRecentVersion toVersion:migratorVersion] != NSOrderedDescending ||
           [self.versionComparator compareVersion:currentVersion    toVersion:migratorVersion] != NSOrderedAscending)
        {
            NSArray *allMigrators = [migrators objectForKey:migratorVersion];
            NSLog(@"Running migrators from %@ to %@",mostRecentVersion, migratorVersion);
            for(_OEMigrator *migrator in allMigrators)
            {
                NSError *error = nil;
                if(![migrator runWithError:&error])
                {
                    [errors addObject:error];
                    hasFailed = YES;
                }
            }
        }
    }
    
    [[NSUserDefaults standardUserDefaults] setObject:currentVersion forKey:@"OEMigrationLastVersion"];    
    
    isRunning = YES;
    
    if([errors count] > 0 && err != NULL)
    {
        hasFailed = YES;
        if([errors count] == 1)
            *err = [errors objectAtIndex:0];
        else
            *err = [NSError errorWithDomain:OEVersionMigrationErrorDomain
                                       code:1 
                                   userInfo:@{
                                              OEVersionMigrationFailureErrorsKey : errors,
                                              NSLocalizedDescriptionKey : NSLocalizedString(@"Some migrations failed to complete properly", @"")
                                              }];
        
        NSLog(@"Error migrating! %@,", *err);
    }
    
    return hasFailed;
}

- (NSArray *)allMigrationVersions
{
    NSArray *allVersions = [[migrators allKeys] sortedArrayUsingComparator:
                            ^ NSComparisonResult (id oldVersion, id newVersion)
                            {
                                return [[self versionComparator] compareVersion:oldVersion toVersion:newVersion];
                            }];
    
    if(!allVersions) allVersions = [NSArray array];
    return allVersions;
}

- (void)addMigratorTarget:(id)target selector:(SEL)selector forVersion:(NSString *)version
{
    if(migrators == nil) migrators = [NSMutableDictionary dictionary];
    
    NSArray *allVersions = [self allMigrationVersions];
    
    NSMutableArray *migratorsForVersion = nil;
    
    // TODO: make this not O(n)
    BOOL breakOut = NO;
    for(NSString *migratorArrayKey in allVersions)
    { 
        switch([[self versionComparator] compareVersion:version toVersion:migratorArrayKey])
        {
            case NSOrderedSame :
                migratorsForVersion = [migrators objectForKey:migratorArrayKey];
            case NSOrderedAscending :
                //we have passed the version, so no migrators exist.
                breakOut = YES;
                break;
            default : break;
        }
        
        if(breakOut) break;
    }
    
    if(migratorsForVersion == nil)
    {
        migratorsForVersion = [NSMutableArray array];
        [migrators setObject:migratorsForVersion forKey:version];
    }
    
    _OEMigrator *migratorContainer = [[_OEMigrator alloc] init];
    migratorContainer.target = target;
    migratorContainer.action = selector;
    
    [migratorsForVersion addObject:migratorContainer];
    
    if(isRunning) [self runMigrationIfNeeded];
}

@end
