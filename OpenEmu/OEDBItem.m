/*
 Copyright (c) 2012, OpenEmu Team
 
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

@implementation OEDBItem
- (OELibraryDatabase *)libraryDatabase
{
    return [[[self managedObjectContext] userInfo] valueForKey:OELibraryDatabaseUserInfoKey];
}

+ (instancetype)objectWithURI:(NSURL *)uri
{
    return [self objectWithURI:uri inLibrary:[OELibraryDatabase defaultDatabase]];
}

+ (instancetype)objectWithURI:(NSURL *)uri inLibrary:(OELibraryDatabase*)library
{
    __block NSManagedObjectID *objectID = nil;
    NSManagedObjectContext *context = [library safeContext];
    [context performBlockAndWait:^{
        objectID = [[context persistentStoreCoordinator] managedObjectIDForURIRepresentation:uri];
    }];
    return [self objectWithID:objectID inLibrary:library];
}
+ (instancetype)objectWithID:(NSManagedObjectID *)objectID
{
    return [self objectWithID:objectID inLibrary:[OELibraryDatabase defaultDatabase]];
}

+ (instancetype)objectWithID:(NSManagedObjectID *)objectID inLibrary:(OELibraryDatabase*)library
{
    if(objectID == nil) return nil;

    __block id result = nil;
    NSManagedObjectContext *context = [library safeContext];
    [context performBlockAndWait:^{
        result = [context objectWithID:objectID];
        if([result isDeleted])
            result = nil;
    }];
    return result;
}

- (NSManagedObjectID*)permanentID
{
    __block NSManagedObjectID *result = nil;
    NSManagedObjectContext *context = [self managedObjectContext];
    [context performBlockAndWait:^{
        if([context obtainPermanentIDsForObjects:@[self] error:nil])
        {
            result = [self objectID];
        }
    }];
    return result;
}

- (NSURL*)permanentIDURI
{
    return [[self permanentID] URIRepresentation];
}

@end
