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

#import "OEPrefGameplayController.h"
@import OpenEmuKit;
#import "OEGameViewController.h"
#import "OEDBSystem.h"
#import "OELibraryDatabase.h"
#import "OpenEmu-Swift.h"

@implementation OEPrefGameplayController
{
    id<NSObject> _token;
}

@synthesize globalDefaultShaderSelection;

- (void)awakeFromNib
{
    [self loadShaderMenu];
}

- (void)loadShaderMenu
{
    // Setup shaders menu
    NSArray *systemShaders = [OEShadersModel.shared.systemShaderNames sortedArrayUsingSelector:@selector(caseInsensitiveCompare:)];

    NSMenu *globalShaderMenu = [NSMenu new];

    for(NSString *aName in systemShaders)
        [globalShaderMenu addItemWithTitle:aName action:NULL keyEquivalent:@""];
    
    NSArray *customShaders = [OEShadersModel.shared.customShaderNames sortedArrayUsingSelector:@selector(caseInsensitiveCompare:)];
    if (customShaders.count > 0) {
        [globalShaderMenu addItem:[NSMenuItem separatorItem]];
        for(NSString *aName in customShaders)
            [globalShaderMenu addItemWithTitle:aName action:NULL keyEquivalent:@""];
    }

    self.globalDefaultShaderSelection.menu = globalShaderMenu;

    NSUserDefaults *defaults = NSUserDefaults.standardUserDefaults;
    NSString *selectedShaderName = [defaults objectForKey:OEGameDefaultVideoShaderKey];

    // Set Pixellate as default shader if the current one is not available (ie. deleted)
    if(![systemShaders containsObject:selectedShaderName] && ![customShaders containsObject:selectedShaderName])
    {
        selectedShaderName = @"Pixellate";
        [defaults setObject:@"Pixellate" forKey:OEGameDefaultVideoShaderKey];
    }

    if(selectedShaderName != nil && [self.globalDefaultShaderSelection itemWithTitle:selectedShaderName])
        [self.globalDefaultShaderSelection selectItemWithTitle:selectedShaderName];
    else
        [self.globalDefaultShaderSelection selectItemAtIndex:0];
}

#pragma mark ViewController Overrides

- (NSString *)nibName
{
	return @"OEPrefGameplayController";
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    __weak typeof(self) weakSelf = self;
    _token = [NSNotificationCenter.defaultCenter addObserverForName:OEShadersModel.shaderModelCustomShadersDidChange
                                                             object:nil
                                                              queue:NSOperationQueue.mainQueue
                                                         usingBlock:
              ^(NSNotification * _Nonnull note) {
        [weakSelf loadShaderMenu];
    }];
}

- (void)viewWillDisappear
{
    [super viewWillDisappear];
    
    if (_token)
    {
        [NSNotificationCenter.defaultCenter removeObserver:_token];
        _token = nil;
    }
}

#pragma mark OEPreferencePane Protocol

- (NSImage *)icon
{
	return [NSImage imageNamed:@"gameplay_tab_icon"];
}

- (NSString *)panelTitle
{
	return @"Gameplay";
}

- (NSString *)localizedPanelTitle
{
    return NSLocalizedString([self panelTitle], @"Preferences: Gamplay Toolbar Item");
}

- (NSSize)viewSize
{
	return self.view.fittingSize;
}

#pragma mark -
#pragma mark UI Actions

- (IBAction)changeGlobalDefaultShader:(id)sender
{
    OELibraryDatabase *database = OELibraryDatabase.defaultDatabase;
    NSManagedObjectContext *context = database.mainThreadContext;
    NSString *shaderName = self.globalDefaultShaderSelection.selectedItem.title;
    NSUserDefaults *defaults = NSUserDefaults.standardUserDefaults;
    NSArray *allSystemIdentifiers = [OEDBSystem allSystemIdentifiersInContext:context];

    for(OECorePlugin *systemIdentifiers in allSystemIdentifiers)
    {
        [defaults removeObjectForKey:[NSString stringWithFormat:OEGameSystemVideoShaderKeyFormat, systemIdentifiers]];
    }

	[defaults setObject:shaderName forKey:OEGameDefaultVideoShaderKey];
}

@end
