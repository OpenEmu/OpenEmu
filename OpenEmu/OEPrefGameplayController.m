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
#import "OEPlugin.h"
#import "OEShaderPlugin.h"
#import "OEGameViewController.h"
#import "OEDBSystem.h"
#import "OELibraryDatabase.h"

@implementation OEPrefGameplayController
@synthesize filterSelection;

- (void)awakeFromNib
{
    // Setup plugins menu
    NSMutableSet   *filterSet     = [NSMutableSet set];
    NSMutableArray *filterPlugins = [NSMutableArray array];
    [filterSet addObjectsFromArray:[OEShaderPlugin allPluginNames]];
    [filterSet filterUsingPredicate:[NSPredicate predicateWithFormat:@"NOT SELF beginswith '_'"]];
    [filterPlugins addObjectsFromArray:[filterSet allObjects]];
    [filterPlugins sortUsingSelector:@selector(caseInsensitiveCompare:)];

	NSMenu *filterMenu = [[NSMenu alloc] init];

    for(NSString *aName in filterPlugins)
		[filterMenu addItemWithTitle:aName action:NULL keyEquivalent:@""];

	[[self filterSelection] setMenu:filterMenu];

	NSUserDefaults *sud = [NSUserDefaults standardUserDefaults];
	NSString *selectedFilterName = [sud objectForKey:OEGameDefaultVideoFilterKey];

    // Set Nearest Neighbor as default filter if the current one is not available (ie. deleted)
    if(![filterPlugins containsObject:selectedFilterName])
    {
        selectedFilterName = @"Nearest Neighbor";
        [sud setObject:@"Nearest Neighbor" forKey:OEGameDefaultVideoFilterKey];
    }

	if(selectedFilterName != nil && [[self filterSelection] itemWithTitle:selectedFilterName])
		[[self filterSelection] selectItemWithTitle:selectedFilterName];
    else
		[[self filterSelection] selectItemAtIndex:0];
}

#pragma mark ViewController Overrides

- (NSString *)nibName
{
	return @"OEPrefGameplayController";
}

#pragma mark OEPreferencePane Protocol

- (NSImage *)icon
{
	return [NSImage imageNamed:@"gameplay_tab_icon"];
}

- (NSString *)title
{
	return @"Gameplay";
}

- (NSString *)localizedTitle
{
    return NSLocalizedString([self title], @"Preferences: Gamplay Toolbar Item");
}

- (NSSize)viewSize
{
	return NSMakeSize(423, 282);
}

#pragma mark -
#pragma mark UI Actions

- (IBAction)changeFilter:(id)sender
{
    OELibraryDatabase *database = [OELibraryDatabase defaultDatabase];
    NSManagedObjectContext *context = [database mainThreadContext];
    NSString *filterName = [[[self filterSelection] selectedItem] title];
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    NSArray *allSystemIdentifiers = [OEDBSystem allSystemIdentifiersInContext:context];
    
    for(OECorePlugin *systemIdentifiers in allSystemIdentifiers)
    {
        [defaults removeObjectForKey:[NSString stringWithFormat:OEGameSystemVideoFilterKeyFormat, systemIdentifiers]];
    }

	[defaults setObject:filterName forKey:OEGameDefaultVideoFilterKey];
}

@end
