//
//  OEDBImageMigrationPolicy.m
//  OpenEmu
//
//  Created by Christoph Leimbrock on 21/04/14.
//
//

#import "OEDBImageMigrationPolicy.h"
#import "NSArray+OEAdditions.h"
#import "OELibraryDatabase.h"

NSString * const OEDBImageMigrateImageFormat = @"OEDBImageMigrateImageFormat";

@implementation OEDBImageMigrationPolicy

- (BOOL)createDestinationInstancesForSourceInstance:(NSManagedObject *)oldObject entityMapping:(NSEntityMapping *)mapping manager:(NSMigrationManager *)manager error:(NSError **)error
{
    NSAssert([[[manager sourceModel] versionIdentifiers] count]==1, @"Found a source model with various versionIdentifiers!");

    NSString *sourceVersion = [[[manager sourceModel] versionIdentifiers] anyObject];
    NSDictionary  *entities = [[manager sourceModel] entitiesByName];

    // Version 1.1 and 1.0 both share version identifier 1.0 :/
    if([sourceVersion isEqualTo:@"1.0"] && [entities objectForKey:@"ImageThumbnail"] != nil)
    {
        [[NSUserDefaults standardUserDefaults] setBool:YES forKey:OEDBImageMigrateImageFormat];

        NSSet *versions = [oldObject valueForKey:@"versions"];
        __block CGFloat size = 0;
        __block NSManagedObject *originalVersion = nil;
        [versions enumerateObjectsUsingBlock:^(NSManagedObject *aVersion, BOOL *stop) {
            CGFloat currentSize = [[aVersion valueForKey:@"width"] floatValue];
            if(currentSize > size)
            {
                originalVersion = aVersion;
                size = currentSize;
            }
        }];

        if(originalVersion)
        {
            NSURL *coverFolderURL = [self coverFolderURL];

            id width  = [originalVersion valueForKey:@"width"];
            id height = [originalVersion valueForKey:@"height"];
            id path   = [originalVersion valueForKey:@"relativePath"];
            id source = [originalVersion valueForKey:@"sourceURL"];

            [versions enumerateObjectsUsingBlock:^(id obj, BOOL *stop) {
                NSString *aPath = [obj valueForKey:@"relativePath"];
                if([aPath isNotEqualTo:path])
                {
                    NSURL *imageURL = [NSURL URLWithString:aPath relativeToURL:coverFolderURL];
                    [[NSFileManager defaultManager] removeItemAtURL:imageURL error:nil];
                }
            }];

            NSManagedObject *newObject = [NSEntityDescription insertNewObjectForEntityForName:@"Image" inManagedObjectContext:[manager destinationContext]];

            [newObject setValue:width  forKey:@"width"];
            [newObject setValue:height forKey:@"height"];
            [newObject setValue:path   forKey:@"relativePath"];
            [newObject setValue:source   forKey:@"relativePath"];

            [manager associateSourceInstance:oldObject withDestinationInstance:newObject forEntityMapping:mapping];

            return YES;
        }
    }

    return NO;
}

- (void)updateMapping:(NSEntityMapping*)mapping setValue:(id)value forKey:(NSString*)key
{
    NSArray *attributeMappings = [mapping attributeMappings];
    NSPropertyMapping *propertyMapping = [attributeMappings firstObjectMatchingBlock:
                                  ^ BOOL (id obj)
                                  {
                                      return [[obj name] isEqualToString:key];
                                  }];
    [propertyMapping setValueExpression:[NSExpression expressionForConstantValue:value]];
}

- (NSURL*)coverFolderURL
{
    NSUserDefaults *standardDefaults  = [NSUserDefaults standardUserDefaults];
    NSString       *libraryFolderPath = [[standardDefaults stringForKey:OEDatabasePathKey] stringByExpandingTildeInPath];
    NSString       *coverFolderPath   = [libraryFolderPath stringByAppendingPathComponent:@"Artwork/"];

    NSURL *baseURL = [NSURL fileURLWithPath:coverFolderPath isDirectory:YES];
    return baseURL;
}
@end
