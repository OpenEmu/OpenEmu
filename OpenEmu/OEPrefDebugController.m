//
//  OEPrefDebugController.m
//  OpenEmu
//
//  Created by Carl Leimbrock on 22.02.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import "OEPrefDebugController.h"
#import "OELocalizationHelper.h"
@implementation OEPrefDebugController
@synthesize regionSelector;
#pragma mark -
- (void)awakeFromNib
{    
    if([[NSUserDefaults standardUserDefaults] valueForKey:UDRegionKey])
    {
        OERegion currentRegion = [[OELocalizationHelper sharedHelper] region];
        [[self regionSelector] selectItemWithTag:currentRegion];
    }
}

- (NSString *)nibName
{
    return @"OEPrefDebugController";
}
#pragma mark -
- (IBAction)changeRegion:(id)sender
{
    if([sender selectedTag] == -1)
    {
        [[NSUserDefaults standardUserDefaults] removeObjectForKey:UDRegionKey];
    }
    else 
    {
        [[NSUserDefaults standardUserDefaults] setInteger:[sender selectedTag] forKey:UDRegionKey];
    }
    
    [[NSNotificationCenter defaultCenter] postNotificationName:OEDBSystemsChangedNotificationName object:self];
}
#pragma mark -
#pragma mark OEPreferencePane Protocol
- (NSImage *)icon
{
    return [NSImage imageNamed:@"debug_tab_icon"];
}

- (NSString *)title
{
    return @"Debug";
}

- (NSString*)localizedTitle
{
    return NSLocalizedString([self title], "");
}

- (NSSize)viewSize
{
    return NSMakeSize(320, 445);
}

@end
