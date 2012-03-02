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

#import "OEDBDataSourceAdditions.h"
#import "OEDBImage.h"

@implementation OEDBGame (DataSourceAdditions)

#pragma mark -
#pragma mark CoverGridDataSourceItem

- (NSString *)gridTitle
{
    return [self valueForKey:@"name"];
}

- (void)setGridTitle:(NSString *)str
{
    [self setValue:str forKey:@"name"];
}

- (int)gridStatus
{
    return [[self valueForKey:@"status"] intValue];
}

- (void)setGridRating:(NSUInteger)newRating
{
    [self setValue:[NSNumber numberWithUnsignedInteger:newRating] forKey:@"rating"];
}

- (NSUInteger)gridRating
{
    return [[self valueForKey:@"rating"] unsignedIntegerValue];
}

- (NSImage *)gridImage
{
    return [[self valueForKey:@"boxImage"] image];
}

- (NSImage *)gridImageWithSize:(NSSize)aSize
{
    return [[self valueForKey:@"boxImage"] imageForSize:aSize];
}

- (void)setGridImage:(NSImage *)gridImage
{
    [self setBoxImageByImage:gridImage];
}

#pragma mark -
#pragma mark CoverFlowDataSourceItem

- (NSString *)imageUID
{
    // Create a new UUID
    CFUUIDRef  uuidObj    = CFUUIDCreate(nil);
    
    // Get the string representation of the UUID
    NSString  *uuidString = (__bridge_transfer NSString *)CFUUIDCreateString(nil, uuidObj);
    CFRelease(uuidObj);
    
    return uuidString;
}

- (NSString *) imageRepresentationType
{
    return IKImageBrowserNSImageRepresentationType;
}

- (id)imageRepresentation
{
    return [self gridImage];
    
    NSManagedObject *boxImage = [self valueForKey:@"boxImage"];
    if(boxImage == nil) return nil;
    
    NSData *data = [boxImage valueForKey:@"data"];
    
    if(data != nil) return [[NSImage alloc] initWithData:data];
    
    //if([boxImage valueForKey:@"url"]) return nil;
    
    return nil;
}

- (NSString *)imageTitle
{
    return [self valueForKey:@"name"];
}

- (NSString *)imageSubtitle
{
    return nil;
}

- (NSUInteger)gameRating
{
    return [[self valueForKey:@"rating"] unsignedIntegerValue];
}

- (void)setImage:(NSImage *)img
{
}

#pragma mark -
#pragma mark ListView DataSource Item

- (NSImage *)listViewStatus:(BOOL)selected
{
    return nil;
}

- (void)setListViewRating:(NSNumber *)number
{
    [self setValue:number forKey:@"rating"];
}

- (NSNumber *)listViewRating
{
    return [self valueForKey:@"rating"];
}

- (NSString *)listViewTitle
{
    return [self valueForKey:@"name"];
}

- (NSString *)listViewLastPlayed
{
    // TODO: properly format date
    return [[self lastPlayed] description] ? : @"";
}

- (NSString *)listViewConsoleName
{
    return NSLocalizedString([[self valueForKey:@"system"] valueForKey:@"name"], @"");
}

- (void)setGridViewRating:(NSNumber *)number
{
    [self setValue:number forKey:@"rating"];
}

@end
#pragma mark -

@implementation OEDBSystem (DataSourceAdditions)

- (NSImage *)sidebarIcon
{
    return [self icon];
}

- (NSString *)sidebarName
{
    return [self name];
}

- (void)setSidebarName:(NSString *)newName
{
}

- (BOOL)isSelectableInSdebar
{
    return YES;
}

- (BOOL)isEditableInSdebar
{
    return NO;
}

- (BOOL)isGroupHeaderInSdebar
{
    return NO;
}

- (BOOL)hasSubCollections
{
    return NO;
}

@end

#pragma mark -

@implementation OEDBCollection (DataSourceAdditions)
- (NSImage *)sidebarIcon
{
    return [NSImage imageNamed:@"collections_simple"];
}

- (NSString *)sidebarName
{
    return [self valueForKey:@"name"];
}

- (void)setSidebarName:(NSString *)newName
{
    [self setValue:newName forKey:@"name"];
}

- (BOOL)isSelectableInSdebar
{
    return YES;
}

- (BOOL)isEditableInSdebar
{
    return YES;
}

- (BOOL)isGroupHeaderInSdebar
{
    return NO;
}

- (BOOL)hasSubCollections
{
    return NO;
}

@end

#pragma mark -
@implementation OEDBSmartCollection (DataSourceAdditions)

- (NSImage *)sidebarIcon
{
    return [NSImage imageNamed:@"collections_smart"];
}

@end

#pragma mark -
@implementation OEDBCollectionFolder (DataSourceAdditions)

- (NSImage *)sidebarIcon
{
    return [NSImage imageNamed:@"collections_folder"];
}

- (BOOL)hasSubCollections
{
    return YES;
}

@end


#pragma mark -
#pragma mark Implementation of items that can be presented by CollectionView

@implementation OEDBSystem (OECollectionViewItemAdditions)

- (NSString *)collectionViewName
{
    return [self name];
}

- (BOOL)isCollectionEditable
{
    return YES;
}

- (BOOL)removingGamesDeletesThem
{
    return YES;
}

- (NSPredicate *)predicate
{
    return [NSPredicate predicateWithFormat:@"system == %@", self];
}

@end

@implementation OEDBCollection (OECollectionViewItemAdditions)

- (NSString *)collectionViewName
{
    return [self valueForKey:@"name"];
}

- (BOOL)isCollectionEditable
{
    return YES;
}

- (BOOL)removingGamesDeletesThem
{
    return YES;
}

- (NSArray *)items
{
    return nil;
}

- (NSPredicate *)predicate
{
    return [NSPredicate predicateWithFormat:@"ANY collections == %@", self];
}

@end

// TODO: check how itunes treats folders
@implementation OEDBCollectionFolder (OECollectionViewItemAdditions)

- (NSString *)collectionViewName
{
    return [self valueForKey:@"name"];
}

- (BOOL)isCollectionEditable
{
    return YES;
}

- (BOOL)removingGamesDeletesThem
{
    return NO;
}

- (NSPredicate *)predicate
{
    return [NSPredicate predicateWithValue:NO];
}

@end

@implementation OEDBSmartCollection (OECollectionViewItemAdditions)

- (NSString *)collectionViewName
{
    return [self valueForKey:@"name"];
}

- (BOOL)isCollectionEditable
{
    return NO;
}

- (BOOL)removingGamesDeletesThem
{
    @throw [NSException exceptionWithName:NSInternalInconsistencyException reason:@"-[OEDBSmartCollection removingGamesDeletesThem] not yet implemented." userInfo:nil];
    
    return NO;
}

- (NSPredicate *)predicate
{
    return [NSPredicate predicateWithValue:NO];
}

@end

@implementation OEDBAllGamesCollection (OECollectionViewItemAdditions)

- (NSString *)collectionViewName
{
    return [self sidebarName];
}

- (BOOL)isCollectionEditable
{
    return YES;
}

- (BOOL)removingGamesDeletesThem
{
    return YES;
}

- (NSArray *)items
{
    return nil;
}

- (NSPredicate *)predicate
{
    return [NSPredicate predicateWithValue:YES];
}

@end
