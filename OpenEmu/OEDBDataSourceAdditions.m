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
#import "OEDBRom.h"
#import "OEGameDocument.h"
#import "OEGameViewController.h"
#import "NSArray+OEAdditions.h"

static NSDateFormatter *_OEListViewDateFormatter;
static void OE_initOEListViewDateFormatter(void) __attribute__((constructor));
static NSString * OE_stringFromElapsedTime(NSTimeInterval);

@implementation OEDBGame (DataSourceAdditions)

#pragma mark -
#pragma mark CoverGridDataSourceItem

- (NSString *)gridTitle
{
    return [self displayName];
}

- (void)setGridTitle:(NSString *)str
{
    [self setDisplayName:str];
}

- (int)gridStatus
{
    return [[self status] intValue];
}

- (void)setGridRating:(NSUInteger)newRating
{
    [self setRating:[NSNumber numberWithUnsignedInteger:newRating]];
}

- (NSUInteger)gridRating
{
    return [[self rating] unsignedIntegerValue];
}

- (NSImage *)gridImage
{
    return [[self boxImage] originalImage];
}

- (NSImage *)gridImageWithSize:(NSSize)aSize
{
    return [[self boxImage] imageForSize:aSize];
}

- (BOOL)hasImage
{
    return [self boxImage] != nil;
}

- (NSSize)actualGridImageSizeforSize:(NSSize)aSize
{
    return [[self boxImage] sizeOfThumbnailForSize:aSize];
}

- (void)setGridImage:(NSImage *)gridImage
{
    [self setBoxImageByImage:gridImage];
}

- (OECoverGridViewCellIndicationType)gridIndicationType
{
    return ([[self status] intValue] == OEDBGameStatusAlert ? OECoverGridViewCellIndicationTypeFileMissing  :
           ([[self status] intValue] == OEDBGameStatusProcessing ? OECoverGridViewCellIndicationTypeProcessing :
            OECoverGridViewCellIndicationTypeNone));
}
#pragma mark -
#pragma mark CoverFlowDataSourceItem
- (NSString *)imageUID
{
    return [[[self boxImage] imagePathForSize:NSMakeSize(20, 20)] absoluteString] ?: @"";

    // Create a new UUID
    CFUUIDRef  uuidObj    = CFUUIDCreate(nil);
    
    // Get the string representation of the UUID
    NSString  *uuidString = (__bridge_transfer NSString *)CFUUIDCreateString(nil, uuidObj);
    CFRelease(uuidObj);
    
    return uuidString;
}

- (NSString *)imageRepresentationType
{
    return IKImageBrowserNSImageRepresentationType;
}

- (id)imageRepresentation
{
    return [[self boxImage] imageForSize:NSMakeSize(300, 300)];
    return [self gridImage];
    
    NSManagedObject *boxImage = [self boxImage];
    if(boxImage == nil) return nil;
    
    NSData *data = [boxImage valueForKey:@"data"];
    
    if(data != nil) return [[NSImage alloc] initWithData:data];
    
    //if([boxImage valueForKey:@"sourceURL"]) return nil;
    
    return nil;
}

- (NSString *)imageTitle
{
    return [self displayName];
}

- (NSString *)imageSubtitle
{
    return nil;
}

- (NSUInteger)gameRating
{
    return [[self rating] unsignedIntegerValue];
}

- (void)setImage:(NSImage *)img
{
}

#pragma mark -
#pragma mark ListView DataSource Item

- (NSImage *)listViewStatus
{
    NSString *imageName = [self OE_listViewStatusImageName];
    return (imageName ? [NSImage imageNamed:imageName] : nil);
}

- (NSImage *)listViewSelectedStatus
{
    NSString *imageName = [self OE_listViewStatusImageName];
    return (imageName ? [NSImage imageNamed:[imageName stringByAppendingString:@"_selected"]] : nil);
}

- (NSString *)OE_listViewStatusImageName
{
    return ([self OE_hasOpenDocument] ? @"list_indicators_playing"  :
            ([[self status] intValue] == OEDBGameStatusAlert) ? @"list_indicators_missing"  :
            ![self lastPlayed]        ? @"list_indicators_unplayed" : nil);
}

- (BOOL)OE_hasOpenDocument
{
    id doc = [[[NSDocumentController sharedDocumentController] documents] firstObjectMatchingBlock:
              ^ BOOL (OEGameDocument *doc)
              {
                  return [doc isKindOfClass:[OEGameDocument class]] && [[[[[doc gameViewController] document] rom] game] isEqual:self];
              }];
    
    return doc != nil;
}

- (void)setListViewRating:(NSNumber *)number
{
    [self setRating:number];
}

