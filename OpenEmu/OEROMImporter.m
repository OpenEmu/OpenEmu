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

#import "OEROMImporter+Private.h"
#import "OEImportOperation.h"

#import "OELibraryDatabase.h"
#import "OEDBRom.h"
#import "OEDBGame.h"
#import "OEDBCollection.h"
#import "OEDBSystem.h"
@import OpenEmuKit;

#import "NSFileManager+OEHashingAdditions.h"
#import "NSArray+OEAdditions.h"

#import <CommonCrypto/CommonDigest.h>
#import <OpenEmuSystem/OpenEmuSystem.h>
#import <XADMaster/XADArchive.h>
#import <objc/runtime.h>

#import "OpenEmu-Swift.h"

NS_ASSUME_NONNULL_BEGIN

#pragma mark - Notifications

NSNotificationName const OEROMImporterDidStartNotification          = @"OEROMImporterDidStart";
NSNotificationName const OEROMImporterDidCancelNotification         = @"OEROMImporterDidCancel";
NSNotificationName const OEROMImporterDidPauseNotification          = @"OEROMImporterDidPause";
NSNotificationName const OEROMImporterDidFinishNotification         = @"OEROMImporterDidFinish";
NSNotificationName const OEROMImporterChangedItemCountNotification  = @"OEROMImporterChangedItemCount";
NSNotificationName const OEROMImporterStoppedProcessingItemNotification = @"OEROMImporterStoppedProcessingItem";

OEROMImporterUserInfoKey const OEROMImporterItemKey = @"OEROMImporterItemKey";

#pragma mark - User Default Keys

NSString *const OECopyToLibraryKey         = @"copyToLibrary";
NSString *const OEAutomaticallyGetInfoKey  = @"automaticallyGetInfo";

#pragma mark - Error Codes
NSString *const OEImportErrorDomainFatal      = @"OEImportFatalDomain";
NSString *const OEImportErrorDomainResolvable = @"OEImportResolvableDomain";
NSString *const OEImportErrorDomainSuccess    = @"OEImportSuccessDomain";

@interface OEROMImporter ()

@property(weak)                 OELibraryDatabase *database;
@property(readwrite, strong)    NSOperationQueue  *operationQueue;
@property(readwrite, strong)    NSManagedObjectContext *context;

@property(readwrite)            OEImporterStatus   status;
@property(readwrite)            NSInteger          numberOfProcessedItems;
@property(readwrite, nonatomic) NSInteger          totalNumberOfItems;

@end

@implementation OEROMImporter
@synthesize database, delegate;

+ (void)initialize
{
    if(self != [OEROMImporter class]) return;

    NSDictionary *defaults = @{
                              OECopyToLibraryKey        : @(YES),
                              OEAutomaticallyGetInfoKey : @(YES),
                              };
    [[NSUserDefaults standardUserDefaults] registerDefaults:defaults];
}

- (id)initWithDatabase:(OELibraryDatabase *)aDatabase
{
    self = [super init];
    if (self != nil)
    {
        self.database = aDatabase;
        self.numberOfProcessedItems = 0;

        NSOperationQueue *queue = [[NSOperationQueue alloc] init];
        queue.maxConcurrentOperationCount = 1;
        queue.name = @"org.openemu.importqueue";
        self.operationQueue = queue;
        self.status = OEImporterStatusStopped;

        NSOperation *initializeMOCOp = [NSBlockOperation blockOperationWithBlock:^{
            NSManagedObjectContext *context = [aDatabase makeChildContext];
            context.name = @"OEROMImporter";
            // An OEDBSystem object's `enabled` property could become out of sync (always enabled=YES) in OEROMImporter child context without this.
            context.automaticallyMergesChangesFromParent = YES;
            self.context = context;
        }];
        [queue addOperations:@[initializeMOCOp] waitUntilFinished:YES];
        queue.suspended = YES;
    }
    return self;
}

#pragma mark - State

- (BOOL)saveQueue
{
    NSOperationQueue *queue = self.operationQueue;
    queue.suspended = YES;

    NSURL *url = self.database.importQueueURL;

    // remove last saved queue if any
    [[NSFileManager defaultManager] removeItemAtURL:url error:nil];
    
    NSData *queueData = [self dataForOperationQueue:queue.operations];
    if(queueData)
    {
        return [queueData writeToURL:url atomically:YES];
    }
    return YES;
}

