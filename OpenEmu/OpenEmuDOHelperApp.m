/*
 Copyright (c) 2013, OpenEmu Team
 
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

#import "OpenEmuDOHelperApp.h"
#import "OESystemPlugin.h"

NSString *const OEHelperServerNamePrefix = @"org.openemu.OpenEmuHelper-";

@implementation OpenEmuDOHelperApp
{
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
     ^(IOSurfaceID surfaceID, OEIntSize screenSize, OEIntSize aspectSize)
     {
         [delegate gameCoreHelperDidSetupEmulationWithIOSurfaceID:surfaceID screenSize:screenSize aspectSize:aspectSize];
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
    [self setDisplayHelper:(id<OEGameCoreDisplayHelper>)displayHelper];

    if([self loadROMAtPath:romPath withCorePluginAtPath:corePluginPath systemIdentifier:[[OESystemPlugin systemPluginWithBundleAtPath:systemPluginPath] systemIdentifier]])
        [delegate setSystemResponderClient:[self gameCore]];
}

@end