- (NSNumber *)listViewRating
{
    return [self rating];
}

- (void)setListViewTitle:(NSString*)title
{
    [self setDisplayName:title];
}

- (NSString *)listViewTitle
{
    return [self displayName];
}

- (NSString *)listViewLastPlayed
{
    return ([self lastPlayed] ?
            [_OEListViewDateFormatter stringFromDate:[self lastPlayed]] :
            @"");
}

- (NSString *)listViewConsoleName
{
    return NSLocalizedString([[self system] valueForKey:@"name"], @"");
}

- (void)setGridViewRating:(NSNumber *)number
{
    [self setRating:number];
}

- (NSNumber *)listViewSaveStateCount
{
    return ([[self saveStateCount] unsignedIntegerValue] > 0 ? [self saveStateCount] : nil);
}

- (NSNumber *)listViewPlayCount
{
    return ([[self playCount] unsignedIntegerValue] > 0 ? [self playCount] : nil);
}

- (NSString *)listViewPlayTime
{
    return ([[self playTime] doubleValue] > 0 ? OE_stringFromElapsedTime([[self playTime] doubleValue]) : @"");
}

static void OE_initOEListViewDateFormatter(void)
{
    @autoreleasepool
    {
        _OEListViewDateFormatter = [NSDateFormatter new];
        [_OEListViewDateFormatter setDateStyle:NSDateFormatterMediumStyle];
        [_OEListViewDateFormatter setTimeStyle:NSDateFormatterNoStyle];
    }
}

NSString * OE_stringFromElapsedTime(NSTimeInterval timeInterval)
{
    const int oneMinuteInSeconds = 60;
    const int oneHourInSeconds   = 60 * oneMinuteInSeconds;
    const int oneDayInSeconds    = 24 * oneHourInSeconds;

    NSString *dayUnit    = @"d";
    NSString *hourUnit   = @"h";
    NSString *minuteUnit = @"m";
    NSString *secondUnit = @"s";

    NSMutableString *s = [NSMutableString new];
    NSUInteger       t = ABS(timeInterval);

    if(t > oneDayInSeconds)
    {
        [s appendFormat:@"%lu%@", (unsigned long)(t / oneDayInSeconds), dayUnit];
        t %= oneDayInSeconds;
    }

    if(t > oneHourInSeconds)
    {
        NSString *format = ([s length] > 0 ? @"%02u%@" : @"%u%@");
        [s appendFormat:format, (unsigned)(t / oneHourInSeconds), hourUnit];
        t %= oneHourInSeconds;
    }

    if(t > oneMinuteInSeconds)
    {
        NSString *format = ([s length] > 0 ? @"%02u%@" : @"%u%@");
        [s appendFormat:format, (unsigned)(t / oneMinuteInSeconds), minuteUnit];
        t %= oneMinuteInSeconds;
    }

    NSString *format = ([s length] > 0 ? @"%02u%@" : @"%u%@");
    [s appendFormat:format, (unsigned)t, secondUnit];

    return s;
}

@end
#pragma mark -
@implementation OEDBSystem (DataSourceAdditions)

- (NSString*)viewControllerClassName
{
    return @"OECollectionViewController";
}

- (NSString*)sidebarID
{
    return [self systemIdentifier];
}

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

@end

#pragma mark -

@implementation OEDBCollection (DataSourceAdditions)
- (NSString*)viewControllerClassName
{
    return @"OECollectionViewController";
}

- (NSString*)sidebarID
{
    return [[[self objectID] URIRepresentation] absoluteString];
}

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

- (NSPredicate *)predicate
{
    return [NSPredicate predicateWithFormat:@"system == %@", self];
}

- (BOOL)shouldShowSystemColumnInListView
{
    return NO;
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

- (NSArray *)items
{
    return nil;
}

- (NSPredicate *)predicate
{
    return [NSPredicate predicateWithFormat:@"ANY collections == %@", self];
}

- (BOOL)shouldShowSystemColumnInListView
{
    return YES;
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

- (NSPredicate *)predicate
{
    return [NSPredicate predicateWithValue:NO];
}

- (BOOL)shouldShowSystemColumnInListView
{
    return NO;
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

- (NSPredicate *)predicate
{
    return [NSPredicate predicateWithValue:NO];
}

- (BOOL)shouldShowSystemColumnInListView
{
    return YES;
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

- (NSArray *)items
{
    return nil;
}

- (NSPredicate *)predicate
{
    return [NSPredicate predicateWithFormat:@"system.enabled = YES"];
}

- (BOOL)shouldShowSystemColumnInListView
{
    return YES;
}

@end
