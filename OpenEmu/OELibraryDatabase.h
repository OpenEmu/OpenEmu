/*
 Copyright (c) 2011, OpenEmu Team
 
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

#import <Foundation/Foundation.h>
#import "OEROMImporter.h"

#import "OEDBRom.h"
#import "OEDBGame.h"
#import "OEDBSystem.h"

@class OEROMImporter;
#define OELibraryErrorCodeFolderNotFound 11789
#define OELibraryErrorCodeFileInFolderNotFound 11790

#define OEDatabaseFileName @"Library.storedata"

extern NSString *const OEDatabasePathKey;
extern NSString *const OEDefaultDatabasePathKey;
extern NSString *const OESaveStateLastFSEventIDKey;
extern NSString *const OELibraryDatabaseUserInfoKey;

@interface OELibraryDatabase : NSObject 
+ (OELibraryDatabase*)defaultDatabase;

+ (BOOL)loadFromURL:(NSURL*)url error:(NSError**)outError;
- (BOOL)save:(NSError**)error;

- (NSManagedObjectContext*)managedObjectContext;

- (id)objectWithURI:(NSURL*)uri;

@property (strong) OEROMImporter *importer;

#pragma mark - Administration
- (void)disableSystemsWithoutPlugin;

#pragma mark - Database queries
- (NSManagedObjectID*)managedObjectIDForURIRepresentation:(NSURL*)uri;

- (NSUInteger)collectionsCount;
- (NSArray*)collections;

- (NSArray*)romsForPredicate:(NSPredicate*)predicate;
- (NSArray*)romsInCollection:(id)collection;

- (NSArray*)lastPlayedRoms;
- (NSDictionary*)lastPlayedRomsBySystem;
#pragma mark - Database Collection editing
- (void)removeCollection:(NSManagedObject*)collection;

- (id)addNewCollection:(NSString*)name;
- (id)addNewSmartCollection:(NSString*)name;
- (id)addNewCollectionFolder:(NSString*)name;
#pragma mark - Database Folders
- (NSURL *)databaseFolderURL;
- (NSURL *)romsFolderURL;
- (NSURL *)unsortedRomsFolderURL;
- (NSURL *)romsFolderURLForSystem:(OEDBSystem *)system;
- (NSURL *)stateFolderURL;
- (NSURL *)stateFolderURLForSystem:(OEDBSystem *)system;
- (NSURL *)stateFolderURLForROM:(OEDBRom *)rom;
- (NSURL *)coverFolderURL;
@end