- (NSData *)dataForOperationQueue:(NSArray<__kindof NSOperation *> *)queue
{
    // only pick OEImportOperations
    NSPredicate *filterPredicate = [NSPredicate predicateWithBlock:^BOOL(id evaluatedObject, NSDictionary *bindings) {
        return [evaluatedObject isKindOfClass:[OEImportOperation class]]
                    && ![evaluatedObject isFinished] && ![evaluatedObject isCancelled];
    }];
    
    NSArray<OEImportOperation *> *operations = [queue filteredArrayUsingPredicate:filterPredicate];
    if (operations.count > 0)
    {
        return [NSKeyedArchiver archivedDataWithRootObject:operations requiringSecureCoding:YES error:nil];
    }
    return nil;
}

- (NSArray<OEImportOperation *> *)operationQueueFromData:(NSData *)data
{
    NSSet<Class> *classes = [NSSet setWithObjects:NSArray.class, OEImportOperation.class, nil];
    return [NSKeyedUnarchiver unarchivedObjectOfClasses:classes fromData:data error:nil];
}

- (BOOL)loadQueue
{
    NSURL *url = self.database.importQueueURL;

    // read previously stored data
    NSData *queueData = [NSData dataWithContentsOfURL:url];
    if(queueData == nil)
        return NO;
    
    // remove file if reading was successfull
    [[NSFileManager defaultManager] removeItemAtURL:url error:nil];
    
    NSArray<OEImportOperation *> *operations = [self operationQueueFromData:queueData];
    if (operations.count > 0)
    {
        self.numberOfProcessedItems = 0;
        self.totalNumberOfItems = operations.count;
        [self.operationQueue addOperations:operations waitUntilFinished:NO];
        return YES;
    }
    return NO;
}

#pragma mark - Importing Items with completion handler

- (BOOL)importItemAtPath:(NSString *)path withCompletionHandler:(nullable OEImportItemCompletionBlock)handler
{
    return [self importItemAtPath:path intoCollectionWithID:nil withCompletionHandler:handler];
}

- (BOOL)importItemsAtPaths:(NSArray <NSString *> *)paths withCompletionHandler:(nullable OEImportItemCompletionBlock)handler
{
    return [self importItemsAtPaths:paths intoCollectionWithID:nil withCompletionHandler:handler];
}

- (BOOL)importItemAtURL:(NSURL *)url withCompletionHandler:(nullable OEImportItemCompletionBlock)handler
{
    return [self importItemAtURL:url intoCollectionWithID:nil withCompletionHandler:handler];
}

- (BOOL)importItemsAtURLs:(NSArray <NSURL *> *)urls withCompletionHandler:(nullable OEImportItemCompletionBlock)handler
{
    return [self importItemsAtURLs:urls intoCollectionWithID:nil withCompletionHandler:handler];
}

#pragma mark - Importing Items without completion handler

- (BOOL)importItemAtPath:(NSString *)path
{
    return [self importItemAtPath:path withCompletionHandler:nil];
}

- (BOOL)importItemsAtPaths:(NSArray <NSString *> *)paths
{
    return [self importItemsAtPaths:paths withCompletionHandler:nil];
}

- (BOOL)importItemAtURL:(NSURL *)url
{
    return [self importItemAtURL:url withCompletionHandler:nil];
}

- (BOOL)importItemsAtURLs:(NSArray <NSURL *> *)urls
{
    return [self importItemsAtURLs:urls withCompletionHandler:nil];
}

#pragma mark - Importing items into collections

- (BOOL)importItemAtPath:(NSString *)path intoCollectionWithID:(nullable NSManagedObjectID *)collectionID
{
    return [self importItemAtPath:path intoCollectionWithID:collectionID withCompletionHandler:nil];
}
- (BOOL)importItemsAtPaths:(NSArray <NSString *> *)paths intoCollectionWithID:(nullable NSManagedObjectID *)collectionID
{
    return [self importItemsAtPaths:paths intoCollectionWithID:collectionID withCompletionHandler:nil];
}

