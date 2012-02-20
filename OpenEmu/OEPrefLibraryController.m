//
//  OEPrefLibraryController.m
//  OpenEmu
//
//  Created by Christoph Leimbrock on 07.06.11.
//  Copyright 2011 none. All rights reserved.
//

#import "OEPrefLibraryController.h"
#import "OECheckBox.h"

#import "OEApplicationDelegate.h"
#import "OELibraryDatabase.h"
#import "OEDBSystem.h"
#import "OESystemPlugin.h"
#import "OECorePlugin.h"

#import "OEHUDAlert.h"
@interface OEPrefLibraryController ()
- (void)_rebuildAvailableLibraries;
- (void)_calculateHeight;
@end
@implementation OEPrefLibraryController

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
        [self _calculateHeight];
    }
    
    return self;
}


- (void)awakeFromNib
{
    height = 473-110;
    [self _rebuildAvailableLibraries];
    
	NSString *path = [[NSUserDefaults standardUserDefaults] objectForKey:UDDatabasePathKey];
	[pathField setStringValue:[path stringByAbbreviatingWithTildeInPath]];
}
#pragma mark ViewController Overrides
- (NSString*)nibName
{
	return @"OEPrefLibraryController";
}

#pragma mark OEPreferencePane Protocol
- (NSImage*)icon
{
	return [NSImage imageNamed:@"library_tab_icon"];
}

- (NSString*)title
{
	return @"Library";
}

- (NSString*)localizedTitle
{
    return NSLocalizedString([self title], "");
}

- (NSSize)viewSize
{
	// TODO: decide how to implement Available Libraries
	return NSMakeSize(423, height);
}
#pragma mark -
#pragma mark UI Actions
- (IBAction)resetLibraryFolder:(id)sender
{
    NSString *databasePath = [[NSUserDefaults standardUserDefaults] valueForKey:UDDefaultDatabasePathKey];
    
    [[NSUserDefaults standardUserDefaults] setValue:databasePath forKey:UDDatabasePathKey];
    [pathField setStringValue:[databasePath stringByAbbreviatingWithTildeInPath]];
}

- (IBAction)changeLibraryFolder:(id)sender
{
    NSOpenPanel *openDlg = [NSOpenPanel openPanel];
    
    openDlg.canChooseFiles = NO;
    openDlg.canChooseDirectories = YES;
    openDlg.canCreateDirectories = YES;
    
    [openDlg beginSheetModalForWindow:self.view.window completionHandler:^(NSInteger result)
    {
        if (NSFileHandlingPanelOKButton == result)
        {
            NSString *databasePath = [[openDlg URL] path];
            
            if (databasePath && ![databasePath isEqualToString:[[NSUserDefaults standardUserDefaults] valueForKey:UDDatabasePathKey]])
            {
                [[NSUserDefaults standardUserDefaults] setValue:databasePath forKey:UDDatabasePathKey];
                [(OEApplicationDelegate *) [NSApplication sharedApplication].delegate OE_loadDatabase];
                [pathField setStringValue:[databasePath stringByAbbreviatingWithTildeInPath]];
            }
        }
    }];
}

- (IBAction)toggleLibrary:(id)sender
{
    NSString *systemIdentifier = [[sender cell] representedObject];
    
    OEDBSystem *system = [OEDBSystem systemForPluginIdentifier:systemIdentifier inDatabase:[OELibraryDatabase defaultDatabase]];
    
    BOOL disabled = ![sender state];
    // Make sure that at least one system is enabled.
    // Otherwise the mainwindow sidebar would be messed up
    if(disabled && [[[OELibraryDatabase defaultDatabase] enabledSystems] count]==1)
    {
        NSString *message = NSLocalizedString(@"At least one System must be enabled", @"");
        NSString *button = NSLocalizedString(@"OK", @"");
        OEHUDAlert *alert = [OEHUDAlert alertWithMessageText:message defaultButton:button alternateButton:nil];
        [alert runModal];
        
        [sender setState:NSOnState];
        
        return;
    }
    
    // Make sure only systems with a valid plugin are enabled.
    // Is also ensured by disabling ui element (checkbox)
    if(![system plugin])
    {
        NSString *message = [NSString stringWithFormat:NSLocalizedString(@"%@ could not be enabled because it's plugin was not found.", @""), [system name]];
        NSString *button = NSLocalizedString(@"OK", @"");
        OEHUDAlert *alert = [OEHUDAlert alertWithMessageText:message defaultButton:button alternateButton:nil];
        [alert runModal];
        
        [sender setState:NSOffState];
        
        return;        
    }
    
    [system setValue:[NSNumber numberWithBool:!disabled] forKey:@"enabled"];
    [[NSNotificationCenter defaultCenter] postNotificationName:OEDBSystemsChangedNotificationName object:system userInfo:nil];
}

