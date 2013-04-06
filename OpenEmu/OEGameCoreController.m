/*
 Copyright (c) 2009, OpenEmu Team

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

#import "OEGameCoreController.h"
#import "OEAbstractAdditions.h"
#import "NSApplication+OEHIDAdditions.h"
#import "OEGameCore.h"
#import "OEHIDEvent.h"
#import "NSUserDefaultsController+OEEventAdditions.h"
#import <objc/runtime.h>

NSString *const OEAdvancedPreferenceKey  = @"OEAdvancedPreferenceKey";
NSString *const OEGameCoreClassKey       = @"OEGameCoreClass";
NSString *const OEGameCorePlayerCountKey = @"OEGameCorePlayerCount";

NSString *OEEventNamespaceKeys[] = { @"", @"OEGlobalNamespace", @"OEKeyboardNamespace", @"OEHIDNamespace", @"OEMouseNamespace", @"OEOtherNamespace" };


@interface OEGameCoreController ()
{
    NSMutableArray *_gameDocuments;
}

@end

@implementation OEGameCoreController

- (id)init
{
    return [self initWithBundle:[NSBundle bundleForClass:[self class]]];
}

- (id)initWithBundle:(NSBundle *)aBundle;
{
    if((self = [super init]))
    {
        _bundle        = aBundle;
        _pluginName    = ([[_bundle infoDictionary] objectForKey:@"CFBundleExecutable"] ? :
                          [[_bundle infoDictionary] objectForKey:@"CFBundleName"]);
        _gameCoreClass = NSClassFromString([[_bundle infoDictionary] objectForKey:OEGameCoreClassKey]);
        _playerCount   = [[[_bundle infoDictionary] objectForKey:OEGameCorePlayerCountKey] integerValue];

        NSArray  *paths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
        NSString *basePath = ([paths count] > 0) ? [paths objectAtIndex:0] : NSTemporaryDirectory();

        NSString *supportFolder = [basePath stringByAppendingPathComponent:@"OpenEmu"];
        _supportDirectoryPath   = [supportFolder stringByAppendingPathComponent:_pluginName];

        _gameDocuments = [[NSMutableArray alloc] init];
    }
    return self;
}

- (NSArray *)systemIdentifiers
{
	return [[[self bundle] infoDictionary] objectForKey:@"OESystemIdentifiers"];
}

- (void)dealloc
{
    [_gameDocuments makeObjectsPerformSelector:@selector(close)];
}

- (BOOL)acceptsFirstResponder
{
    return YES;
}

- (OEGameCore *)newGameCore
{
    return [[[self gameCoreClass] alloc] init];
}

@end
