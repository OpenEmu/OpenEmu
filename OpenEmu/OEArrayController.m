//
//  OEArrayController.m
//  OpenEmu
//
//  Created by Christoph Leimbrock on 23/12/13.
//
//

#import "OEArrayController.h"

@implementation OEArrayController
- (BOOL)fetchWithRequest:(NSFetchRequest *)fetchRequest merge:(BOOL)merge error:(NSError *__autoreleasing *)error
{
    if(fetchRequest == nil)
    {
        fetchRequest = [[NSFetchRequest alloc] init];
        [fetchRequest setEntity:[NSEntityDescription entityForName:[self entityName] inManagedObjectContext:[self managedObjectContext]]];
        [fetchRequest setPredicate:[self fetchPredicate]];
    }
    if([[self fetchSortDescriptors] count])
        [fetchRequest setSortDescriptors:[self fetchSortDescriptors]];
    if([self limit] > 0)
        [fetchRequest setFetchLimit:self.limit];
    else
        [fetchRequest setFetchLimit:0];
    
    return [super fetchWithRequest:fetchRequest merge:merge error:error];
}

@end