- (BOOL)importItemAtURL:(NSURL *)url intoCollectionWithID:(nullable NSManagedObjectID *)collectionID
{
    return [self importItemAtURL:url intoCollectionWithID:collectionID withCompletionHandler:nil];
}
- (BOOL)importItemsAtURLs:(NSArray <NSURL *> *)urls intoCollectionWithID:(nullable NSManagedObjectID *)collectionID
{
    return [self importItemsAtURLs:urls intoCollectionWithID:collectionID withCompletionHandler:nil];
}
#pragma mark - Importing items into collections with completion handlers

- (BOOL)importItemAtPath:(NSString *)path intoCollectionWithID:(nullable NSManagedObjectID *)collectionID withCompletionHandler:(nullable OEImportItemCompletionBlock)handler
{
    NSURL *url = [NSURL fileURLWithPath:path];
    return [self importItemAtURL:url intoCollectionWithID:collectionID withCompletionHandler:handler];
}
- (BOOL)importItemsAtPaths:(NSArray <NSString *> *)paths intoCollectionWithID:(nullable NSManagedObjectID *)collectionID  withCompletionHandler:(nullable OEImportItemCompletionBlock)handler
{
    BOOL success = NO;
    for(NSString *obj in paths)
    {
        success = [self importItemAtPath:obj intoCollectionWithID:collectionID withCompletionHandler:handler] || success;
    }
    return success;
}

- (BOOL)importItemAtURL:(NSURL *)url intoCollectionWithID:(nullable NSManagedObjectID *)collectionID  withCompletionHandler:(nullable OEImportItemCompletionBlock)handler
{
    NSOperationQueue *queue = self.operationQueue;
    NSArray *operations     = queue.operations;

    // check operation queue for items that have already import the same url
    OEImportOperation *item = [operations firstObjectMatchingBlock:^ BOOL (id item){
        return [item isKindOfClass:[OEImportOperation class]] && [[item URL] isEqualTo:url];
    }];

    if(item == nil)
    {
        OEImportOperation *item = [OEImportOperation operationWithURL:url inImporter:self];
        item.completionHandler = handler;
        item.collectionID = collectionID;

        if(item)
        {
            [self addOperation:item];
            return YES;
        }
    }

    return NO;
}

- (BOOL)importItemsAtURLs:(NSArray <NSURL *> *)urls intoCollectionWithID:(nullable NSManagedObjectID *)collectionID  withCompletionHandler:(nullable OEImportItemCompletionBlock)handler
{
    BOOL success = NO;
    for(NSURL *obj in urls)
    {
        success = [self importItemAtURL:obj intoCollectionWithID:collectionID withCompletionHandler:handler] || success;
    }
    return success;
}

#pragma mark -

- (void)addOperation:(OEImportOperation *)operation
{
    if(operation.completionBlock == nil)
    {
        operation.completionBlock = [self OE_completionHandlerForOperation:operation];
    }

    NSOperationQueue *queue = self.operationQueue;
    [queue addOperation:operation];
    self.totalNumberOfItems++;
    [self start];

    [self postNotificationName:OEROMImporterChangedItemCountNotification userInfo:nil];
    [self OE_delegateRespondsToSelector:@selector(romImporterChangedItemCount:) block: ^{
        [self.delegate romImporterChangedItemCount:self];
    }];
}

- (void)rescheduleOperation:(OEImportOperation* )operation
{
    OEImportOperation *copy = [operation copy];
    [copy setCompletionBlock:[self OE_completionHandlerForOperation:copy]];

    NSOperationQueue *queue = self.operationQueue;
    [queue addOperation:copy];

    self.numberOfProcessedItems --;
}

- (void(^)(void))OE_completionHandlerForOperation:(OEImportOperation*)op
{
    __block OEROMImporter     *importer = self;
    return ^{
        OEImportExitStatus state = [op exitStatus];
        if(state == OEImportExitSuccess)
        {
            importer.numberOfProcessedItems ++;
        }
        else if(state == OEImportExitErrorFatal)
        {
            importer.numberOfProcessedItems ++;
        }
        else if(state == OEImportExitErrorResolvable)
        {
        }

        OEImportItemCompletionBlock block = op.completionHandler;
        if(block != nil)
        {
            // save so changes propagate to other stores
            [[importer context] save:nil];
            
            dispatch_after(1.0, dispatch_get_main_queue(), ^{
                block(op.romObjectID);
            });
        }
        
        [self postNotificationName:OEROMImporterStoppedProcessingItemNotification userInfo:@{ OEROMImporterItemKey: op }];
        [self OE_delegateRespondsToSelector:@selector(romImporter:stoppedProcessingItem:) block:^{
            [self.delegate romImporter:self stoppedProcessingItem:op];
        }];

        if(importer.operationQueue.operationCount == 0)
        {
            [importer finish];
        }

        op.completionHandler = nil;
    };
}

