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
#import "OEImportOperation.h"

@class OELibraryDatabase;
@protocol OEROMImporterDelegate;

NS_ASSUME_NONNULL_BEGIN

#pragma mark - Notifications

extern NSNotificationName const OEROMImporterDidStartNotification  NS_SWIFT_NAME(oeROMImporterDidStart);
extern NSNotificationName const OEROMImporterDidCancelNotification NS_SWIFT_NAME(oeROMImporterDidCancel);
extern NSNotificationName const OEROMImporterDidPauseNotification  NS_SWIFT_NAME(oeROMImporterDidPause);
extern NSNotificationName const OEROMImporterDidFinishNotification NS_SWIFT_NAME(oeROMImporterDidFinish);
extern NSNotificationName const OEROMImporterChangedItemCountNotification NS_SWIFT_NAME(oeROMImporterChangedItemCount);
extern NSNotificationName const OEROMImporterStoppedProcessingItemNotification NS_SWIFT_NAME(oeROMImporterStoppedProcessingItem);

typedef NSString * OEROMImporterUserInfoKey NS_TYPED_ENUM;

extern OEROMImporterUserInfoKey const OEROMImporterItemKey;

#pragma mark - User Default Keys

extern NSString *const OECopyToLibraryKey;
extern NSString *const OEAutomaticallyGetInfoKey;

#pragma mark - Error Codes

extern NSString *const OEImportErrorDomainFatal;
extern NSString *const OEImportErrorDomainResolvable;
extern NSString *const OEImportErrorDomainSuccess;

typedef NS_ENUM(NSInteger, OEImportErrorCode) {
    OEImportErrorCodeAlreadyInDatabase     = -1,
    OEImportErrorCodeMultipleSystems       = 2,
    OEImportErrorCodeNoSystem              = 3,
    OEImportErrorCodeInvalidFile           = 4,
    OEImportErrorCodeAdditionalFiles       = 5,
    OEImportErrorCodeNoHash                = 6,
    OEImportErrorCodeNoGame                = 7,
    OEImportErrorCodeDisallowArchivedFile  = 8,
    OEImportErrorCodeEmptyFile             = 9,
    OEImportErrorCodeAlreadyInDatabaseFileUnreachable = 10,
};

#pragma mark - Importer Status

typedef NS_ENUM(NSInteger, OEImporterStatus) {
    OEImporterStatusStopped  = 1,
    OEImporterStatusRunning  = 2,
   // OEImporterStatusPausing  = 3,
    OEImporterStatusPaused   = 4,
   // OEImporterStatusStopping = 5,
};

@interface OEROMImporter : NSObject

- (instancetype)initWithDatabase:(OELibraryDatabase *)aDatabase;

@property(weak, readonly) OELibraryDatabase *database;
@property(strong)         id<OEROMImporterDelegate> delegate;
@property(readonly)       OEImporterStatus status;
@property(readonly)       NSManagedObjectContext *context;

#pragma mark - Importing Items -

- (BOOL)importItemAtPath:(NSString *)path;
- (BOOL)importItemsAtPaths:(NSArray *)path;
- (BOOL)importItemAtURL:(NSURL *)url;
- (BOOL)importItemsAtURLs:(NSArray *)url;

- (BOOL)importItemAtPath:(NSString *)path intoCollectionWithID:(nullable NSManagedObjectID *)collectionID;
- (BOOL)importItemsAtPaths:(NSArray <NSString *> *)path intoCollectionWithID:(nullable NSManagedObjectID *)collectionID;
- (BOOL)importItemAtURL:(NSURL *)itemURL intoCollectionWithID:(nullable NSManagedObjectID *)collectionID;
- (BOOL)importItemsAtURLs:(NSArray <NSURL *> *)itemURLs intoCollectionWithID:(nullable NSManagedObjectID *)collectionID;

- (BOOL)importItemAtPath:(NSString *)path intoCollectionWithID:(nullable NSManagedObjectID *)collectionID withCompletionHandler:(nullable OEImportItemCompletionBlock)handler;
- (BOOL)importItemsAtPaths:(NSArray <NSString *> *)paths intoCollectionWithID:(nullable NSManagedObjectID *)collectionID  withCompletionHandler:(nullable OEImportItemCompletionBlock)handler;
- (BOOL)importItemAtURL:(NSURL *)itemURL intoCollectionWithID:(nullable NSManagedObjectID *)collectionID  withCompletionHandler:(nullable OEImportItemCompletionBlock)handler;
- (BOOL)importItemsAtURLs:(NSArray <NSURL *> *)itemURLs intoCollectionWithID:(nullable NSManagedObjectID *)collectionID  withCompletionHandler:(nullable OEImportItemCompletionBlock)handler;

- (BOOL)importItemAtPath:(NSString *)path withCompletionHandler:(nullable OEImportItemCompletionBlock)handler;
- (BOOL)importItemsAtPaths:(NSArray <NSString *> *)paths withCompletionHandler:(nullable OEImportItemCompletionBlock)handler;
- (BOOL)importItemAtURL:(NSURL *)itemURL withCompletionHandler:(nullable OEImportItemCompletionBlock)handler;
- (BOOL)importItemsAtURLs:(NSArray *)itemURLs withCompletionHandler:(nullable OEImportItemCompletionBlock)handler;

- (void)addOperation:(OEImportOperation *)operation;
- (void)rescheduleOperation:(OEImportOperation *)operation;

@end

#pragma mark - Controlling Import

@interface OEROMImporter (Control)

- (void)start;
- (void)togglePause;
- (void)pause;
- (void)cancel;

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
- (void)romImporter:(OEROMImporter *)importer startedProcessingItem:(OEImportOperation *)item;
- (void)romImporter:(OEROMImporter *)importer changedProcessingPhaseOfItem:(OEImportOperation *)item;
- (void)romImporter:(OEROMImporter *)importer stoppedProcessingItem:(OEImportOperation *)item;
@end

NS_ASSUME_NONNULL_END
