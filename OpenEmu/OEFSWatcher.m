/*
 Copyright (c) 2010, OpenEmu Team
 
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

#import "OEFSWatcher.h"

@interface OEFSWatcher ()
- (id)initWithPersistentKey:(NSString*)key;
@property           FSEventStreamRef            stream;

- (void)OE_updateLastEventID:(uint64_t)eventID;
@property (strong) NSNumber *lastEventID;
@property (copy)   NSString *persistentKey;

- (void)OE_setupRestartingProperties;
- (void)OE_removeRestartingProperties;
- (NSArray*)OE_restartingPropertyKeys;

void OEFSWatcher_callback(ConstFSEventStreamRef streamRef,
                     void *userData,
                     size_t numEvents,
                     void *eventPaths,
                     const FSEventStreamEventFlags eventFlags[],
                     const FSEventStreamEventId eventIDs[]);
@end

@implementation OEFSWatcher
@synthesize stream;

+ (id)persistentWatcherWithKey:(NSString*)key forPath:(NSString*)path withBlock:(OEFSBlock)block
{
    OEFSWatcher *watcher = [[OEFSWatcher alloc] initWithPersistentKey:key];
    
    [watcher setPath:path];
    [watcher setCallbackBlock:block];
    
    return watcher;    
}

+ (id)watcherForPath:(NSString*)path withBlock:(OEFSBlock)block
{
    OEFSWatcher *watcher = [[OEFSWatcher alloc] initWithPersistentKey:nil];
    
    [watcher setPath:path];
    [watcher setCallbackBlock:block];
    
    return watcher;
}

#pragma mark -
- (id)initWithPersistentKey:(NSString*)key
{
    self = [super init];
    if (self) {
        NSUserDefaults  *defaults = [NSUserDefaults standardUserDefaults];
        NSNumber        *eventID  = key && [defaults valueForKey:key] ? [defaults valueForKey:key] : [NSNumber numberWithUnsignedLongLong:kFSEventStreamEventIdSinceNow];
        [self setLastEventID:eventID];
        [self setPersistentKey:key];
    }
    return self;
}

- (void)dealloc
{
    [self stopWatching];
}

#pragma mark -
- (void)startWatching
{
    [self restartWatching];
}

- (void)restartWatching
{
    if(stream != NULL)
        [self stopWatching];
    
    FSEventStreamContext context       = {0, (__bridge void *)self, NULL, NULL, NULL};
    CFArrayRef           paths         = (__bridge CFArrayRef)[NSArray arrayWithObject:path];
    uint64_t             lastEventID_t = [[self lastEventID] unsignedLongLongValue];
    
	stream = FSEventStreamCreate(NULL,
	                             &OEFSWatcher_callback,
	                             &context,
	                             paths,
                                 lastEventID_t,
	                             [self delay],
	                             kFSEventStreamCreateFlagUseCFTypes|kFSEventStreamCreateFlagIgnoreSelf|kFSEventStreamCreateFlagFileEvents
                                 );
    
	FSEventStreamScheduleWithRunLoop(stream, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
	FSEventStreamStart(stream);
    
    [self OE_setupRestartingProperties];
}

void OEFSWatcher_callback(ConstFSEventStreamRef streamRef,
                          void *userData,
                          size_t numEvents,
                          void *eventPaths,
                          const FSEventStreamEventFlags eventFlags[],
                          const FSEventStreamEventId eventIDs[])
{
    OEFSWatcher *watcher = (__bridge OEFSWatcher *)userData;
	NSArray     *paths   = (__bridge NSArray*)eventPaths;
    [paths enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
        [watcher OE_updateLastEventID:eventIDs[idx]];
        [watcher callbackBlock](obj, eventFlags[idx]);
    }];
}

- (void)stopWatching
{
    if(stream == NULL)
        return;
    
    if([self persistentKey])
    {
        NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
        [defaults setObject:[self lastEventID] forKey:[self persistentKey]];
        [defaults synchronize];
    }
    
    FSEventStreamStop(stream);
    FSEventStreamInvalidate(stream);
    
    stream = NULL;
    
    [self OE_removeRestartingProperties];
}
#pragma mark -
#pragma mark Config
@synthesize path;
@synthesize delay;
@synthesize callbackBlock;
@synthesize streamFlags;

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    if(stream != NULL)
    {
        [self restartWatching];
    }
}

- (void)OE_setupRestartingProperties
{
    [[self OE_restartingPropertyKeys] enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
        [self addObserver:self forKeyPath:obj options:0 context:nil]; 
    }];
}

- (void)OE_removeRestartingProperties
{
    [[self OE_restartingPropertyKeys] enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
        [self removeObserver:self forKeyPath:obj];
    }];
}

- (NSArray*)OE_restartingPropertyKeys
{
    return [NSArray arrayWithObjects:@"path", @"delay", @"callbackBlock", @"streamFlags", nil];
}
#pragma mark -
@synthesize persistentKey;
@synthesize lastEventID;
- (void)OE_updateLastEventID:(uint64_t)eventID
{
    [self setLastEventID:[NSNumber numberWithUnsignedLongLong:eventID]];
}
@end
