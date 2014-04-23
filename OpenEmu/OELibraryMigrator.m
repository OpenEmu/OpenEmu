/*
 Copyright (c) 2014, OpenEmu Team
 
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
        [self setStoreURL:storeURL];
    }
    return self;
}

#pragma mark -
- (BOOL)runMigration:(NSError *__autoreleasing *)outError
{
    NSBundle *bundle = [NSBundle mainBundle];
    NSDictionary *sourceMetadata = [NSPersistentStoreCoordinator metadataForPersistentStoreOfType:NSSQLiteStoreType
                                                                                              URL:_storeURL
                                                                                            error:outError];
    NSURL *modelURL = [[NSBundle mainBundle] URLForResource:@"OEDatabase" withExtension:@"momd"];
    NSManagedObjectModel *destinationModel = [[NSManagedObjectModel alloc] initWithContentsOfURL:modelURL];
    if(sourceMetadata == nil)
        return NO;

    if([destinationModel isConfiguration:nil compatibleWithStoreMetadata:sourceMetadata])
    {
        DLog(@"No migration needed");
        return YES;
    }

    NSManagedObjectModel *sourceModel = [NSManagedObjectModel mergedModelFromBundles:@[bundle] forStoreMetadata:sourceMetadata];
    if(sourceModel == nil)
    {
        DLog(@"Can't find souce model");
        return NO;
    }

    NSMappingModel *mappingModel = [NSMappingModel mappingModelFromBundles:@[bundle] forSourceModel:sourceModel destinationModel:destinationModel];
    if(mappingModel == nil)
    {
        DLog(@"No mapping model, try to infer one");
        mappingModel = [NSMappingModel inferredMappingModelForSourceModel:sourceModel destinationModel:destinationModel error:outError];
    }

    if(mappingModel == nil)
    {
        DLog(@"Still no mapping model");
        return NO;
    }

    // create urls for migration
    NSString *fileExtension = [_storeURL pathExtension];
    NSURL *temporaryDestination = [_storeURL URLByDeletingPathExtension];
    NSString *fileName = [temporaryDestination lastPathComponent];
    NSString *temporaryFileName = [NSString stringWithFormat:@"Temporary %@.%@", fileName, fileExtension];
    temporaryDestination = [[_storeURL URLByDeletingLastPathComponent] URLByAppendingPathComponent:temporaryFileName];

    NSString *previousStoreFileName = [NSString stringWithFormat:@"Previous %@.%@", fileName, fileExtension];
    NSURL *previousStoreURL = [[_storeURL URLByDeletingLastPathComponent] URLByAppendingPathComponent:previousStoreFileName];

    NSFileManager *fileManager = [NSFileManager defaultManager];
    [fileManager removeItemAtURL:temporaryDestination error:nil];

    NSMigrationManager *manager = [[NSMigrationManager alloc] initWithSourceModel:sourceModel destinationModel:destinationModel];

    OEMigrationWindowController *windowController = [[OEMigrationWindowController alloc] initWithMigrationManager:manager];
    dispatch_sync(dispatch_get_main_queue(), ^{
        [windowController showWindow:self];
        [[windowController indicator] setIndeterminate:YES];
        [[windowController indicator] startAnimation:self];
    });

    BOOL(^cleanup)(BOOL) = ^(BOOL success){
        dispatch_sync(dispatch_get_main_queue(), ^{
            [fileManager removeItemAtURL:temporaryDestination error:nil];
            [windowController close];
        });
        return success;
    };

    BOOL success = [manager migrateStoreFromURL:_storeURL type:NSSQLiteStoreType options:@{} withMappingModel:mappingModel toDestinationURL:temporaryDestination destinationType:NSSQLiteStoreType destinationOptions:@{} error:outError];
    if(success == NO)
    {
        DLog(@"Migration failed!");
        return cleanup(NO);
    }

    [fileManager removeItemAtURL:previousStoreURL error:nil];
    if(![fileManager moveItemAtURL:_storeURL toURL:previousStoreURL error:outError])
    {
        DLog(@"Moving to previous library location failed");
        return cleanup(NO);
    }
    [fileManager removeItemAtURL:_storeURL error:nil];
    if(![fileManager moveItemAtURL:temporaryDestination toURL:_storeURL error:outError])
    {
        DLog(@"Moving to default library location failed");
        return cleanup(NO);
    }

    DLog(@"Migration Done");
    return cleanup(YES);
}

@end
