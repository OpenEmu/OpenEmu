//
//  OEGamePreferenceController_Toolbar.m
//  OpenEmu
//
//  Created by Remy Demarest on 25/02/2009.
//  Copyright 2009 Psycho Inc.. All rights reserved.
//

#import "OEGamePreferenceController_Toolbar.h"
#import "PluginInfo.h"

//static NSString *OEPreferenceToolbarIdentifier     = @"OEPreferenceToolbarIdentifier";
static NSString *OEVideoSoundToolbarItemIdentifier = @"OEVideoSoundToolbarItemIdentifier";
static NSString *OEControlsToolbarItemIdentifier   = @"OEControlsToolbarItemIdentifier";
static NSString *OEPluginsToolbarItemIdentifier    = @"OEPluginsToolbarItemIdentifier";

@implementation OEGamePreferenceController (Toolbar)

// ============================================================
// NSToolbar Related Methods
// ============================================================
- (void)setupToolbarForWindow:(NSWindow *)theWindow
{
    /*
    NSToolbar *toolbar = [[[NSToolbar alloc] initWithIdentifier:OEPreferenceToolbarIdentifier] autorelease];
    
    [toolbar setAllowsUserCustomization:NO];
    [toolbar setAutosavesConfiguration:NO];
    [toolbar setDisplayMode:NSToolbarDisplayModeIconAndLabel];
    [toolbar setDelegate:self];
    [theWindow setToolbar:toolbar];
    */
}

- (NSArray *)toolbarAllowedItemIdentifiers:(NSToolbar *)toolbar
{
    static NSArray *standardItems = nil;
    if(standardItems == nil)
    {
        standardItems = [[NSArray arrayWithObjects:OEVideoSoundToolbarItemIdentifier, OEControlsToolbarItemIdentifier, OEPluginsToolbarItemIdentifier, nil] retain];
    }
    
    return standardItems;
}

- (NSArray *)toolbarDefaultItemIdentifiers:(NSToolbar *)toolbar
{
    return [self toolbarAllowedItemIdentifiers:toolbar];
}

- (NSToolbarItem *)toolbar:(NSToolbar *)toolbar itemForItemIdentifier:(NSString *)itemIdentifier willBeInsertedIntoToolbar:(BOOL)flag
{
    // Required delegate method:  Given an item identifier, this method returns an item 
    // The toolbar will use this method to obtain toolbar items that can be displayed in the customization sheet, or in the toolbar itself 
    NSToolbarItem *toolbarItem = [[[NSToolbarItem alloc] initWithItemIdentifier:itemIdentifier] autorelease];
	
    if(itemIdentifier == OEVideoSoundToolbarItemIdentifier)
    {
        // Set the text label to be displayed in the toolbar and customization palette 
        [toolbarItem setLabel:@"Video & Sound"];
        [toolbarItem setPaletteLabel:@"Video & Sound"];
        
        // Set up a reasonable tooltip, and image   Note, these aren't localized, but you will likely want to localize many of the item's properties 
        [toolbarItem setToolTip:@"Video & Sound Preferences"];
        [toolbarItem setImage:[NSImage imageNamed: NSImageNameComputer]];
		
        // Tell the item what message to send when it is clicked 
        [toolbarItem setTarget:self];
        [toolbarItem setAction:@selector(switchView:)];
		
    }
    else if(itemIdentifier == OEControlsToolbarItemIdentifier)
    {
        // Set the text label to be displayed in the toolbar and customization palette 
        [toolbarItem setLabel: @"Controls"];
        [toolbarItem setPaletteLabel: @"Controls"];
        
        // Set up a reasonable tooltip, and image   Note, these aren't localized, but you will likely want to localize many of the item's properties 
        [toolbarItem setToolTip: @"Control Preferences"];
        [toolbarItem setImage: [NSImage imageNamed: NSImageNamePreferencesGeneral]];
        
        // Tell the item what message to send when it is clicked 
        [toolbarItem setTarget:self];
        [toolbarItem setAction:@selector(switchView:)];
    }
    /* Video and Sound are merged
    else if([itemIdent isEqual:AudioToolbarItemIdentifier]) {		
        // Set the text label to be displayed in the toolbar and customization palette 
        [toolbarItem setLabel: @"Audio"];
        [toolbarItem setPaletteLabel: @"Audio"];
        
        // Set up a reasonable tooltip, and image   Note, these aren't localized, but you will likely want to localize many of the item's properties 
        [toolbarItem setToolTip: @"Audio Preferences"];
        [toolbarItem setImage: [NSImage imageNamed: NSImageNameAdvanced]];
        
        // Tell the item what message to send when it is clicked 
        [toolbarItem setTarget: self];
        [toolbarItem setAction: @selector(switchView:)];
	}*/
    else if (itemIdentifier == OEPluginsToolbarItemIdentifier) {		
        // Set the text label to be displayed in the toolbar and customization palette 
        [toolbarItem setLabel: @"Plugins"];
        [toolbarItem setPaletteLabel: @"Plugins"];
        
        // Set up a reasonable tooltip, and image   Note, these aren't localized, but you will likely want to localize many of the item's properties 
        [toolbarItem setToolTip: @"Plugin Preferences"];
        [toolbarItem setImage: [NSImage imageNamed: NSImageNameEveryone]];
        
        // Tell the item what message to send when it is clicked 
        [toolbarItem setTarget: self];
        [toolbarItem setAction: @selector(switchView:)];
    }
    else
    {
        toolbarItem = nil;
        /*
        // itemIdent refered to a toolbar item that is not provide or supported by us or cocoa 
        // Returning nil will inform the toolbar this kind of item is not supported 
        toolbarItem = [customIcons objectForKey:itemIdent];
         */
    }
    return toolbarItem;
}

