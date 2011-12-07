//
//  OEPrefLibraryController.m
//  OpenEmu
//
//  Created by Christoph Leimbrock on 07.06.11.
//  Copyright 2011 none. All rights reserved.
//

#import "OEPrefLibraryController.h"
#import "OECheckBox.h"

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

- (void)dealloc{
    [super dealloc];
}

- (void)awakeFromNib
{
    height = 480-106;
    [self _rebuildAvailableLibraries];
    
	NSString* path = [[NSUserDefaults standardUserDefaults] objectForKey:UDDatabasePathKey];
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

- (NSSize)viewSize
{
	// TODO: decide how to implement Available Libraries
	return NSMakeSize(423, height);
}
#pragma mark -
#pragma mark UI Actions
- (IBAction)resetLibraryFolder:(id)sender
{
	
}

- (IBAction)changeLibraryFolder:(id)sender
{
}

- (IBAction)toggleLibrary:(id)sender
{
    NSString* systemIdentifier = [[sender cell] representedObject];

    OEDBSystem* system = [OEDBSystem systemForPluginIdentifier:systemIdentifier inDatabase:[OELibraryDatabase defaultDatabase]];
    
    BOOL disabled = ![sender state];
    if(disabled && [[[OELibraryDatabase defaultDatabase] enabledSystems] count]==1)
    {
        NSString* message = NSLocalizedString(@"At least one System must be enabled", @"");
        NSString* button = NSLocalizedString(@"OK", @"");
        OEHUDAlert* alert = [OEHUDAlert alertWithMessageText:message defaultButton:button alternateButton:nil];
        [alert runModal];
        
        [sender setState:NSOnState];
        
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
    NSArray* systems = [[OELibraryDatabase defaultDatabase] systems];
                        
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
    [systems enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) 
     {
         OEDBSystem* system = (OEDBSystem*)obj;
         // if we're still in the first column an we should be in the second
         if(x==0 && idx>[systems count]/2){
             // we reset x and y
             x += iWidth+hSpace;
             y = librariesView.frame.size.height-iHeight;
         }
         
         // creating the button
         NSRect rect = (NSRect){{x, y}, {iWidth, iHeight}};
         OECheckBox* button = [[OECheckBox alloc] initWithFrame:rect];
         
         NSString* systemIdentifier = [system valueForKey:@"systemIdentifier"];
         [button setTarget:self];
         [button setAction:@selector(toggleLibrary:)];
         [button setTitle:[system name]];
         [button setState:[[system valueForKey:@"enabled"] intValue]];
         [[button cell] setRepresentedObject:systemIdentifier];
         [librariesView addSubview:button];
         [button release];
         
        
         BOOL foundCore = NO;
         NSArray* allPlugins = [OECorePlugin allPlugins];
         for(OECorePlugin* obj in allPlugins)
         {
             
             if([[obj systemIdentifiers] containsObject:systemIdentifier])
             {
                 foundCore = YES;
                 break;
             }
         }
         
         if(!foundCore){

         }
                  
         // decreasing y
         y -= iHeight+vSpace;
     }];
}

@end