#pragma mark -
- (void)_calculateHeight{
    [self _rebuildAvailableLibraries];
}

- (void)_rebuildAvailableLibraries
{
    // remove all subviews (if any)
    while(librariesView && [[librariesView subviews] count])
    {
        [[[librariesView subviews] lastObject] removeFromSuperview];
    }
    
    // get all system plugins, ordered them by name
    NSArray *systems = [[OELibraryDatabase defaultDatabase] systems];
    
    // calculate number of rows (using 2 columns)
    int rows = ceil([systems count]/2.0);
    
    // set some spaces and dimensions
    float hSpace = 16, vSpace = 10;
    float iWidth=163, iHeight = 18;
    
    // calculate complete view height
    height = 374+(iHeight*rows+(rows-1)*vSpace);
    
    if(!librariesView)
        return;
    
    [librariesView setFrameSize:(NSSize){librariesView.frame.size.width, (iHeight*rows+(rows-1)*vSpace)}];
    
    __block float x = 0;
    __block float y =  librariesView.frame.size.height-iHeight;
    
    // enumerate plugins and add buttons for them
    [systems enumerateObjectsUsingBlock:
     ^(OEDBSystem *system, NSUInteger idx, BOOL *stop)
     {
         // if we're still in the first column an we should be in the second
         if(x==0 && idx>[systems count]/2){
             // we reset x and y
             x += iWidth+hSpace;
             y = librariesView.frame.size.height-iHeight;
         }
         
         // creating the button
         NSRect rect = (NSRect){{x, y}, {iWidth, iHeight}};
         OECheckBox *button = [[OECheckBox alloc] initWithFrame:rect];
         
         NSString *systemIdentifier = [system valueForKey:@"systemIdentifier"];
         [button setTarget:self];
         [button setAction:@selector(toggleLibrary:)];
         [button setTitle:[system name]];
         [button setState:[[system valueForKey:@"enabled"] intValue]];
         [[button cell] setRepresentedObject:systemIdentifier];
         
         
         // Check if a core is installed that is capable of running this system
         BOOL foundCore = NO;
         NSArray *allPlugins = [OECorePlugin allPlugins];
         for(OECorePlugin *obj in allPlugins)
         {
             
             if([[obj systemIdentifiers] containsObject:systemIdentifier])
             {
                 foundCore = YES;
                 break;
             }
         }
         
         // TODO: warnings should also give advice on how to solve them
         // e.g. Go to Cores preferences and download Core x
         // or we could add a "Fix This" button that automatically launches the "No core for system ... " - Dialog
         NSMutableArray *warnings = [NSMutableArray arrayWithCapacity:2];
         if(![system plugin])
         {
             [warnings addObject:NSLocalizedString(@"The System plugin could not be found!", @"")];
             
             // disabling ui element here so no system without a plugin can be enabled
             [button setEnabled:NO];
         }
         
         if(!foundCore)
         {
             [warnings addObject:NSLocalizedString(@"This System has no corresponding core installed.", @"")];
         }
         
         if([warnings count]!=0)
         {
             // Show a warning badge next to the checkbox
             // this is currently misusing the beta_icon image
             
             NSPoint badgePosition = [button badgePosition];
             NSImageView *imageView = [[NSImageView alloc] initWithFrame:(NSRect){badgePosition, { 16, 17} }];
             [imageView setImage:[NSImage imageNamed:@"beta_icon"]];
             
             // TODO: Use a custom tooltip that fits our style better
             [imageView setToolTip:[warnings componentsJoinedByString:@"\n"]];
             [librariesView addSubview:imageView];
         }
         
         [librariesView addSubview:button];
         
         // decreasing y
         y -= iHeight+vSpace;
     }];
}

@end
