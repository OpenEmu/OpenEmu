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
    NSArray *filterNames = [filterPlugins arrayByAddingObjectsFromArray:
                            [NSArray arrayWithObjects:
                             @"Linear",
                             @"Nearest Neighbor",
                             @"Scale2xHQ",
                             @"Scale2xPlus",
                             @"Scale4x",
                             @"Scale4xHQ",
                             nil]];
	NSMenu *filterMenu = [[NSMenu alloc] init];
    for(NSString *aName in filterNames)
    {
		[filterMenu addItemWithTitle:aName action:NULL keyEquivalent:@""];
	}
	[[self filterSelection] setMenu:filterMenu];
	[filterMenu release];	
	
	NSUserDefaults *sud = [NSUserDefaults standardUserDefaults];
	NSString *selectedFilterName = [sud objectForKey:UDVideoFilterKey];
	if(selectedFilterName && [[self filterSelection] itemWithTitle:selectedFilterName])
    {
		[[self filterSelection] selectItemWithTitle:selectedFilterName];
	} 
    else 
    {
		[[self filterSelection] selectItemAtIndex:0];
	}
	[self changeFilter:[self filterSelection]];
    
    
    [[self filterPreviewContainer] setWantsLayer:YES];  
    CATransition *awesomeCrossFade = [CATransition animation];
    awesomeCrossFade.type = kCATransitionFade;
    awesomeCrossFade.timingFunction = [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionDefault];
    awesomeCrossFade.duration = 1.0;
    
    [[self filterPreviewContainer] setAnimations:[NSDictionary dictionaryWithObject:awesomeCrossFade forKey:@"subviews"]];

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
	return NSMakeSize(423, 364);
}
#pragma mark -
#pragma mark UI Actions
- (IBAction)changeFilter:(id)sender
{
	NSString *filterName =  [[[self filterSelection] selectedItem] title];
    
    OECompositionPlugin *plugin = [OECompositionPlugin compositionPluginWithName:filterName];
    NSImage *filterPreviewImage;
    if(plugin && ![plugin isBuiltIn])
        filterPreviewImage = [plugin previewImage];
    else
        filterPreviewImage = [[NSBundle mainBundle] imageForResource:[NSString stringWithFormat:@"%@.png", filterName]];

	NSImageView *newPreviewView = [[NSImageView alloc] initWithFrame:(NSRect){{0,0}, [[self filterPreviewContainer] frame].size}];
    [newPreviewView setImage:filterPreviewImage];
    [newPreviewView setImageAlignment:NSImageAlignCenter];
    [newPreviewView setImageFrameStyle:NSImageFrameNone];
    [newPreviewView setImageScaling:NSImageScaleNone];
    NSView *currentImageView = [[[self filterPreviewContainer] subviews] lastObject];
    if(currentImageView)
    {  
        [[[self filterPreviewContainer] animator] replaceSubview:currentImageView with:newPreviewView];

    }
    else
    {
        [[self filterPreviewContainer] addSubview:newPreviewView];
    }
    [newPreviewView release];
    
	NSUserDefaults *sud = [NSUserDefaults standardUserDefaults];
	[sud setObject:filterName forKey:UDVideoFilterKey];
}
@synthesize filterPreviewContainer, filterSelection;
@end