#pragma mark - Controlling Import -

- (void)start
{
    DLog(@"%s", BOOL_STR(self.operationQueue.operationCount != 0 && self.status != OEImporterStatusRunning));
    if(self.operationQueue.operationCount != 0 && self.status != OEImporterStatusRunning)
    {
        self.status = OEImporterStatusRunning;
        self.operationQueue.suspended = NO;
        [self postNotificationName:OEROMImporterDidStartNotification userInfo:nil];
        [self OE_delegateRespondsToSelector:@selector(romImporterDidStart:) block:^{
            [self.delegate romImporterDidStart:self];
        }];
    }
}

- (void)pause
{
    DLog(@"%s", BOOL_STR(self.status == OEImporterStatusRunning));
    if(self.status == OEImporterStatusRunning)
    {
        self.status = OEImporterStatusPaused;
        self.operationQueue.suspended = YES;
        [self postNotificationName:OEROMImporterDidPauseNotification userInfo:nil];
        [self OE_delegateRespondsToSelector:@selector(romImporterDidPause:) block:^{
            [self.delegate romImporterDidPause:self];
        }];
    }
}

- (void)togglePause
{
    DLog(@"%@", (self.status == OEImporterStatusPaused ? @"start" : (self.status == OEImporterStatusRunning ? @"pause" : @"nothing")));

    if(self.status == OEImporterStatusPaused)
        [self start];
    else if(self.status == OEImporterStatusRunning)
        [self pause];
}

- (void)cancel
{
    DLog(@"cancel");

    self.status = OEImporterStatusStopped;
    [self.operationQueue cancelAllOperations];
    
    self.numberOfProcessedItems = 0;
    self.totalNumberOfItems = 0;
    self.operationQueue.suspended = YES;

    [self postNotificationName:OEROMImporterDidCancelNotification userInfo:nil];
    [self OE_delegateRespondsToSelector:@selector(romImporterDidCancel:) block:^{
        [self.delegate romImporterDidCancel:self];
    }];
}

- (void)finish
{
    DLog(@"Finish");

    [self setStatus:OEImporterStatusStopped];
    [self.operationQueue cancelAllOperations];

    self.numberOfProcessedItems = 0;
    self.totalNumberOfItems = 0;
    self.operationQueue.suspended = YES;

    [self postNotificationName:OEROMImporterDidFinishNotification userInfo:nil];
    [self OE_delegateRespondsToSelector:@selector(romImporterDidFinish:) block:^{
        [self.delegate romImporterDidFinish:self];
    }];
}

#pragma mark - Private Methods -

- (void)setTotalNumberOfItems:(NSInteger)totalNumberOfItems
{
    _totalNumberOfItems = totalNumberOfItems;
    [self postNotificationName:OEROMImporterChangedItemCountNotification userInfo:nil];
    [self OE_delegateRespondsToSelector:@selector(romImporterChangedItemCount:) block:^{
        [self.delegate romImporterChangedItemCount:self];
    }];
}

- (void)OE_delegateRespondsToSelector:(SEL)selector block:(void (^)(void))block
{
    if ([self.delegate respondsToSelector:selector])
    {
        if (NSThread.isMainThread)
        {
            block();
        }
        else
        {
            dispatch_sync(dispatch_get_main_queue(), block);
        }
    }
}

- (void)postNotificationName:(NSNotificationName)name userInfo:(NSDictionary * _Nullable)userInfo
{
    __auto_type block = ^{
        [NSNotificationCenter.defaultCenter postNotificationName:name object:self userInfo:userInfo];
    };
    
    if (NSThread.isMainThread)
    {
        block();
    }
    else
    {
        dispatch_sync(dispatch_get_main_queue(), block);
    }
}

@end

NS_ASSUME_NONNULL_END