- (NSRect)frameForNewContentViewFrame:(NSRect)viewFrame
{
    NSWindow *window = [self window];
    NSRect newFrameRect = [window frameRectForContentRect:viewFrame];
    NSRect oldFrameRect = [window frame];
    NSSize newSize = newFrameRect.size;
    NSSize oldSize = oldFrameRect.size;
    
    NSRect frame = [window frame];
    frame.size = newSize;
    frame.origin.y -= (newSize.height - oldSize.height);
    NSLog(@"frameForNewContentViewFrame: %@", NSStringFromRect(frame));
    return frame;
}

- (void)switchView:(id)sender
{
    // Figure out the new view, the old view, and the new size of the window
	NSView *previousView;
	NSView *view;
	if(sender == self)
	{
		previousView = nil;
		view = controlsView;
		currentViewIdentifier = OEControlsToolbarItemIdentifier;
	}
	else
	{
		previousView = [self viewForIdentifier: currentViewIdentifier];
		currentViewIdentifier = [sender itemIdentifier];
		view = [self viewForIdentifier:currentViewIdentifier];
	}
	
	NSRect newFrame = [self frameForNewContentViewFrame:[view frame]];
    NSLog(@"view frame: %@", NSStringFromRect([view frame]));
	
	// Using an animation grouping because we may be changing the duration
	[NSAnimationContext beginGrouping];
	
	// With the shift key down, do slow-mo animation
	if ([[NSApp currentEvent] modifierFlags] & NSShiftKeyMask)
	    [[NSAnimationContext currentContext] setDuration:5.0];
	
	// Call the animator instead of the view / window directly
	if(previousView) [[[[self window] animator] contentView] replaceSubview:previousView with:view];
	else             [[[[self window] animator] contentView] addSubview:view];
	[[[self window] animator] setFrame:newFrame display:YES];
	
	[NSAnimationContext endGrouping];
    
    if(currentViewIdentifier == OEControlsToolbarItemIdentifier)
        [pluginDrawer open];
    else [pluginDrawer close];
}

- (NSView *)viewForIdentifier:(NSString*)identifier {
   	if(identifier  == OEControlsToolbarItemIdentifier)
		return controlsView;
	else if(identifier == OEVideoSoundToolbarItemIdentifier)
		return videoView;
	//else if([identifier isEqualToString:AudioToolbarItemIdentifier])
	//	return audioView;
	else if(identifier == OEPluginsToolbarItemIdentifier)
		return pluginsView;
	//else
	//	return [customViews objectForKey:identifier];
    
    return nil;
}

@end
