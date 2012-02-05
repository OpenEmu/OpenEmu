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

@class OEDBSystem, OEDBGame, OEDBRom;

#define OELibraryErrorCodeFolderNotFound 11789
#define OELibraryErrorCodeFileInFolderNotFound 11790

#define OEDatabaseFileName @"Library.storedata"
@interface OELibraryDatabase : NSObject 
{
@private
    NSArrayController* romsController;
    
    NSManagedObjectModel *__managedObjectModel;
    NSManagedObjectContext *__managedObjectContext;    
    NSMutableDictionary* managedObjectContexts;
}
#pragma mark -
+ (BOOL)loadFromURL:(NSURL*)url error:(NSError**)outError;
#pragma mark -
- (BOOL)save:(NSError**)error;
- (NSManagedObjectContext*)managedObjectContext;

+ (OELibraryDatabase*)defaultDatabase;
#pragma mark -
#pragma mark Database queries
- (NSArray*)systems;
- (NSArray*)enabledSystems;
- (OEDBSystem*)systemWithIdentifier:(NSString*)identifier;
- (OEDBSystem*)systemWithArchiveID:(NSNumber*)aID;
- (OEDBSystem*)systemWithArchiveName:(NSString*)name;
- (OEDBSystem*)systemWithArchiveShortname:(NSString*)shortname;

- (OEDBSystem*)systemForFile:(NSString*)filePath;
- (NSInteger)systemsCount;

- (OEDBGame*)gameWithArchiveID:(NSNumber*)archiveID;

- (NSUInteger)collectionsCount;
- (NSArray*)collections;

- (OEDBRom*)romForMD5Hash:(NSString*)hashString;
- (OEDBRom*)romForCRC32Hash:(NSString*)crc32String;
- (OEDBRom*)romForWithPath:(NSString*)path;
- (NSArray*)romsForPredicate:(NSPredicate*)predicate;
- (NSArray*)romsInCollection:(id)collection;
#pragma mark -
#pragma mark Database Collection editing
- (void)removeCollection:(NSManagedObject*)collection;

- (id)addNewCollection:(NSString*)name;
- (id)addNewSmartCollection:(NSString*)name;
- (id)addNewCollectionFolder:(NSString*)name;

#pragma mark -
#pragma mark Database Game editing
- (BOOL)isFileInDatabaseWithPath:(NSString*)path error:(NSError**)error;
- (void)addGamesFromPath:(NSString*)path toCollection:(NSManagedObject*)collection searchSubfolders:(BOOL)subfolderFlag DEPRECATED_ATTRIBUTE;

- (OEDBRom*)createROMandGameForFile:(NSString*)filePath error:(NSError**)outError;

/*
 - (void)addRomsWithFiles:(NSArray*)files;
 - (void)addRomWithFile:(NSString*)file;
 
 - (void)deleteRomWithID:(id)romID;
 - (void)deleteRomsWithIDs:(NSArray*)romIDs;
 
 - (void)updateRom:(id)newValues;
 - (void)updateRoms:(NSArray*)newValues;
 
 #pragma mark -
 #pragma mark Basic Collection Handling
 - (void)addRom:(id)rom toCollection:(id)collection;
 - (void)removeRom:(id)rom fromCollection:(id)collection;
 - (void)moveRomAtIndex:(id)index toIndex:(id)newIndex;
 
 #pragma mark -
 #pragma mark Smart Collection Handling
 - (void)addRule:(id)rule toCollection:(id)collection;
 - (void)removeRule:(id)rule fromCollection:(id)collection;
 */
#pragma mark -
@property (readonly) NSString* databaseFolderPath;
@property (readonly) NSString* databaseUnsortedRomsPath;
#pragma mark -
@property (copy) NSURL* databaseURL;
@property (retain) NSPersistentStoreCoordinator * persistentStoreCoordinator;
@end
