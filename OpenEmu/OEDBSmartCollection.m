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

#import "OEDBSmartCollection.h"
#import "OETheme.h"

NS_ASSUME_NONNULL_BEGIN

@implementation OEDBSmartCollection

+ (NSString *)entityName
{
    return @"SmartCollection";
}

#pragma mark - Sidebar Item Protocol

- (NSImage *)sidebarIcon
{
    return [[OETheme sharedTheme] imageForKey:@"collections_smart" forState:OEThemeStateDefault];
}

- (BOOL)isEditableInSidebar
{
    return YES;
}

- (void)setSidebarName:(nullable NSString *)newName
{}

#pragma mark - Game Collection View Item
- (nullable NSString *)collectionViewName
{
    NSString *name = [self valueForKey:@"name"];
    if([name isEqualToString:@"Recently Added"])
    {
        return NSLocalizedString(@"Recently Added", @"Recently Added Smart Collection Name");
    }

    return name;
}

- (BOOL)isCollectionEditable
{
    return NO;
}

- (void)setFetchPredicate:(NSPredicate *)fetchPredicate
{
    self.predicateData = [NSKeyedArchiver archivedDataWithRootObject:fetchPredicate];
    // TODO: invalidate cached predicated
}

- (NSPredicate *)fetchPredicate
{
    // TODO: cache predicate
    NSPredicate *predicate = [NSKeyedUnarchiver unarchiveObjectWithData:self.predicateData];
    return predicate ?: [NSPredicate predicateWithValue:NO];
}

- (BOOL)shouldShowSystemColumnInListView
{
    return YES;
}

- (nullable NSNumber*)fetchLimit
{
    return @30;
}

- (NSArray <NSSortDescriptor *> *)fetchSortDescriptors
{
    if(self.fetchSortKey)
    {
        return @[[NSSortDescriptor sortDescriptorWithKey:self.fetchSortKey ascending:self.fetchSortAscending]];
    }

    return @[];
}

@end

NS_ASSUME_NONNULL_END
