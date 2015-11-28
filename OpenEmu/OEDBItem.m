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

#import "OEDBItem.h"
#import "OELibraryDatabase.h"

#import "OpenEmu-Swift.h"

NS_ASSUME_NONNULL_BEGIN

@implementation OEDBItem

- (OELibraryDatabase *)libraryDatabase
{
    return self.managedObjectContext.libraryDatabase;
}

+ (instancetype)createObjectInContext:(NSManagedObjectContext *)context
{
    return [NSEntityDescription insertNewObjectForEntityForName:[self entityName] inManagedObjectContext:context];
}

+ (NSArray <__kindof OEDBItem *> *)allObjectsInContext:(NSManagedObjectContext *)context
{
    return [self allObjectsInContext:context error:nil];
}

+ (NSArray <__kindof OEDBItem *> *)allObjectsInContext:(NSManagedObjectContext *)context error:(NSError **)error
{
    return [self allObjectsInContext:context sortBy:nil error:error];
}

+ (NSArray <__kindof OEDBItem *> *)allObjectsInContext:(NSManagedObjectContext *)context sortBy:(nullable NSArray <NSSortDescriptor *> *)sortDescriptors error:(NSError **)error
{
    NSFetchRequest *request = [NSFetchRequest fetchRequestWithEntityName:[self entityName]];
    request.sortDescriptors = sortDescriptors;
    return [context executeFetchRequest:request error:error];
}

+ (instancetype)objectWithURI:(NSURL *)objectURI inContext:(NSManagedObjectContext*)context
{
    __block NSManagedObjectID *objectID = nil;
    [context performBlockAndWait:^{
        objectID = [context.persistentStoreCoordinator managedObjectIDForURIRepresentation:objectURI];
    }];
    return [self objectWithID:objectID inContext:context];
}

+ (instancetype)objectWithID:(NSManagedObjectID *)objectID inContext:(NSManagedObjectContext*)context
{
    if(objectID == nil) return nil;

    __block id result = nil;
    [context performBlockAndWait:^{
        result = [context objectWithID:objectID];

        NSFetchRequest *request = [[NSFetchRequest alloc] initWithEntityName:[self entityName]];
        NSPredicate *predicate = [NSPredicate predicateWithFormat: @"(self == %@)", result];
        request.predicate = predicate;
        result = [context executeFetchRequest:request error:nil].lastObject;

    }];
    return result;
}

- (NSManagedObjectID *)permanentID
{
    NSManagedObjectID *result = self.objectID;

    if(result.isTemporaryID)
    {
        [self.managedObjectContext obtainPermanentIDsForObjects:@[self] error:nil];
        result = self.objectID;
    }

    return result;
}

- (NSURL *)permanentIDURI
{
    return self.permanentID.URIRepresentation;
}


+ (NSString *)entityName
{
    NSAssert(NO, @"+entityName must be overriden");
    return nil;
}

- (NSString *)entityName
{
    return [self.class entityName];
}

+ (NSEntityDescription *)entityDescriptionInContext:(NSManagedObjectContext *)context
{
    return [NSEntityDescription entityForName:[self entityName] inManagedObjectContext:context];
}

- (BOOL)save
{
    __block BOOL result = NO;
    NSManagedObjectContext *context = self.managedObjectContext;
    [context performBlockAndWait:^{
        NSError *error = nil;
        result = [context save:&error];
        if(!result)
            DLog(@"%@", error);
    }];
    return result;
}

- (void)delete
{
    NSManagedObjectContext *context = self.managedObjectContext;
    [context performBlockAndWait:^{
        [context deleteObject:self];
    }];
}

@end

NS_ASSUME_NONNULL_END
