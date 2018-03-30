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

#import "OEROMImporter.h"
#import "OEImportOperation.h"

#import "OELibraryDatabase.h"
#import "OEDBRom.h"
#import "OEDBGame.h"
#import "OEDBCollection.h"
#import "OEDBSystem.h"
#import "OESystemPlugin.h"

#import "NSFileManager+OEHashingAdditions.h"
#import "NSArray+OEAdditions.h"

#import <CommonCrypto/CommonDigest.h>
#import <OpenEmuSystem/OpenEmuSystem.h>
#import <XADMaster/XADArchive.h>
#import <objc/runtime.h>

#import "OpenEmu-Swift.h"

NS_ASSUME_NONNULL_BEGIN

#pragma mark User Default Keys -

NSString *const OECopyToLibraryKey         = @"copyToLibrary";
NSString *const OEAutomaticallyGetInfoKey  = @"automaticallyGetInfo";

#pragma mark Error Codes -
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
@property(readwrite, strong)    NSMutableArray    *spotlightSearchResults;

@end

@implementation OEROMImporter
@synthesize database, delegate;
@synthesize spotlightSearchResults;

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
        self.spotlightSearchResults = [NSMutableArray arrayWithCapacity:1];
        self.numberOfProcessedItems = 0;

        NSOperationQueue *queue = [[NSOperationQueue alloc] init];
        queue.maxConcurrentOperationCount = 1;
        queue.name = @"org.openemu.importqueue";
        self.operationQueue = queue;
        self.status = OEImporterStatusStopped;

        NSOperation *initializeMOCOp = [NSBlockOperation blockOperationWithBlock:^{
            NSManagedObjectContext *context = [aDatabase makeChildContext];
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

    // only pick OEImportOperations
    NSPredicate *filterPredicate = [NSPredicate predicateWithBlock:^BOOL(id evaluatedObject, NSDictionary *bindings) {
        return [evaluatedObject isKindOfClass:[OEImportOperation class]]
                    && ![evaluatedObject isFinished] && ![evaluatedObject isCancelled];
    }];
    NSArray *operations = [queue.operations filteredArrayUsingPredicate:filterPredicate];

    // only save queue if it's not empty
    if(operations.count != 0)
    {
        // write new queue data
        NSData *queueData = [NSKeyedArchiver archivedDataWithRootObject:operations];
        return [queueData writeToURL:url atomically:YES];
    }
    return YES;
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
    
    // Restore queue
    NSMutableArray *operations = [NSKeyedUnarchiver unarchiveObjectWithData:queueData];
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

    [self OE_performSelectorOnDelegate:@selector(romImporterChangedItemCount:) withObject:self];
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

        [self OE_performSelectorOnDelegate:@selector(romImporter:stoppedProcessingItem:) withObject:op];

        if(importer.operationQueue.operationCount == 0)
        {
            [importer finish];
        }

        op.completionHandler = nil;
    };
}

#pragma mark - Spotlight importing -

- (void)discoverRoms:(NSArray *)volumes
{
    DLog();
    // TODO: limit searching or results to the volume URLs only.
    
    NSMutableArray *supportedFileExtensions = [[OESystemPlugin supportedTypeExtensions] mutableCopy];
    
    // We skip common types by default.
    NSArray *commonTypes = @[@"zip", @"elf"];
    
    [supportedFileExtensions removeObjectsInArray:commonTypes];
    
    //NSLog(@"Supported search Extensions are: %@", supportedFileExtensions);
    
    NSString *searchString = @"";
    for(NSString *extension in supportedFileExtensions)
    {
        searchString = [searchString stringByAppendingFormat:@"(kMDItemDisplayName == *.%@)", extension];
        searchString = [searchString stringByAppendingString:@" || "];
    }
    
    searchString = [searchString substringWithRange:NSMakeRange(0, searchString.length - 4)];
    
    DLog(@"SearchString: %@", searchString);
    
    MDQueryRef searchQuery = MDQueryCreate(kCFAllocatorDefault, (__bridge CFStringRef)searchString, NULL, NULL);
    
    if(searchQuery != NULL)
    {
        // Limit Scope to selected volumes / URLs only
        MDQuerySetSearchScope(searchQuery, (__bridge CFArrayRef) volumes, 0);
        
        [self.spotlightSearchResults removeAllObjects];
        
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(finalizeSearchResults:)
                                                     name:(NSString *)kMDQueryDidFinishNotification
                                                   object:(__bridge id)searchQuery];
        
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(updateSearchResults:)
                                                     name:(NSString *)kMDQueryProgressNotification
                                                   object:(__bridge id)searchQuery];
        
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(updateSearchResults:)
                                                     name:(NSString *)kMDQueryDidUpdateNotification
                                                   object:(__bridge id)searchQuery];
        
        if(MDQueryExecute(searchQuery, kMDQueryWantsUpdates))
            DLog(@"Searching for importable roms");
        else
        {
            CFRelease(searchQuery);
            searchQuery = nil;
            // leave this log message in...
            DLog(@"MDQuery failed to start.");
        }
        
    }
    else
        DLog(@"Invalid Search Query");
}

