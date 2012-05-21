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
@synthesize regionSelector, dbActionSelector, contentView;
#pragma mark -
- (void)awakeFromNib
{    
    if([[NSUserDefaults standardUserDefaults] valueForKey:UDRegionKey])
    {
        OERegion currentRegion = [[OELocalizationHelper sharedHelper] region];
        [[self regionSelector] selectItemWithTag:currentRegion];
    }
        
    NSScrollView *scrollView = (NSScrollView*)[self view];    
    [scrollView setDocumentView:[self contentView]];
    [[self contentView] setFrameOrigin:(NSPoint){0,-[[self contentView] frame].size.height+[scrollView frame].size.height}];
}

- (NSString *)nibName
{
    return @"OEPrefDebugController";
}
#pragma mark -
- (IBAction)changeRegion:(id)sender
{
    if([sender selectedTag] == -1)
        [[NSUserDefaults standardUserDefaults] removeObjectForKey:UDRegionKey];
    else 
        [[NSUserDefaults standardUserDefaults] setInteger:[sender selectedTag] forKey:UDRegionKey];
    
    [[NSNotificationCenter defaultCenter] postNotificationName:OEDBSystemsChangedNotificationName object:self];
}


- (IBAction)executeDatbaseAction:(id)sender
{
    NSError *error = nil;
    NSArray *allGames = [OEDBGame allGamesWithError:&error];
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
                if([[obj archiveID] integerValue]!=0)
                {
                    NSSet* roms = [obj roms];
                    [roms enumerateObjectsUsingBlock:^(id obj, BOOL *stop) {
                      printf("%s\n", [[obj path] cStringUsingEncoding:NSUTF8StringEncoding]);
                    }];
                    
                }
            }];
            printf("\nDone\n");
            break;
        case 1:
            printf("\nLogging all games without archive ID\n\n");
            [allGames enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
                if([[obj archiveID] integerValue]==0)
                {
                    NSSet* roms = [obj roms];
                    [roms enumerateObjectsUsingBlock:^(id obj, BOOL *stop) {
                        printf("%s\n", [[obj path] cStringUsingEncoding:NSUTF8StringEncoding]);
                    }];
                    
                }
            }];
            printf("\nDone\n");
            break;
        case 2:
            printf("\nRemoving All Metadata\n\n");
            [allGames enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
                [obj setArchiveID:nil];
                [obj setGameDescription:nil];
                [obj setLastArchiveSync:nil];
                [obj setRating:[NSNumber numberWithInt:0]];
                [obj setBoxImage:nil];
                [obj setCredits:nil];
                [obj setGenres:nil];
            }];
            printf("\nDone\n");
            break;
            
        case 3:
            printf("\nRunning archive sync on all games\n\n");
            [allGames enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
                [(OEDBGame*)obj setNeedsFullSyncWithArchiveVG];
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
    return NSMakeSize(320, 400);
}

@end
