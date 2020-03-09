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

#import "OEDBCollection.h"
#import "OETheme.h"

NS_ASSUME_NONNULL_BEGIN

@implementation OEDBCollection

+ (NSString*)entityName
{
    return @"Collection";
}

#pragma mark - Data Model Properties

@dynamic name;

#pragma mark - Data Model Relationships

@dynamic games;

- (nullable NSMutableSet <OEDBGame *> *)mutableGames;
{
    return [self mutableSetValueForKeyPath:@"games"];
}

#pragma mark - Sidebar Item Protocol

- (nullable NSString *)viewControllerClassName
{
    return @"OEGameCollectionViewController";
}

- (nullable NSString *)sidebarID
{
    return self.permanentIDURI.absoluteString;
}

- (nullable NSImage *)sidebarIcon
{
    return [[OETheme sharedTheme] imageForKey:@"collections_simple" forState:OEThemeStateDefault];
}

- (NSString *)sidebarName
{
    return [self valueForKey:@"name"];
}

- (void)setSidebarName:(NSString *)newName
{
    [self setValue:newName forKey:@"name"];
}

- (BOOL)isSelectableInSidebar
{
    return YES;
}

- (BOOL)isEditableInSidebar
{
    return YES;
}

- (BOOL)isGroupHeaderInSidebar
{
    return NO;
}

- (BOOL)hasSubCollections
{
    return NO;
}

#pragma mark - OEGameCollectionView item

- (nullable NSString *)collectionViewName
{
    return [self valueForKey:@"name"];
}

- (BOOL)isCollectionEditable
{
    return YES;
}

- (NSArray * _Nullable)items
{
    return nil;
}

- (NSPredicate *)fetchPredicate
{
    return [NSPredicate predicateWithFormat:@"ANY collections == %@", self];
}

- (NSInteger)fetchLimit
{
    return 0;
}

- (NSArray <NSSortDescriptor *> *)fetchSortDescriptors
{
    return @[];
}

- (BOOL)shouldShowSystemColumnInListView
{
    return YES;
}

@end

NS_ASSUME_NONNULL_END
