/*
 Copyright (c) 2015, OpenEmu Team
 
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

#import "OELibraryMigrator.h"
#import "OEMigrationWindowController.h"
#import "OELibraryDatabase.h"
#import "OEDBScreenshot.h"
#import "ALIterativeMigrator.h"

#import "OpenEmu-Swift.h"

NS_ASSUME_NONNULL_BEGIN

NSString *const OEMigrationErrorDomain = @"OEMigrationErrorDomain";

@interface OELibraryMigrator ()
@property NSURL *storeURL;
@end

@implementation OELibraryMigrator

- (id)initWithStoreURL:(NSURL *)url
{
    self = [super init];
    if (self) {
        NSURL *storeURL = [url URLByAppendingPathComponent:OEDatabaseFileName];
        _storeURL = storeURL;
    }
    return self;
}

#pragma mark -

- (BOOL)runMigration:(NSError **)outError
{
    NSURL *modelURL = [[NSBundle mainBundle] URLForResource:@"OEDatabase" withExtension:@"momd"];
    NSManagedObjectModel *destinationModel = [[NSManagedObjectModel alloc] initWithContentsOfURL:modelURL];

    NSArray* modelNames = @[@"OEDatabase",
                            @"OEDatabase 0.2",
                            @"OEDatabase 0.3",
                            @"OEDatabase 0.4",
                            @"OEDatabase 0.5",
                            @"OEDatabase 1.0",
                            @"OEDatabase 1.1",
                            @"OEDatabase 1.2",
                            @"OEDatabase 1.3"];
    if (![ALIterativeMigrator iterativeMigrateURL:_storeURL ofType:NSSQLiteStoreType toModel:destinationModel orderedModelNames:modelNames error:outError])
    {
        if(outError)
            DLog(@"Error migrating to latest model: %@\n %@", *outError, [*outError userInfo]);

        return NO;
    }

    NSDictionary *sourceMetadata = [NSPersistentStoreCoordinator metadataForPersistentStoreOfType:NSSQLiteStoreType URL:_storeURL options:nil error:outError];
    NSArray *versions = sourceMetadata[NSStoreModelVersionIdentifiersKey];
    versions = [versions sortedArrayUsingSelector:@selector(caseInsensitiveCompare:)];

    NSString *sourceVersion = versions.lastObject;
    if([sourceVersion compare:@"1.3"] == NSOrderedAscending)
    {
        [[NSUserDefaults standardUserDefaults] setBool:YES forKey:OEDBScreenshotImportRequired];
    }

    DLog(@"Migration Done");
    return YES;
}

@end

NS_ASSUME_NONNULL_END
