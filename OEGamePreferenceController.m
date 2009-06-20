/*
 Copyright (c) 2009, OpenEmu Team
 All rights reserved.
 
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

#import "OEGamePreferenceController.h"
#import "GameDocumentController.h"
#import "OEGamePreferenceController_Toolbar.h"
#import "OECorePlugin.h"
#import "OEGameCoreController.h"

@implementation OEGamePreferenceController

@dynamic plugins;
@synthesize selectedPlugins, availablePluginsPredicate;

- (id)init
{
    self = [super initWithWindowNibName:@"GamePreferences"];
    if(self != nil)
    {
        [self setupToolbar];
    }
    return self;
}

- (id)initWithWindowNibName:(NSString *)aName
{
    return [self init];
}

- (NSSize)windowWillResize:(NSWindow *)window toSize:(NSSize)proposedFrameSize
{
    NSLog(@"%s %@", __FUNCTION__, NSStringFromSize(proposedFrameSize));
    return proposedFrameSize;
}

- (void)windowDidBecomeKey:(NSNotification *)notification
{
    NSLog(@"%s %@", __FUNCTION__, NSStringFromRect([[self window] frame]));
    //[self switchView:self];
}

- (void)windowDidResize:(NSNotification *)notification
{
    NSLog(@"%s %@", __FUNCTION__, NSStringFromRect([[self window] frame]));
}

- (NSArray *)plugins
{
    return [[GameDocumentController sharedDocumentController] plugins];
}

- (void) dealloc
{
    [selectedPlugins release];
    [preferencePanels release];
    [currentViewController release];
    [super dealloc];
}

- (void)setSelectedPlugins:(NSIndexSet *)indexes
{
    [selectedPlugins release];
    NSUInteger index = [indexes firstIndex];
    
    if(indexes != nil && index < [[self plugins] count] && index != NSNotFound)
    {
        currentPlugin = [[pluginController selectedObjects] objectAtIndex:0];
        selectedPlugins = [[NSIndexSet alloc] initWithIndex:index];
    }
    else
    {
        selectedPlugins = [[NSIndexSet alloc] init];
        currentPlugin = nil;
    }
    
    [self switchView:nil];
}

@end
