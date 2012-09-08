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
#import "OEImportItem.h"

#pragma mark Error Constants
#define MaxSimulatenousImports 1 // imports can't really be simulatenous because access to queue is not ready for multithreadding right now
#define OEImportErrorDomainFatal @"OEImportFatalDomain"
#define OEImportErrorDomainResolvable @"OEImportResolvableDomain"
#define OEImportErrorDomainSuccess @"OEImportSucessDomain"

#define OEImportErrorCodeAlreadyInDatabase -1

#define OEImportErrorCodeWaitingForArchiveSync 1
#define OEImportErrorCodeMultipleSystems 2

#pragma mark - Import Info Keys
#define OEImportInfoMD5 @"md5"
#define OEImportInfoCRC @"crc"
#define OEImportInfoROMObjectID @"RomObjectID"
#define OEImportInfoSystemID @"systemID"
#define OEImportInfoArchiveSync @"archiveSync"
#pragma mark -

extern NSString *const OEOrganizeLibraryKey;
extern NSString *const OECopyToLibraryKey;
extern NSString *const OEAutomaticallyGetInfoKey;

@class OELibraryDatabase;
@protocol OEROMImporterDelegate;
@interface OEROMImporter : NSObject
- (id)initWithDatabase:(OELibraryDatabase *)aDatabase;

@property (weak, readonly) OELibraryDatabase *database;
@property (readonly) BOOL isBusy;

@property (strong) id <OEROMImporterDelegate> delegate;

@property NSMutableArray *queue;
#pragma mark - Importing Items
- (void)importItemAtPath:(NSString*)path;
- (void)importItemsAtPaths:(NSArray*)path;
- (void)importItemAtURL:(NSURL*)url;
- (void)importItemsAtURLs:(NSArray*)url;

- (void)importItemAtPath:(NSString*)path withCompletionHandler:(OEImportItemCompletionBlock)handler;
- (void)importItemsAtPaths:(NSArray*)paths withCompletionHandler:(OEImportItemCompletionBlock)handler;
- (void)importItemAtURL:(NSURL*)url withCompletionHandler:(OEImportItemCompletionBlock)handler;
- (void)importItemsAtURLs:(NSArray*)urls withCompletionHandler:(OEImportItemCompletionBlock)handler;

#pragma mark - Handle Spotlight importing
- (void)discoverRoms:(NSArray*)volumes;
- (void)updateSearchResults:(NSNotification*)notification;
- (void)finalizeSearchResults:(NSNotification*)notification;
- (void)importInBackground;
@end

#pragma mark - Controlling Import
@interface OEROMImporter (Control)
- (void)pause;
- (void)start;
- (void)startQueueIfNeeded;
- (void)cancel;
- (void)removeFinished;

- (NSUInteger)numberOfItems;
- (NSUInteger)finishedItems;
@end

#pragma mark - Importer Delegate
@protocol OEROMImporterDelegate <NSObject>
- (void)romImporter:(OEROMImporter*)importer startedProcessingItem:(OEImportItem*)item;
- (void)romImporter:(OEROMImporter *)importer changedProcessingPhaseOfItem:(OEImportItem*)item;
- (void)romImporter:(OEROMImporter*)importer finishedProcessingItem:(OEImportItem*)item;
@end
