//
//  OEPrefGameplayController.m
//  OpenEmu
//
//  Created by Christoph Leimbrock on 07.06.11.
//  Copyright 2011 none. All rights reserved.
//

#import "OEPrefGameplayController.h"
#import "OEPlugin.h"
#import "OECompositionPlugin.h"
@implementation OEPrefGameplayController

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) 
    {
    }
    return self;
}

- (void)dealloc
{
    [super dealloc];
}

- (void)awakeFromNib
{
    // Setup plugins menu
	NSArray *filterPlugins = [[OECompositionPlugin allPluginNames] sortedArrayUsingSelector:@selector(caseInsensitiveCompare:)];
    // These filters are loaded and run by GL, and do not rely on QTZs
    NSArray* filterNames = [filterPlugins arrayByAddingObjectsFromArray:
                            [NSArray arrayWithObjects:
                             @"Linear",
                             @"Nearest Neighbor",
                             @"Scale2xHQ",
                             @"Scale2xPlus",
                             @"Scale4x",
                             @"Scale4xHQ",
                             nil]];
	NSMenu* filterMenu = [[NSMenu alloc] init];
    for(NSString* aName in filterNames)
    {
		[filterMenu addItemWithTitle:aName action:NULL keyEquivalent:@""];
	}
	[filterSelection setMenu:filterMenu];
	[filterMenu release];	
	
	NSUserDefaults* sud = [NSUserDefaults standardUserDefaults];
	NSString* selectedFilterName = [sud objectForKey:UDVideoFilterKey];
	if(selectedFilterName && [filterSelection itemWithTitle:selectedFilterName])
    {
		[filterSelection selectItemWithTitle:selectedFilterName];
	} 
    else 
    {
		[filterSelection selectItemAtIndex:0];
	}
	[self changeFilter:filterSelection];
}
#pragma mark ViewController Overrides
- (NSString*)nibName
{
	return @"OEPrefGameplayController";
}

#pragma mark OEPreferencePane Protocol
- (NSImage*)icon
{
	return [NSImage imageNamed:@"gameplay_tab_icon"];
}

- (NSString*)title
{
	return @"Gameplay";
}

- (NSSize)viewSize
{
	return NSMakeSize(423, 368);
}
#pragma mark -
#pragma mark UI Actions
- (IBAction)changeFilter:(id)sender
{
	NSString* filterName =  [[filterSelection selectedItem] title];
    // TODO: check if filter is loaded externaly -> use an image from there
	// else ...
	NSImage* filterPreviewImage = [[NSBundle mainBundle] imageForResource:[NSString stringWithFormat:@"%@.png", filterName]];
    [filterPreviewView setImage:filterPreviewImage];
	
	NSUserDefaults* sud = [NSUserDefaults standardUserDefaults];
	[sud setObject:filterName forKey:UDVideoFilterKey];
}
@end
