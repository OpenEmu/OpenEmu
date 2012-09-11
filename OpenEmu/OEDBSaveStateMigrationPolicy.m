//
//  OEDBSaveStateMigrationPolicy.m
//  OpenEmu
//
//  Created by Christoph Leimbrock on 11.09.12.
//
//

#import "OEDBSaveStateMigrationPolicy.h"
#import "NSArray+OEAdditions.h"
@implementation OEDBSaveStateMigrationPolicy

- (BOOL)createDestinationInstancesForSourceInstance:(NSManagedObject *)oldObject entityMapping:(NSEntityMapping *)mapping manager:(NSMigrationManager *)manager error:(NSError **)error
{
    NSAssert([[[manager sourceModel] versionIdentifiers] count]==1, @"Found a source model with various versionIdentifiers!");
    
    NSString *version = [[[manager sourceModel] versionIdentifiers] anyObject];
    
    if([version isEqualTo:@"1.0 Beta"])
    {
        NSString *path     = [oldObject valueForKey:@"path"];
        NSURL    *url      = [NSURL URLWithString:path];
        NSString *location = [url absoluteString];
        if(location)
        {
            NSArray *attributeMappings = [mapping attributeMappings];
            NSPropertyMapping *mapping = [attributeMappings firstObjectMatchingBlock:
                                          ^ BOOL (id obj)
                                          {
                                              return [[obj name] isEqualToString:@"location"];
                                          }];
            [mapping setValueExpression:[NSExpression expressionForConstantValue:location]];
        }
    }
    return [super createDestinationInstancesForSourceInstance:oldObject entityMapping:mapping manager:manager error:error];
}

@end
