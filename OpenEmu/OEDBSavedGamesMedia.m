/*
 Copyright (c) 2015, OpenEmu Team
 
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

#import "OEDBSavedGamesMedia.h"
#import "OETheme.h"
#import "OEDBSaveState.h"
#import "OEDBSaveCheat.h"
NS_ASSUME_NONNULL_BEGIN

NSString * const OEDBSavedGamesMediaShowsAutoSaves = @"OEDBSavedGamesMediaShowsAutoSaves";
NSString * const OEDBSavedGamesMediaShowsQuickSaves = @"OEDBSavedGamesMediaShowsQuickSaves";

@implementation OEDBSavedGamesMedia

+ (instancetype)sharedDBSavedGamesMedia
{
    static OEDBSavedGamesMedia *sharedInstance;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        sharedInstance = [OEDBSavedGamesMedia new];
    });
    return sharedInstance;
}

#pragma mark - OESidebarItem

- (nullable NSImage *)sidebarIcon
{
    return [[OETheme sharedTheme] imageForKey:@"media_saved_games" forState:OEThemeStateDefault];
}

- (NSString *)sidebarName
{
    return NSLocalizedString(@"Saved Games", @"");
}

- (void)setSidebarName:(NSString *)newName
{
    NSLog(@"OEDBSavedGamesMedia: can not change name!");
}

- (nullable NSString*)viewControllerClassName
{
    return @"OEMediaViewController";
}

- (nullable NSString*)sidebarID
{
    return @"savedGames";
}

- (BOOL)isSelectableInSidebar
{
    return YES;
}

- (BOOL)isEditableInSidebar
{
    return NO;
}

- (BOOL)isGroupHeaderInSidebar
{
    return NO;
}

- (BOOL)hasSubCollections
{
    return NO;
}

#pragma mark - OECollectionViewItemProtocol

- (BOOL)isCollectionEditable
{
    return NO;
}

- (NSPredicate *)baseFilterPredicate
{
    NSMutableArray <NSPredicate *> *subpredicates = [NSMutableArray array];
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];

    if(![defaults boolForKey:OEDBSavedGamesMediaShowsAutoSaves]){
        [subpredicates addObject:[NSPredicate predicateWithFormat:@"name != %@", OESaveStateAutosaveName]];
    }

    if(![defaults boolForKey:OEDBSavedGamesMediaShowsQuickSaves]){
       [subpredicates addObject:[NSPredicate predicateWithFormat:@"NOT(name BEGINSWITH %@)", OESaveStateQuicksaveName]];
    }

    return [NSCompoundPredicate andPredicateWithSubpredicates:subpredicates];
}
@end

NS_ASSUME_NONNULL_END
