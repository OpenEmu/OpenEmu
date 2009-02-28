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
#import "PluginInfo.h"
#import "OEGameCoreController.h"

@implementation OEGamePreferenceController

@synthesize plugins, selectedPlugins;

- (id)init
{
    self = [super initWithWindowNibName:@"GamePreferences"];
    if(self != nil)
    {
        plugins = [[GameDocumentController sharedDocumentController] plugins];
    }
    return self;
}

- (id)initWithWindowNibName:(NSString *)aName
{
    return [self init];
}

- (void)awakeFromNib
{
    [controlsView retain];
    [videoView retain];
    [pluginsView retain];
    [audioView retain];
    [self setupToolbarForWindow:[self window]];
    [self switchView:self];
}

- (void) dealloc
{
    [controlsView release];
    [pluginsView  release];
    [videoView    release];
    [audioView    release];
    [super        dealloc];
}

- (void)setSelectedPlugins:(NSIndexSet *)indexes
{
    selectedPlugins = indexes;
    if(currentPlugin != nil)
    {
        [[[[currentPlugin controller] controlsPreferences] view] removeFromSuperview];
    }
    NSUInteger index = [indexes firstIndex];
    if(index < [plugins count])
    {
        currentPlugin = [plugins objectAtIndex:index];
        NSView *view = [[[currentPlugin controller] controlsPreferences] view];
        NSLog(@"%@", view);
        NSRect ctrlFrame = [controlsView frame];
        NSRect frame = [view frame];
        frame.origin.x = ctrlFrame.size.width - frame.size.width;
        frame.origin.y = ctrlFrame.size.height - frame.size.height;
        [view setFrame:frame];
        [controlsView addSubview:view];
    }
}

@end
