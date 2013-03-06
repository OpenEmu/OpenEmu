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

#ifdef DEBUG_IMPORT
#define IMPORTDLog DLog
#else
#define IMPORTDLog(format, ...) do {} while (0)
#endif

#pragma mark User Default Keys -
extern NSString *const OEOrganizeLibraryKey;
extern NSString *const OECopyToLibraryKey;
extern NSString *const OEAutomaticallyGetInfoKey;

#pragma mark Error Codes -
extern NSString *const OEImportErrorDomainFatal;
extern NSString *const OEImportErrorDomainResolvable;
extern NSString *const OEImportErrorDomainSuccess;

typedef enum : NSInteger {
    OEImportErrorCodeAlreadyInDatabase     = -1,
    OEImportErrorCodeMultipleSystems       = 2,
    OEImportErrorCodeNoSystem              = 3,
    OEImportErrorCodeInvalidFile           = 4,
    OEImportErrorCodeAdditionalFiles       = 5,
} OEImportErrorCode;

#pragma mark - Import Info Keys
extern NSString *const OEImportInfoMD5;
extern NSString *const OEImportInfoCRC;
extern NSString *const OEImportInfoROMObjectID;
extern NSString *const OEImportInfoSystemID;

#pragma mark - Importer Status
typedef enum : NSInteger {
    OEImporterStatusStopped  = 1,
    OEImporterStatusRunning  = 2,
    OEImporterStatusPausing  = 3,
    OEImporterStatusPaused   = 4,
    OEImporterStatusStopping = 5,
} OEImporterStatus;

@class OELibraryDatabase;
@protocol OEROMImporterDelegate;

@interface OEROMImporter : NSObject

- (id)initWithDatabase:(OELibraryDatabase *)aDatabase;

@property(weak, readonly) OELibraryDatabase *database;
@property(strong) id<OEROMImporterDelegate> delegate;

@property(readonly) NSInteger status;

#pragma mark - Importing Items -
- (BOOL)importItemAtPath:(NSString *)path;
- (BOOL)importItemsAtPaths:(NSArray *)path;
- (BOOL)importItemAtURL:(NSURL *)url;
- (BOOL)importItemsAtURLs:(NSArray *)url;

- (BOOL)importItemAtPath:(NSString *)path withCompletionHandler:(OEImportItemCompletionBlock)handler;
- (BOOL)importItemsAtPaths:(NSArray *)paths withCompletionHandler:(OEImportItemCompletionBlock)handler;
- (BOOL)importItemAtURL:(NSURL *)url withCompletionHandler:(OEImportItemCompletionBlock)handler;
- (BOOL)importItemsAtURLs:(NSArray *)urls withCompletionHandler:(OEImportItemCompletionBlock)handler;

#pragma mark - Spotlight importing -
- (void)discoverRoms:(NSArray *)volumes;
- (void)updateSearchResults:(NSNotification *)notification;
- (void)finalizeSearchResults:(NSNotification *)notification;
- (void)importSpotlightResultsInBackground;
@end

#pragma mark - Controlling Import
@interface OEROMImporter (Control)
- (void)start;
- (void)togglePause;
- (void)pause;
- (void)cancel;

- (void)processNextItemIfNeeded;
- (void)removeFinished;

- (BOOL)saveQueue;
- (BOOL)loadQueue;

@property(readonly) NSInteger totalNumberOfItems;
@property(readonly) NSInteger numberOfProcessedItems;
@end

#pragma mark - Importer Delegate

@protocol OEROMImporterDelegate <NSObject>
@optional
- (void)romImporterDidStart:(OEROMImporter *)importer;
- (void)romImporterDidCancel:(OEROMImporter *)importer;
- (void)romImporterDidPause:(OEROMImporter *)importer;
- (void)romImporterDidFinish:(OEROMImporter *)importer;
- (void)romImporterChangedItemCount:(OEROMImporter *)importer;
- (void)romImporter:(OEROMImporter *)importer startedProcessingItem:(OEImportItem *)item;
- (void)romImporter:(OEROMImporter *)importer changedProcessingPhaseOfItem:(OEImportItem *)item;
- (void)romImporter:(OEROMImporter *)importer stoppedProcessingItem:(OEImportItem *)item;
@end
