//
//  OEPrefDebugController.m
//  OpenEmu
//
//  Created by Carl Leimbrock on 22.02.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import "OEPrefDebugController.h"
#import "OELocalizationHelper.h"

#import "OEDBGame.h"
@implementation OEPrefDebugController
@synthesize regionSelector, dbActionSelector;
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


- (IBAction)executeDatbaseAction:(id)sender
{
    NSError* error = nil;
    NSArray* allGames = [OEDBGame allGamesWithError:&error];
    if(!allGames)
    {
        NSLog(@"Error getting all games");
        NSLog(@"%@", [error localizedDescription]);
        return;
    }
    
    switch ([[self dbActionSelector] selectedTag]) {
        case 0:
            printf("\nLogging all games with archive ID\n\n");
            [allGames enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
                if([[obj valueForKey:@"archiveID"] integerValue]!=0)
                {
                    NSSet* roms = [obj valueForKey:@"roms"];
                    [roms enumerateObjectsUsingBlock:^(id obj, BOOL *stop) {
                      printf("%s\n", [[obj valueForKey:@"path"] cStringUsingEncoding:NSUTF8StringEncoding]);
                    }];
                    
                }
            }];
            printf("\nDone\n");
            break;
        case 1:
            printf("\nLogging all games without archive ID\n\n");
            [allGames enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
                if([[obj valueForKey:@"archiveID"] integerValue]==0)
                {
                    NSSet* roms = [obj valueForKey:@"roms"];
                    [roms enumerateObjectsUsingBlock:^(id obj, BOOL *stop) {
                        printf("%s\n", [[obj valueForKey:@"path"] cStringUsingEncoding:NSUTF8StringEncoding]);
                    }];
                    
                }
            }];
            printf("\nDone\n");
            break;
        case 2:
            printf("\nRemoving All Metadata\n\n");
            [allGames enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
                [obj setValue:nil forKey:@"archiveID"];
                [obj setValue:nil forKey:@"gameDescription"];
                [obj setValue:nil forKey:@"lastArchiveSync"];
                [obj setValue:[NSNumber numberWithInt:0] forKey:@"rating"];
                [obj setValue:nil forKey:@"boxImage"];
                [obj setValue:nil forKey:@"credits"];
                [obj setValue:nil forKey:@"genres"];
            }];
            printf("\nDone\n");
            break;
            
        case 3:
            printf("\nRunning archive sync on all games\n\n");
            [allGames enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
                NSError* syncError = nil;
                if(![(OEDBGame*)obj performSyncWithArchiveVG:&syncError])
                {
                    NSLog(@"Error with archive sync:");
                    NSLog(@"%@", [error localizedDescription]);
                }
            }];
            printf("\nDone\n");
            break;
    }
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
    return NSMakeSize(320, 533);
}

@end
