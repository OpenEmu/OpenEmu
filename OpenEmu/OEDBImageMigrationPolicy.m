//
//  OEDBImageMigrationPolicy.m
//  OpenEmu
//
//  Created by Christoph Leimbrock on 21/04/14.
//
//

#import "OEDBImageMigrationPolicy.h"
#import "NSArray+OEAdditions.h"
@implementation OEDBImageMigrationPolicy
- (BOOL)createDestinationInstancesForSourceInstance:(NSManagedObject *)oldObject entityMapping:(NSEntityMapping *)mapping manager:(NSMigrationManager *)manager error:(NSError **)error
{
    NSAssert([[[manager sourceModel] versionIdentifiers] count]==1, @"Found a source model with various versionIdentifiers!");

    NSString *sourceVersion = [[[manager sourceModel] versionIdentifiers] anyObject];
    NSDictionary  *entities = [[manager sourceModel] entitiesByName];
    // Version 1.1 and 1.0 both share version identifier 1.0 :/
    if([sourceVersion isEqualTo:@"1.0"] && [entities objectForKey:@"ImageThumbnail"] != nil)
    {
        NSSet *versions = [oldObject valueForKey:@"versions"];
        __block CGFloat size = 0;
        __block NSManagedObject *originalVersion = nil;
        [versions enumerateObjectsUsingBlock:^(NSManagedObject *aVersion, BOOL *stop) {
            CGFloat currentSize = [[aVersion valueForKey:@"width"] floatValue] * [[aVersion valueForKey:@"height"] floatValue];

            if(currentSize > size)
            {
                originalVersion = aVersion;
                currentSize = size;
            }
        }];

        if(originalVersion)
        {
            id width  = [originalVersion valueForKey:@"width"];
            id height = [originalVersion valueForKey:@"height"];
            id path   = [originalVersion valueForKey:@"relativePath"];

            [self updateMapping:mapping setValue:width  forKey:@"width"];
            [self updateMapping:mapping setValue:height forKey:@"height"];
            [self updateMapping:mapping setValue:path   forKey:@"relativePath"];
        }
    }

    return [super createDestinationInstancesForSourceInstance:oldObject entityMapping:mapping manager:manager error:error];
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
@end
