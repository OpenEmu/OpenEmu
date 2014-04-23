//
//  OELibraryMigrator.m
//  OpenEmu
//
//  Created by Christoph Leimbrock on 23/04/14.
//
//

#import "OELibraryMigrator.h"
#import "OEMigrationWindowController.h"
#import "OELibraryDatabase.h"
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

    NSFileManager *fileManager = [NSFileManager defaultManager];
    [fileManager removeItemAtURL:temporaryDestination error:nil];

    NSMigrationManager *manager = [[NSMigrationManager alloc] initWithSourceModel:sourceModel destinationModel:destinationModel];

    OEMigrationWindowController *windowController = [[OEMigrationWindowController alloc] initWithMigrationManager:manager];
    dispatch_sync(dispatch_get_main_queue(), ^{
        [windowController showWindow:self];
        [[windowController indicator] setIndeterminate:YES];
        [[windowController indicator] startAnimation:self];
    });


    BOOL success = [manager migrateStoreFromURL:_storeURL type:NSSQLiteStoreType options:@{} withMappingModel:mappingModel toDestinationURL:temporaryDestination destinationType:NSSQLiteStoreType destinationOptions:@{} error:outError];
    if(success == NO)
    {
        DLog(@"Migration failed!");
        return NO;
    }

    NSString *previousStoreFileName = [NSString stringWithFormat:@"Previous %@.%@", fileName, fileExtension];
    NSURL *previousStoreURL = [[_storeURL URLByDeletingLastPathComponent] URLByAppendingPathComponent:previousStoreFileName];
    [fileManager removeItemAtURL:previousStoreURL error:nil];
    if(![fileManager moveItemAtURL:_storeURL toURL:previousStoreURL error:outError])
    {
        DLog(@"Moving to previous library location failed");
        return NO;
    }
    [fileManager removeItemAtURL:_storeURL error:nil];
    if(![fileManager moveItemAtURL:temporaryDestination toURL:_storeURL error:outError])
    {
        DLog(@"Moving to default library location failed");
        return NO;
    }

    DLog(@"Migration Done");
    dispatch_sync(dispatch_get_main_queue(), ^{
        [windowController close];
    });
    return YES;
}

@end
