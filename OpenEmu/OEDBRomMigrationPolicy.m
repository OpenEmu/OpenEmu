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

#import "OEDBRomMigrationPolicy.h"
#import <XADMaster/XADArchive.h>
#import "OELibraryDatabase.h"
#import "NSArray+OEAdditions.h"

#import "OpenEmu-Swift.h"

NS_ASSUME_NONNULL_BEGIN

extern NSString *const OELibraryRomsFolderURLKey;

@implementation OEDBRomMigrationPolicy

- (BOOL)createDestinationInstancesForSourceInstance:(NSManagedObject *)oldObject entityMapping:(NSEntityMapping *)mapping manager:(NSMigrationManager *)manager error:(NSError **)error
{
    NSAssert(manager.sourceModel.versionIdentifiers.count == 1, @"Found a source model with various versionIdentifiers!");
    
    NSString *version = manager.sourceModel.versionIdentifiers. anyObject;

    if([version isEqualTo:@"1.0 Beta"])
    {
        NSData *bookmarkData = [oldObject valueForKey:@"bookmarkData"];
        NSURL *url = [NSURL URLByResolvingBookmarkData:bookmarkData options:0 relativeToURL:nil bookmarkDataIsStale:nil error:nil];
        NSString *location = url.absoluteString;
        if(location)
        {
            NSArray<NSPropertyMapping *> *attributeMappings = mapping.attributeMappings;
            NSPropertyMapping *mapping = [attributeMappings firstObjectMatchingBlock:
             ^ BOOL (NSPropertyMapping *obj)
             {
                 return [[obj name] isEqualToString:@"location"];
             }];
            [mapping setValueExpression:[NSExpression expressionForConstantValue:location]];
        }
    }
    else if([version isEqualTo:@"1.0 Beta 4"]) // Migrate from single rom archives to multi rom archives
    {
        NSPersistentStoreCoordinator *coord = oldObject.managedObjectContext.persistentStoreCoordinator;
        NSURL *romsFolder = [self romsFolderURLWithPersistentStoreCoordinator:coord];

        NSString *relativePath = [oldObject valueForKey:@"location"];
        NSURL *romURL = [NSURL URLWithString:relativePath relativeToURL:romsFolder];
        NSString *fullPath = romURL.path;
        XADArchive *archive = nil;
        @try
        {
            archive = [XADArchive archiveForFile:fullPath];
        }
        @catch (NSException *exc)
        {
            archive = nil;
        }
        
        if(archive && archive.numberOfEntries == 1)
        {
            NSArray<NSPropertyMapping *> *attributeMappings = mapping.attributeMappings;
            NSPropertyMapping *mapping = [attributeMappings firstObjectMatchingBlock:
                                          ^ BOOL (NSPropertyMapping *obj)
                                          {
                                              return [[obj name] isEqualToString:@"archiveFileIndex"];
                                          }];
            mapping.valueExpression = [NSExpression expressionForConstantValue:@(0)];
        }
        else
        {
            NSArray<NSPropertyMapping *> *attributeMappings = mapping.attributeMappings;
            NSPropertyMapping *mapping = [attributeMappings firstObjectMatchingBlock:
                                          ^ BOOL (NSPropertyMapping *obj)
                                          {
                                              return [[obj name] isEqualToString:@"archiveFileIndex"];
                                          }];
            mapping.valueExpression = [NSExpression expressionForConstantValue:nil];

        }
    }
    else if([version isEqualToString:@"1.2"])
    {
        NSPersistentStoreCoordinator *coord = oldObject.managedObjectContext.persistentStoreCoordinator;
        NSURL *romsFolderURL = [self romsFolderURLWithPersistentStoreCoordinator:coord];
        NSString *urlString = [oldObject valueForKey:@"location"];
        NSURL *url = nil;
        if(![urlString containsString:@"file://"])
             url = [NSURL URLWithString:urlString relativeToURL:romsFolderURL];
        else
            url = [NSURL URLWithString:urlString];

        NSURL *relativeURL = [url URLRelativeToURL:romsFolderURL];
        NSString *location = relativeURL.relativeString;
        if(location)
        {
            NSArray<NSPropertyMapping *> *attributeMappings = mapping.attributeMappings;
            NSPropertyMapping *mapping = [attributeMappings firstObjectMatchingBlock:
                                          ^ BOOL (NSPropertyMapping *obj)
                                          {
                                              return [[obj name] isEqualToString:@"location"];
                                          }];
            mapping.valueExpression = [NSExpression expressionForConstantValue:location];
        }
    }

    return [super createDestinationInstancesForSourceInstance:oldObject entityMapping:mapping manager:manager error:error];
}

- (NSURL *)romsFolderURLWithPersistentStoreCoordinator:(NSPersistentStoreCoordinator*)coord
{
    NSPersistentStore *persistentStore = coord.persistentStores.lastObject;
    NSDictionary *metadata = [coord metadataForPersistentStore:persistentStore];

    NSURL *databaseFolderURL = persistentStore.URL.URLByDeletingLastPathComponent;
    NSURL *result = nil;
    if(metadata[OELibraryRomsFolderURLKey])
    {
        NSString *urlString = metadata[OELibraryRomsFolderURLKey];
        if(![urlString containsString:@"file://"])
             result = [NSURL URLWithString:urlString relativeToURL:databaseFolderURL];
        else result = [NSURL URLWithString:urlString];
    }
    else
    {
        result = [databaseFolderURL URLByAppendingPathComponent:@"roms" isDirectory:YES];
    }

    return result.standardizedURL;
}

@end

NS_ASSUME_NONNULL_END
