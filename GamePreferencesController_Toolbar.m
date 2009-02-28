
#warning This file should be deleted.
#if 0
#import "GamePreferencesController.h"

static NSString *PreferenceToolbar     = @"Preference Toolbar";
static NSString *VideoToolbarItemIdentifier   = @"Video Item Identifier";
static NSString *ControlsToolbarItemIdentifier = @"Controls Item Identifier";
static NSString *AudioToolbarItemIdentifier 	= @"Audio Item Identifier";
static NSString *PluginToolbarItemIdentifier 	= @"Plugin Item Identifier";

@implementation GamePreferencesController(Toolbar)

// ============================================================
// NSToolbar Related Methods
// ============================================================
- (void)setupToolbarForWindow:(NSWindow *)theWindow {
    // Create a new toolbar instance, and attach it to our document window 
    NSToolbar *toolbar = [[[NSToolbar alloc] initWithIdentifier: PreferenceToolbar] autorelease];
	
    // Set up toolbar properties: Allow customization, give a default display mode, and remember state in user defaults 
    [toolbar setAllowsUserCustomization: YES];
    [toolbar setAutosavesConfiguration: YES];
    [toolbar setDisplayMode: NSToolbarDisplayModeIconAndLabel];
	
    // We are the delegate
    [toolbar setDelegate: self];
	
	NSMutableDictionary* mCustomViews = [[[NSMutableDictionary alloc] init] autorelease];
	NSMutableDictionary* mCustomIcons = [[[NSMutableDictionary alloc] init] autorelease];
	
	
	for(NSBundle * bundle in [docController bundles])
	{
		if([bundle pathForResource:@"Preferences" ofType:@"nib"])
		{
			NSNib * nib = [[NSNib alloc] initWithNibNamed:@"Preferences" bundle:bundle];
			NSArray * objects;
			if([nib instantiateNibWithOwner:self topLevelObjects:&objects])
			{
				for(NSObject*object in objects)
				{
					if([object isKindOfClass:[NSView class]])
					{
						
						[mCustomViews setObject:object forKey:[bundle bundleIdentifier]];
						
						NSToolbarItem *toolbarItem = [[[NSToolbarItem alloc] initWithItemIdentifier: [bundle bundleIdentifier]] autorelease];
						
						
						NSString * iconPath = [bundle pathForResource:[bundle objectForInfoDictionaryKey:@"CFIconName"] ofType:@"icns"];
						
						[toolbarItem setLabel:[bundle objectForInfoDictionaryKey:@"ToolbarLabel"]];
						[toolbarItem setPaletteLabel:[bundle objectForInfoDictionaryKey:@"ToolbarLabel"]];
						
						// Set up a reasonable tooltip, and image   Note, these aren't localized, but you will likely want to localize many of the item's properties 
						[toolbarItem setToolTip:@"I can't believe this worked"];
						[toolbarItem setImage:[[NSImage alloc] initWithContentsOfFile: iconPath]];
						
						// Tell the item what message to send when it is clicked 
						[toolbarItem setTarget:self];
						[toolbarItem setAction:@selector(switchView:)];
		
						
						[mCustomIcons setObject:toolbarItem forKey:[bundle bundleIdentifier]];
					}
				}
			}
		}
	}
	
	customViews = [[NSDictionary alloc] initWithDictionary:mCustomViews];
	
	customIcons = [[NSDictionary alloc] initWithDictionary:mCustomIcons];
	
	
	NSLog(@"Custom views: %@", mCustomViews);
	
    // Attach the toolbar to the document window 
    [theWindow setToolbar:toolbar];
}

