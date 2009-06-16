//
//  OEGamePreferenceController.m
//  OpenEmu
//
//  Created by Remy Demarest on 25/02/2009.
//  Copyright 2009 Psycho Inc.. All rights reserved.
//

#import "OEGamePreferenceController.h"
#import "GameDocumentController.h"
#import "OEGamePreferenceController_Toolbar.h"
#import "OECorePlugin.h"
#import "OEGameCoreController.h"

@implementation OEGamePreferenceController

@synthesize plugins, selectedPlugins, availablePluginsPredicate;

- (id)init
{
    self = [super initWithWindowNibName:@"GamePreferences"];
    if(self != nil)
    {
        plugins = [[GameDocumentController sharedDocumentController] plugins];
        
        [self setupToolbar];
    }
    return self;
}

- (id)initWithWindowNibName:(NSString *)aName
{
    return [self init];
}

- (void)awakeFromNib
{
    [self switchView:self];
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
    
    if(index < [plugins count] && index != NSNotFound)
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
