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

#import "OEBindingsController.h"
#import "OESystemController.h"
#import "OEBindingsController_Internal.h"
#import "OEDeviceManager.h"
#import "OEDeviceHandler.h"

@interface OEBindingsController ()
{
    NSMutableDictionary *systems;
    NSMutableDictionary *systemRepresentations;
    
    BOOL requiresSynchronization;
}

- (void)OE_setupBindingsController;
- (void)OE_setupNotificationObservation;
- (void)OE_applicationDidChangeState:(NSNotification *)notification;

- (void)OE_HIDManagerDidAddDeviceNotification:(NSNotification *)notif;
- (void)OE_HIDManagerDidRemoveDeviceNotification:(NSNotification *)notif;

- (void)OE_registerSystemController:(OESystemController *)aController;

@end

@implementation OEBindingsController
@synthesize configurationName;

static dispatch_queue_t     bindingsControllerQueue  = nil;
static NSMutableDictionary *bindingsControllers      = nil;
static NSMutableSet        *systemControllers        = nil;
static NSString            *configurationsFolderPath = nil;

+ (void)initialize
{
    if(self == [OEBindingsController class])
    {
        bindingsControllerQueue = dispatch_queue_create("org.openemu.system.OEBindingsController", DISPATCH_QUEUE_SERIAL);
        
        systemControllers       = [[NSMutableSet alloc] init];
        bindingsControllers     = [[NSMutableDictionary alloc] initWithCapacity:1];
        
        NSArray  *paths  = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
        
        if([paths count] > 0)
        {
            configurationsFolderPath = [[[paths objectAtIndex:0] stringByAppendingPathComponent:@"OpenEmu"] stringByAppendingPathComponent:@"Bindings Configurations"];
            [[NSFileManager defaultManager] createDirectoryAtPath:configurationsFolderPath withIntermediateDirectories:YES attributes:nil error:NULL];
        }
    }
}

+ (void)registerSystemController:(OESystemController *)aController;
{
    dispatch_sync(bindingsControllerQueue, ^{
        if([systemControllers containsObject:aController]) return;
        
        for(OEBindingsController *bindingsController in [bindingsControllers allValues])
            [bindingsController OE_registerSystemController:aController];
        
        [systemControllers addObject:aController];
    });
}

+ (OEBindingsController *)defaultBindingsController;
{
    static OEBindingsController *sharedSystemUserDefaultsController = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        sharedSystemUserDefaultsController = [[OEBindingsController alloc] init];
    });
    
    return sharedSystemUserDefaultsController;
}

+ (OEBindingsController *)bindingsControllerWithConfigurationName:(NSString *)aName;
{
    return [[self alloc] initWithConfigurationName:aName];
}

+ (NSString *)filePathForConfigurationWithName:(NSString *)aName;
{
    return [[configurationsFolderPath stringByAppendingPathComponent:aName] stringByAppendingPathExtension:@"oebindings"];
}

- (NSString *)filePath;
{
    return [[self class] filePathForConfigurationWithName:[self configurationName]];
}

- (id)init
{
    return [self initWithConfigurationName:nil];
}

- (id)initWithConfigurationName:(NSString *)aName;
{
    if([aName length] == 0) aName = @"Default";
    
    __block OEBindingsController *ret = nil;
    
    dispatch_sync(bindingsControllerQueue, ^{
        ret = [bindingsControllers objectForKey:aName];
    });
    
    if(ret == nil)
    {
        if((self = [super init]))
        {
            dispatch_sync(bindingsControllerQueue, ^{
                configurationName     = [aName copy];
                systemRepresentations = [NSKeyedUnarchiver unarchiveObjectWithFile:[[self class] filePathForConfigurationWithName:aName]];
                [self OE_setupBindingsController];
                [self OE_setupNotificationObservation];
                [bindingsControllers setObject:self forKey:configurationName];
            });
            ret = self;
        }
    }
    
    return ret;
}

