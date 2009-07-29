//
//  OESaveStateController.m
//  OpenEmu
//
//  Created by Joshua Weinberg on 7/28/09.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import "OESaveStateController.h"
#import "GameDocumentController.h"

@implementation OESaveStateController


@synthesize plugins, availablePluginsPredicate, selectedPlugins, docController, selectedRomPredicate;

- (id)init
{
    self = [super initWithWindowNibName:@"SaveStateManager"];
    if(self != nil)
    {
		self.docController = [GameDocumentController sharedDocumentController];
		self.selectedRomPredicate = nil;
    }
    return self;
}

- (NSArray *)plugins
{
    return [[GameDocumentController sharedDocumentController] plugins];
}

- (void)dealloc
{
	self.docController = nil;
    [selectedPlugins release];
    [super dealloc];
}

- (void)setSelectedPlugins:(NSIndexSet *)indexes
{
    [selectedPlugins release];
    NSUInteger index = [indexes firstIndex];
    
    if(indexes != nil && index < [[pluginController arrangedObjects] count] && index != NSNotFound)
    {
        currentPlugin = [[pluginController selectedObjects] objectAtIndex:0];
        selectedPlugins = [[NSIndexSet alloc] initWithIndex:index];
		self.selectedRomPredicate = [NSPredicate predicateWithFormat:@"%K matches[c] %@",  @"emulatorID", [currentPlugin displayName] ];
    }
    else
    {
        selectedPlugins = [[NSIndexSet alloc] init];
        currentPlugin = nil;
    }
}

@end
