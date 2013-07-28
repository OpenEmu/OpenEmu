//
//  OpenEmuDOHelperApp.m
//  OpenEmu
//
//  Created by Remy Demarest on 20/07/2013.
//
//

#import "OpenEmuDOHelperApp.h"
#import "OESystemPlugin.h"

NSString *const OEHelperServerNamePrefix = @"org.openemu.OpenEmuHelper-";

@implementation OpenEmuDOHelperApp
{
    id<OEDOGameCoreDisplayHelper> _displayHelper;

    NSConnection *_connection;
    NSString     *_serviceUUID;
}

- (void)launchApplication
{
    _serviceUUID = [[[NSProcessInfo processInfo] arguments] objectAtIndex:1];

    NSApplication *application = [NSApplication sharedApplication];
    [application setDelegate:self];
    [application run];
}

- (void)applicationDidFinishLaunching:(NSNotification *)notification
{
    [super applicationDidFinishLaunching:notification];

    _connection = [[NSConnection alloc] init];
    [_connection setRootObject:self];

    [_connection registerName:[OEHelperServerNamePrefix stringByAppendingString:_serviceUUID]];
}

- (oneway void)setupEmulationWithDelegate:(byref id<OEDOGameCoreHelperDelegate>)delegate;
{
    [self setupEmulationWithCompletionHandler:
     ^{
         [delegate gameCoreHelperDidSetupEmulation];
     }];
}

- (oneway void)resetEmulationWithDelegate:(byref id<OEDOGameCoreHelperDelegate>)delegate;
{
    [self resetEmulationWithCompletionHandler:
     ^{
         [delegate gameCoreHelperDidResetEmulation];
     }];
}

- (oneway void)stopEmulationWithDelegate:(byref id<OEDOGameCoreHelperDelegate>)delegate;
{
    [self stopEmulationWithCompletionHandler:
     ^{
         [delegate gameCoreHelperDidStopEmulation];

         [[NSApplication sharedApplication] terminate:self];
     }];
}

- (oneway void)saveStateToFileAtPath:(NSString *)fileName withDelegate:(byref id<OEDOGameCoreHelperDelegate>)delegate;
{
    [self saveStateToFileAtPath:fileName completionHandler:
     ^(BOOL success, NSError *error)
     {
         [delegate gameCoreHelperDidSaveState:success error:error];
     }];
}

- (oneway void)loadStateFromFileAtPath:(NSString *)fileName withDelegate:(byref id<OEDOGameCoreHelperDelegate>)delegate;
{
    [self loadStateFromFileAtPath:fileName completionHandler:
     ^(BOOL success, NSError *error)
     {
         [delegate gameCoreHelperDidLoadState:success error:error];
     }];
}

- (oneway void)loadROMAtPath:(bycopy NSString *)romPath usingCorePluginAtPath:(bycopy NSString *)corePluginPath systemPluginAtPath:(bycopy NSString *)systemPluginPath withDelegate:(byref id<OEDOGameCoreHelperDelegate>)delegate displayHelper:(byref id<OEDOGameCoreDisplayHelper>)displayHelper;
{
    [(NSDistantObject *)delegate setProtocolForProxy:@protocol(OEDOGameCoreDisplayHelper)];
    _displayHelper = displayHelper;

    if([self loadROMAtPath:romPath withCorePluginAtPath:corePluginPath systemIdentifier:[[OESystemPlugin systemPluginWithBundleAtPath:systemPluginPath] systemIdentifier]])
        [delegate setSystemResponderClient:[self gameCore]];
}

- (void)updateEnableVSync:(BOOL)enable;
{
    [_displayHelper setEnableVSync:enable];
}

- (void)updateScreenSize:(OEIntSize)newScreenSize withIOSurfaceID:(IOSurfaceID)newSurfaceID;
{
    [_displayHelper setScreenSize:newScreenSize withIOSurfaceID:newSurfaceID];
}

- (void)updateAspectSize:(OEIntSize)newAspectSize withIOSurfaceID:(IOSurfaceID)newSurfaceID;
{
    [_displayHelper setAspectSize:newAspectSize withIOSurfaceID:newSurfaceID];
}

- (void)updateScreenRect:(OEIntRect)newScreenRect;
{
    [_displayHelper setScreenRect:newScreenRect];
}

- (void)updateFrameInterval:(NSTimeInterval)newFrameInterval;
{
    [_displayHelper setFrameInterval:newFrameInterval];
}

@end