- (NSToolbarItem *)toolbar:(NSToolbar *)toolbar itemForItemIdentifier:(NSString *)itemIdent willBeInsertedIntoToolbar:(BOOL)willBeInserted {
    // Required delegate method:  Given an item identifier, this method returns an item 
    // The toolbar will use this method to obtain toolbar items that can be displayed in the customization sheet, or in the toolbar itself 
    NSToolbarItem *toolbarItem = [[[NSToolbarItem alloc] initWithItemIdentifier: itemIdent] autorelease];
	
    if ([itemIdent isEqual:VideoToolbarItemIdentifier]) {
        // Set the text label to be displayed in the toolbar and customization palette 
        [toolbarItem setLabel:@"Video"];
        [toolbarItem setPaletteLabel:@"Video"];
        
        // Set up a reasonable tooltip, and image   Note, these aren't localized, but you will likely want to localize many of the item's properties 
        [toolbarItem setToolTip:@"Video Preferences"];
        [toolbarItem setImage:[NSImage imageNamed: NSImageNameComputer]];
		
        // Tell the item what message to send when it is clicked 
        [toolbarItem setTarget:self];
        [toolbarItem setAction:@selector(switchView:)];
		
    } else if ([itemIdent isEqual:ControlsToolbarItemIdentifier]) {
        // Set the text label to be displayed in the toolbar and customization palette 
        [toolbarItem setLabel: @"Controls"];
        [toolbarItem setPaletteLabel: @"Controls"];
        
        // Set up a reasonable tooltip, and image   Note, these aren't localized, but you will likely want to localize many of the item's properties 
        [toolbarItem setToolTip: @"Control Preferences"];
        [toolbarItem setImage: [NSImage imageNamed: NSImageNamePreferencesGeneral]];
        
        // Tell the item what message to send when it is clicked 
        [toolbarItem setTarget:self];
        [toolbarItem setAction:@selector(switchView:)];
        
    } else if ([itemIdent isEqual:AudioToolbarItemIdentifier]) {		
        // Set the text label to be displayed in the toolbar and customization palette 
        [toolbarItem setLabel: @"Audio"];
        [toolbarItem setPaletteLabel: @"Audio"];
        
        // Set up a reasonable tooltip, and image   Note, these aren't localized, but you will likely want to localize many of the item's properties 
        [toolbarItem setToolTip: @"Audio Preferences"];
        [toolbarItem setImage: [NSImage imageNamed: NSImageNameAdvanced]];
        
        // Tell the item what message to send when it is clicked 
        [toolbarItem setTarget: self];
        [toolbarItem setAction: @selector(switchView:)];
	} else if ([itemIdent isEqual:PluginToolbarItemIdentifier]) {		
        // Set the text label to be displayed in the toolbar and customization palette 
        [toolbarItem setLabel: @"Plugins"];
        [toolbarItem setPaletteLabel: @"Plugins"];
        
        // Set up a reasonable tooltip, and image   Note, these aren't localized, but you will likely want to localize many of the item's properties 
        [toolbarItem setToolTip: @"Plugin Preferences"];
        [toolbarItem setImage: [NSImage imageNamed: NSImageNameEveryone]];
        
        // Tell the item what message to send when it is clicked 
        [toolbarItem setTarget: self];
        [toolbarItem setAction: @selector(switchView:)];
    } else {
        // itemIdent refered to a toolbar item that is not provide or supported by us or cocoa 
        // Returning nil will inform the toolbar this kind of item is not supported 
        toolbarItem = [customIcons objectForKey:itemIdent];
    }
    return toolbarItem;
}

- (NSArray *)toolbarDefaultItemIdentifiers:(NSToolbar*)toolbar {
    /* Required method.  Returns the ordered list of items to be shown in the toolbar by default.   If during initialization, no overriding values are found in the user defaults, or if the user chooses to revert to the default items this set will be used. */
	NSArray* standardItems = [NSArray arrayWithObjects:
							  VideoToolbarItemIdentifier, ControlsToolbarItemIdentifier, AudioToolbarItemIdentifier,PluginToolbarItemIdentifier, nil];

	
	return [standardItems arrayByAddingObjectsFromArray:[customIcons allKeys]];
}

- (NSArray *)toolbarAllowedItemIdentifiers:(NSToolbar*)toolbar {
    /* Required method.  Returns the list of all allowed items by identifier.  By default, the toolbar does not assume any items are allowed, even the separator.  So, every allowed item must be explicitly listed.  The set of allowed items is used to construct the customization palette.  The order of items does not necessarily guarantee the order of appearance in the palette.  At minimum, you should return the default item list.*/
    NSArray* standardItems = [NSArray arrayWithObjects:
			VideoToolbarItemIdentifier, ControlsToolbarItemIdentifier, AudioToolbarItemIdentifier,PluginToolbarItemIdentifier,
			NSToolbarFlexibleSpaceItemIdentifier, NSToolbarSpaceItemIdentifier, 
            NSToolbarSeparatorItemIdentifier, NSToolbarCustomizeToolbarItemIdentifier,
			nil];
	return [standardItems arrayByAddingObjectsFromArray:[customIcons allKeys]];
}

- (BOOL)validateToolbarItem:(NSToolbarItem *)toolbarItem {
     return YES;
}

-(IBAction)switchView:(id)sender {
	
	// Figure out the new view, the old view, and the new size of the window
	NSView *previousView;
	NSView *view;
	NSString* identifier;
	if(sender == self)
	{
		previousView = nil;
		view = controlsView;
		currentViewIdentifier = ControlsToolbarItemIdentifier;
	}
	else
	{
		identifier = [sender itemIdentifier];
		view = [self viewForIdentifier:identifier];
		previousView = [self viewForIdentifier: currentViewIdentifier];
		currentViewIdentifier = identifier;	
	}
	
	
	
	
	
	NSRect newFrame = [self newFrameForNewContentView:view];
	
	// Using an animation grouping because we may be changing the duration
	[NSAnimationContext beginGrouping];
	
	// With the shift key down, do slow-mo animation
	if ([[NSApp currentEvent] modifierFlags] & NSShiftKeyMask)
	    [[NSAnimationContext currentContext] setDuration:5.0];
	
	// Call the animator instead of the view / window directly
	if(previousView)
		[[[[self window] animator] contentView] replaceSubview:previousView with:view];
	else
	{
		[[[[self window] animator] contentView] addSubview:view];
	}
	[[[self window] animator] setFrame:newFrame display:YES];
	
	[NSAnimationContext endGrouping];
}

-(NSView *)viewForIdentifier:(NSString*)identifier {
   	if([identifier isEqualToString:ControlsToolbarItemIdentifier])
		return controlsView;
	else if([identifier isEqualToString:VideoToolbarItemIdentifier])
		return videoView;
	else if([identifier isEqualToString:AudioToolbarItemIdentifier])
		return audioView;
	else if([identifier isEqualToString:PluginToolbarItemIdentifier])
		return pluginsView;
	else
		return [customViews objectForKey:identifier];
}

@end
#endif