- (void)updateSearchResults:(NSNotification *)notification
{
    DLog();
    
    MDQueryRef searchQuery = (__bridge MDQueryRef)notification.object;
    
    
    // If you're going to have the same array for every iteration,
    // don't allocate it inside the loop !
    NSArray *excludedPaths = @[
                               @"System",
                               @"Library",
                               @"Developer",
                               @"Volumes",
                               @"Applications",
                               @"cores",
                               @"dev",
                               @"etc",
                               @"home",
                               @"net",
                               @"sbin",
                               @"private",
                               @"tmp",
                               @"usr",
                               @"var",
                               @"ReadMe", // markdown
                               @"readme", // markdown
                               @"README", // markdown
                               @"Readme", // markdown
                               ];
    
    // assume the latest result is the last index?
    for(CFIndex index = 0, limit = MDQueryGetResultCount(searchQuery); index < limit; index++)
    {
        MDItemRef resultItem = (MDItemRef)MDQueryGetResultAtIndex(searchQuery, index);
        NSString *resultPath = (__bridge_transfer NSString *)MDItemCopyAttribute(resultItem, kMDItemPath);
        
        // Nothing in common
        if([resultPath.pathComponents firstObjectCommonWithArray:excludedPaths] == nil)
        {
            NSDictionary *resultDict = @{ @"Path" : resultPath,
                                          @"Name" : resultPath.lastPathComponent.stringByDeletingPathExtension };
            
            if(![self.spotlightSearchResults containsObject:resultDict])
            {
                [self.spotlightSearchResults addObject:resultDict];
                
                //                NSLog(@"Result Path: %@", resultPath);
            }
        }
    }
}

- (void)finalizeSearchResults:(NSNotification *)notification
{
    MDQueryRef searchQuery = (__bridge_retained MDQueryRef)[notification object];
    DLog(@"Finished searching, found: %lu items", MDQueryGetResultCount(searchQuery));
    
    if(MDQueryGetResultCount(searchQuery))
    {
        [self importSpotlightResultsInBackground];
        
        MDQueryStop(searchQuery);
    }
    
    CFRelease(searchQuery);
}

- (void)importSpotlightResultsInBackground;
{
    DLog();
    [self importItemsAtPaths:[self.spotlightSearchResults valueForKey:@"Path"]];
}

#pragma mark - Controlling Import -

- (void)start
{
    IMPORTDLog(@"%s", BOOL_STR(self.operationQueue.operationCount != 0 && self.status != OEImporterStatusRunning));
    if(self.operationQueue.operationCount != 0 && self.status != OEImporterStatusRunning)
    {
        self.status = OEImporterStatusRunning;
        self.operationQueue.suspended = NO;
        [self OE_performSelectorOnDelegate:@selector(romImporterDidStart:) withObject:self];
    }
}

- (void)pause
{
    DLog(@"%s", BOOL_STR(self.status == OEImporterStatusRunning));
    if(self.status == OEImporterStatusRunning)
    {
        self.status = OEImporterStatusPaused;
        self.operationQueue.suspended = YES;
        [self OE_performSelectorOnDelegate:@selector(romImporterDidPause:) withObject:self];
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

    [self OE_performSelectorOnDelegate:@selector(romImporterDidCancel:) withObject:self];
}

- (void)finish
{
    DLog(@"Finish");

    [self setStatus:OEImporterStatusStopped];
    [self.operationQueue cancelAllOperations];

    self.numberOfProcessedItems = 0;
    self.totalNumberOfItems = 0;
    self.operationQueue.suspended = YES;

    [self OE_performSelectorOnDelegate:@selector(romImporterDidFinish:) withObject:self];
}

#pragma mark - Private Methods -

- (void)setTotalNumberOfItems:(NSInteger)totalNumberOfItems
{
    _totalNumberOfItems = totalNumberOfItems;
    [self OE_performSelectorOnDelegate:@selector(romImporterChangedItemCount:) withObject:nil];
}

- (void)OE_performSelectorOnDelegate:(SEL)selector withObject:(nullable id)object
{
    if(![self.delegate respondsToSelector:selector] ||
       ![self.delegate respondsToSelector:@selector(performSelectorOnMainThread:withObject:waitUntilDone:)])
        return;
    
    NSAssert(protocol_getMethodDescription(@protocol(OEROMImporterDelegate), selector, NO, YES).name == selector,
             @"Unknown delegate method %@", NSStringFromSelector(selector));

    [(NSObject *)self.delegate performSelectorOnMainThread:selector withObject:object waitUntilDone:YES];
}

@end

NS_ASSUME_NONNULL_END
