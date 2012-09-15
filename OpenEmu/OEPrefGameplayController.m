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
#import "OECompositionPlugin.h"
#import "OEGameViewController.h"

@implementation OEPrefGameplayController
@synthesize filterPreviewContainer, filterSelection;

- (void)awakeFromNib
{   
    // Setup plugins menu
	NSArray *filterPlugins = [[OECompositionPlugin allPluginNames] sortedArrayUsingSelector:@selector(caseInsensitiveCompare:)];
    // These filters are loaded and run by GL, and do not rely on QTZs
    NSArray *filterNames = [filterPlugins arrayByAddingObjectsFromArray:OEOpenGLFilterNameArray];
    
	NSMenu *filterMenu = [[NSMenu alloc] init];
    
    for(NSString *aName in filterNames)
		[filterMenu addItemWithTitle:aName action:NULL keyEquivalent:@""];
    
	[[self filterSelection] setMenu:filterMenu];
	
	NSUserDefaults *sud = [NSUserDefaults standardUserDefaults];
	NSString *selectedFilterName = [sud objectForKey:OEGameVideoFilterKey];
    
	if(selectedFilterName != nil && [[self filterSelection] itemWithTitle:selectedFilterName])
		[[self filterSelection] selectItemWithTitle:selectedFilterName];
    else 
		[[self filterSelection] selectItemAtIndex:0];
    
	[self changeFilter:[self filterSelection]];
    
    
    [[self filterPreviewContainer] setWantsLayer:YES];  
    CATransition *awesomeCrossFade = [CATransition animation];
    awesomeCrossFade.type = kCATransitionFade;
    awesomeCrossFade.timingFunction = [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionDefault];
    awesomeCrossFade.duration = 1.0;
    
    [[self filterPreviewContainer] setAnimations:[NSDictionary dictionaryWithObject:awesomeCrossFade forKey:@"subviews"]];

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
    return NSLocalizedString([self title], "");
}

- (NSSize)viewSize
{
	return NSMakeSize(423, 381);
}

#pragma mark -
#pragma mark UI Actions

- (IBAction)changeFilter:(id)sender
{
	NSString *filterName = [[[self filterSelection] selectedItem] title];
    
    OECompositionPlugin *plugin = [OECompositionPlugin compositionPluginWithName:filterName];
    NSImage *filterPreviewImage = (plugin == nil && ![plugin isBuiltIn]
                                   ? [plugin previewImage]
                                   : [[NSBundle mainBundle] imageForResource:[filterName stringByAppendingPathExtension:@"png"]]);

	NSImageView *newPreviewView = [[NSImageView alloc] initWithFrame:(NSRect){ .size = [[self filterPreviewContainer] frame].size }];
    [newPreviewView setImage:filterPreviewImage];
    [newPreviewView setImageAlignment:NSImageAlignCenter];
    [newPreviewView setImageFrameStyle:NSImageFrameNone];
    [newPreviewView setImageScaling:NSImageScaleNone];
    
    NSView *currentImageView = [[[self filterPreviewContainer] subviews] lastObject];
    
    if(currentImageView != nil)
        [[[self filterPreviewContainer] animator] replaceSubview:currentImageView with:newPreviewView];
    else
        [[self filterPreviewContainer] addSubview:newPreviewView];
    
	[[NSUserDefaults standardUserDefaults] setObject:filterName forKey:OEGameVideoFilterKey];
}

@end
