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
#import "OEPrefLibraryController.h"
#import "OEApplicationDelegate.h"
#import "OELibraryDatabase.h"
#import "OEDBSystem.h"
#import "OESystemPlugin.h"
#import "OECorePlugin.h"

#import "OEButton.h"
#import "OEHUDAlert.h"

@interface OEPrefLibraryController ()
{
    CGFloat height;
}

- (void)OE_rebuildAvailableLibraries;
- (void)OE_calculateHeight;
@end

@implementation OEPrefLibraryController

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    if((self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil]))
    {
        [self OE_calculateHeight];
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(_rebuildAvailableLibraries) name:OEDBSystemsChangedNotificationName object:nil];
    }
    
    return self;
}

- (void)awakeFromNib
{
    height = 473 - 110;
    [self OE_rebuildAvailableLibraries];
    
	NSString *path = [[NSUserDefaults standardUserDefaults] objectForKey:OEDatabasePathKey];
	[[self pathField] setStringValue:[path stringByAbbreviatingWithTildeInPath]];
}

#pragma mark ViewController Overrides

- (NSString *)nibName
{
	return @"OEPrefLibraryController";
}

#pragma mark OEPreferencePane Protocol

- (NSImage *)icon
{
	return [NSImage imageNamed:@"library_tab_icon"];
}

- (NSString *)title
{
	return @"Library";
}

- (NSString *)localizedTitle
{
    return NSLocalizedString([self title], "");
}

- (NSSize)viewSize
{
	return NSMakeSize(423, height);
}

#pragma mark -
#pragma mark UI Actions

- (IBAction)resetLibraryFolder:(id)sender
{
    NSString *databasePath = [[NSUserDefaults standardUserDefaults] valueForKey:OEDefaultDatabasePathKey];
    
    [[NSUserDefaults standardUserDefaults] setValue:databasePath forKey:OEDatabasePathKey];
    [[self pathField] setStringValue:[databasePath stringByAbbreviatingWithTildeInPath]];
}

- (IBAction)changeLibraryFolder:(id)sender
{
    NSOpenPanel *openDlg = [NSOpenPanel openPanel];
    
    openDlg.canChooseFiles = NO;
    openDlg.canChooseDirectories = YES;
    openDlg.canCreateDirectories = YES;
    
    [openDlg beginSheetModalForWindow:self.view.window completionHandler:^(NSInteger result)
    {
        if(NSFileHandlingPanelOKButton == result)
        {
            NSString *databasePath = [[openDlg URL] path];
            
            if(databasePath != nil && ![databasePath isEqualToString:[[NSUserDefaults standardUserDefaults] valueForKey:OEDatabasePathKey]])
            {
                [[NSUserDefaults standardUserDefaults] setValue:databasePath forKey:OEDatabasePathKey];
                FIXME("ewwwwwwwwww that's ugly, don't get the app delegate like that, EVER.");
                [(OEApplicationDelegate *) [NSApplication sharedApplication].delegate loadDatabase];
                [[self pathField] setStringValue:[databasePath stringByAbbreviatingWithTildeInPath]];
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
    if(disabled && [[OEDBSystem enabledSystems] count] == 1)
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
    
    [system setEnabled:[NSNumber numberWithBool:!disabled]];
    [[NSNotificationCenter defaultCenter] postNotificationName:OEDBSystemsChangedNotificationName object:system userInfo:nil];
}

#pragma mark -

- (void)OE_calculateHeight
{
    [self OE_rebuildAvailableLibraries];
}

- (void)OE_rebuildAvailableLibraries
{
    [[[[self librariesView] subviews] copy] makeObjectsPerformSelector:@selector(removeFromSuperviewWithoutNeedingDisplay)];
    
    // get all system plugins, ordered them by name
    NSArray *systems = [OEDBSystem allSystems];
    
    // calculate number of rows (using 2 columns)
    NSInteger rows = ceil([systems count] / 2.0);
    
    // set some spaces and dimensions
    CGFloat hSpace = 16, vSpace = 10;
    CGFloat iWidth = 163, iHeight = 18;
    
    // calculate complete view height
    height = 374 + (iHeight * rows + (rows - 1) * vSpace);
    
    if([self librariesView] == nil) return;
    
    [[self librariesView] setFrameSize:(NSSize){ [[self librariesView] frame].size.width, (iHeight * rows + (rows - 1) * vSpace)}];
    
    __block CGFloat x = 0;
    __block CGFloat y = [[self librariesView] frame].size.height - iHeight;
    
    // enumerate plugins and add buttons for them
    [systems enumerateObjectsUsingBlock:
     ^(OEDBSystem *system, NSUInteger idx, BOOL *stop)
     {
         // if we're still in the first column an we should be in the second
         if(x == 0 && idx > [systems count] / 2)
         {
             // we reset x and y
             x += iWidth+hSpace;
             y = [[self librariesView] frame].size.height-iHeight;
         }
         
         // creating the button
         NSRect rect = (NSRect){ { x, y }, { iWidth, iHeight } };
         NSString *systemIdentifier = [system systemIdentifier];
         OEButton *button = [[OEButton alloc] initWithFrame:rect];
         [button setThemeKey:@"dark_checkbox"];
         [button setButtonType:NSSwitchButton];
         [button setTarget:self];
         [button setAction:@selector(toggleLibrary:)];
         [button setTitle:[system name]];
         [button setState:[[system enabled] intValue]];
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
         if([system plugin] == nil)
         {
             [warnings addObject:NSLocalizedString(@"The System plugin could not be found!", @"")];
             
             // disabling ui element here so no system without a plugin can be enabled
             [button setEnabled:NO];
         }
         
         if(!foundCore)
             [warnings addObject:NSLocalizedString(@"This System has no corresponding core installed.", @"")];
         
         if([warnings count] != 0)
         {
             // Show a warning badge next to the checkbox
             // this is currently misusing the beta_icon image
             
             NSPoint badgePosition = [button badgePosition];
             NSImageView *imageView = [[NSImageView alloc] initWithFrame:(NSRect){ badgePosition, { 16, 17 } }];
             [imageView setImage:[NSImage imageNamed:@"beta_icon"]];
             
             // TODO: Use a custom tooltip that fits our style better
             [imageView setToolTip:[warnings componentsJoinedByString:@"\n"]];
             [[self librariesView] addSubview:imageView];
         }
         
         [[self librariesView] addSubview:button];
         
         // decreasing y
         y -= iHeight + vSpace;
     }];
}

@end
