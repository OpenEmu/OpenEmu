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

@import Foundation;
@import CoreData;

@class OEDBRom;
@class OEDBGame;
@class OEDBSystem;
@class OEROMImporter;
@protocol OESidebarItem;

NS_ASSUME_NONNULL_BEGIN

extern const int OELibraryErrorCodeFolderNotFound;
extern const int OELibraryErrorCodeFileInFolderNotFound;

#define OEDatabaseFileName @"Library.storedata"

extern NSNotificationName const OELibraryDidLoadNotificationName;

extern NSString *const OEDatabasePathKey;
extern NSString *const OEDefaultDatabasePathKey;
extern NSString *const OESaveStateLastFSEventIDKey;
extern NSString *const OELibraryDatabaseUserInfoKey;
extern NSString *const OESaveStateFolderURLKey;
extern NSString *const OEScreenshotFolderURLKey;

extern NSString *const OEManagedObjectContextHasDirectChangesKey;

@interface OELibraryDatabase: NSObject

@property(class, readonly, nullable) OELibraryDatabase *defaultDatabase;

+ (BOOL)loadFromURL:(NSURL *)libraryURL error:(NSError **)error;

@property(readonly) NSManagedObjectContext *writerContext;
@property(readonly) NSManagedObjectContext *mainThreadContext;

- (NSManagedObjectContext *)makeChildContext;
- (NSManagedObjectContext *)makeWriterChildContext;

@property(strong) OEROMImporter *importer;

#pragma mark - Administration

- (void)disableSystemsWithoutPlugin;

#pragma mark - Database queries

@property(readonly) NSArray<id<OESidebarItem>> *collections;
- (NSArray *)romsForPredicate:(NSPredicate *)predicate;
@property(readonly, nullable) NSArray <OEDBRom *> *lastPlayedRoms;
@property(readonly, nullable) NSDictionary <NSString *, NSArray <OEDBRom *> *> *lastPlayedRomsBySystem;

#pragma mark - Database Collection editing

- (id)addNewCollection:(nullable NSString *)name;
- (id)addNewSmartCollection:(nullable NSString *)name;
- (id)addNewCollectionFolder:(nullable NSString *)name;

#pragma mark - Database Folders

@property(readonly) NSURL *databaseFolderURL;
@property (nullable) NSURL *romsFolderURL;
@property(readonly) NSURL *unsortedRomsFolderURL;
- (NSURL *)romsFolderURLForSystem:(OEDBSystem *)system;
@property(readonly) NSURL *stateFolderURL;
- (NSURL *)stateFolderURLForSystem:(OEDBSystem *)system;
- (NSURL *)stateFolderURLForROM:(OEDBRom *)rom;
@property(readonly) NSURL *screenshotFolderURL;
@property(readonly) NSURL *coverFolderURL;
@property(readonly) NSURL *importQueueURL;
@property(readonly) NSURL *autoImportFolderURL;

#pragma mark - OpenVGDB Sync

- (void)startOpenVGDBSync;

// Exposed for library migration
@property(strong, nullable) NSPersistentStoreCoordinator *persistentStoreCoordinator;

#pragma mark - Debug

- (void)dump;

@end

NS_ASSUME_NONNULL_END
