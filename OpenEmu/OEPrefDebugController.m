/*
 Copyright (c) 2012, OpenEmu Team
 
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


#import "OEPrefDebugController.h"
#import "OELocalizationHelper.h"
#import "OELibraryDatabase.h"
#import "OESidebarController.h"

#import "OEDBGame.h"

@implementation OEPrefDebugController
@synthesize regionSelector, dbActionSelector, contentView;

#pragma mark -

- (void)awakeFromNib
{    
    if([[NSUserDefaults standardUserDefaults] valueForKey:OERegionKey])
    {
        OERegion currentRegion = [[OELocalizationHelper sharedHelper] region];
        [[self regionSelector] selectItemWithTag:currentRegion];
    }
        
    NSScrollView *scrollView = (NSScrollView*)[self view];    
    [scrollView setDocumentView:[self contentView]];
    [[self contentView] setFrameOrigin:(NSPoint){ 0 , -[[self contentView] frame].size.height + [scrollView frame].size.height}];
}

- (NSString *)nibName
{
    return @"OEPrefDebugController";
}

#pragma mark -

- (IBAction)changeRegion:(id)sender
{
    if([sender selectedTag] == -1)
        [[NSUserDefaults standardUserDefaults] removeObjectForKey:OERegionKey];
    else 
        [[NSUserDefaults standardUserDefaults] setInteger:[sender selectedTag] forKey:OERegionKey];
    
    [[NSNotificationCenter defaultCenter] postNotificationName:OEDBSystemsChangedNotificationName object:self];
}

- (IBAction)executeDatbaseAction:(id)sender
{
    NSError *error = nil;
    NSArray *allGames = [OEDBGame allGamesInDatabase:[OELibraryDatabase defaultDatabase] error:&error];
    
    if(allGames == nil)
    {
        NSLog(@"Error getting all games");
        NSLog(@"%@", [error localizedDescription]);
        return;
    }
    
    switch([[self dbActionSelector] selectedTag])
    {
        case 0 :
            printf("\nLogging all games with archive ID\n\n");
            [allGames enumerateObjectsUsingBlock:
             ^(OEDBGame *obj, NSUInteger idx, BOOL *stop)
             {
                 if([[obj archiveID] integerValue] != 0)
                     printf("%s\n", [[[[[obj roms] valueForKey:@"path"] allObjects] componentsJoinedByString:@"\n"] UTF8String]);
             }];
            printf("\nDone\n");
            break;
        case 1 :
            printf("\nLogging all games without archive ID\n\n");
            [allGames enumerateObjectsUsingBlock:
             ^(OEDBGame *obj, NSUInteger idx, BOOL *stop)
             {
                if([[obj archiveID] integerValue] == 0)
                    printf("%s\n", [[[[[obj roms] valueForKey:@"path"] allObjects] componentsJoinedByString:@"\n"] UTF8String]);
             }];
            printf("\nDone\n");
            break;
        case 2 :
            printf("\nRemoving All Metadata\n\n");
            [allGames enumerateObjectsUsingBlock:
             ^(id obj, NSUInteger idx, BOOL *stop)
             {
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
            [allGames enumerateObjectsUsingBlock:
             ^(id obj, NSUInteger idx, BOOL *stop)
             {
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

- (NSString *)localizedTitle
{
    return NSLocalizedString([self title], "");
}

- (NSSize)viewSize
{
    return NSMakeSize(320, 400);
}

@end