- (void)OE_setupNotificationObservation;
{
    NSNotificationCenter *nc = [NSNotificationCenter defaultCenter];
    [nc addObserver:self selector:@selector(OE_applicationDidChangeState:) name:NSApplicationWillResignActiveNotification object:NSApp];
    [nc addObserver:self selector:@selector(OE_applicationDidChangeState:) name:NSApplicationWillTerminateNotification object:NSApp];
    
    [nc addObserver:self selector:@selector(OE_HIDManagerDidAddDeviceNotification:) name:OEDeviceManagerDidAddDeviceHandlerNotification object:nil];
    [nc addObserver:self selector:@selector(OE_HIDManagerDidRemoveDeviceNotification:) name:OEDeviceManagerDidRemoveDeviceHandlerNotification object:nil];
}

- (void)dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}

- (void)OE_applicationDidChangeState:(NSNotification *)notification;
{
    [self synchronize];
}

- (void)OE_HIDManagerDidAddDeviceNotification:(NSNotification *)notif;
{
    OEDeviceHandler *handler = [[notif userInfo] objectForKey:OEDeviceManagerDeviceHandlerUserInfoKey];
    if([handler isKeyboardDevice]) return;
    
    [systems enumerateKeysAndObjectsUsingBlock:
     ^(NSString *key, OESystemBindings *obj, BOOL *stop)
     {
         [obj OE_didAddDeviceHandler:handler];
     }];
}

- (void)OE_HIDManagerDidRemoveDeviceNotification:(NSNotification *)notif;
{
    OEDeviceHandler *handler = [[notif userInfo] objectForKey:OEDeviceManagerDeviceHandlerUserInfoKey];
    if([handler isKeyboardDevice]) return;
    
    [systems enumerateKeysAndObjectsUsingBlock:
     ^(NSString *key, OESystemBindings *obj, BOOL *stop)
     {
         [obj OE_didRemoveDeviceHandler:handler];
     }];
}

- (void)OE_setupBindingsController;
{
    systems = [NSMutableDictionary dictionary];
    
    for(OESystemController *controller in systemControllers)
        [self OE_registerSystemController:controller];
}

- (void)OE_setRequiresSynchronization;
{
    requiresSynchronization = YES;
}

- (BOOL)synchronize;
{
    if(!requiresSynchronization) return YES;
    
    /* systemRepresentations contains all the representations that were stored
     * in the original file but not yet parsed because its OESystemController
     * was not yet registered, so we have to save the already registered
     * system controllers but also keep the unregistered ones.
     */
    NSMutableDictionary *systemReps = [systemRepresentations mutableCopy] ? : [NSMutableDictionary dictionaryWithCapacity:[systems count]];
    
    [systems enumerateKeysAndObjectsUsingBlock:
     ^(NSString *identifier, OESystemBindings *ctrl, BOOL *stop)
     {
         [systemReps setObject:[ctrl OE_dictionaryRepresentation] forKey:identifier];
     }];
    
    if([NSKeyedArchiver archiveRootObject:systemReps toFile:[self filePath]])
    {
        requiresSynchronization = NO;
        return YES;
    }
    
    return NO;
}

- (void)OE_registerSystemController:(OESystemController *)aController;
{
    NSString *identifier = [aController systemIdentifier];
    OESystemBindings *bindingsController = [systems objectForKey:identifier];
    
    if(bindingsController == nil)
    {
        [self willChangeValueForKey:@"systemBindings"];
        
        bindingsController = [[OESystemBindings alloc] OE_initWithBindingsController:self systemController:aController dictionaryRepresentation:[systemRepresentations objectForKey:identifier]];
        
        [systemRepresentations removeObjectForKey:identifier];
        
        for(OEDeviceHandler *handler in [[OEDeviceManager sharedDeviceManager] deviceHandlers])
            if(![handler isKeyboardDevice])
                [bindingsController OE_didAddDeviceHandler:handler];
        
        [systems setObject:bindingsController forKey:identifier];
        
        [self didChangeValueForKey:@"systemBindings"];
    }
}

- (NSArray *)systemBindings
{
    return [systems allValues];
}

- (OESystemBindings *)systemBindingsForSystemIdentifier:(NSString *)systemIdentifier;
{
    return [systems objectForKey:systemIdentifier];
}

- (OESystemBindings *)systemBindingsForSystemController:(OESystemController *)systemController;
{
    [[self class] registerSystemController:systemController];
    
    return [self systemBindingsForSystemIdentifier:[systemController systemIdentifier]];
}

@end
