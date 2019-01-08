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

#import "OEDBDataSourceAdditions.h"
#import "OEDBImage.h"
#import "OEDBRom.h"
#import "OEGameDocument.h"
#import "OEGameViewController.h"
#import "OETheme.h"
#import "NSArray+OEAdditions.h"
@import Quartz;

#import "OpenEmu-Swift.h"

NS_ASSUME_NONNULL_BEGIN

static NSDateFormatter *_OEListViewDateFormatter;
static void OE_initOEListViewDateFormatter(void) __attribute__((constructor));
static NSString * OE_localizedStringFromElapsedTime(NSTimeInterval);

NSString * const OECoverGridViewAutoDownloadEnabledKey = @"OECoverGridViewAutoDownloadEnabledKey";

@implementation OEDBGame (DataSourceAdditions)

#pragma mark - CoverGridDataSourceItem

- (void)setGridTitle:(NSString *)str
{
    self.displayName = str;
    [self.managedObjectContext save:nil];
}

- (void)setGridRating:(NSUInteger)newRating
{
    self.rating = @(newRating);
    [self.managedObjectContext save:nil];
}

- (NSUInteger)gridRating
{
    return self.rating.unsignedIntegerValue;
}

- (BOOL)hasImage
{
    return self.boxImage != nil;
}

- (BOOL)shouldIndicateDeletable
{
    return self.defaultROM.source != nil;
}

- (BOOL)shouldIndicateDownloadable
{
    OEDBRom *rom = self.defaultROM;
    return !([self.status isEqualTo:@(OEDBGameStatusDownloading)] || rom.source == nil || [rom.URL checkResourceIsReachableAndReturnError:nil]);
}

- (NSString *)imageUID
{
    OEDBImage *image = self.boxImage;
    NSString *result = nil;
    if(image.isLocalImageAvailable && image.relativePath != nil)
        result = image.UUID;
    else if(image != nil && image.source != nil && [[NSUserDefaults standardUserDefaults] boolForKey:OECoverGridViewAutoDownloadEnabledKey])
        result = image.source;
    else
    {
        CGFloat aspectRatio = self.system.coverAspectRatio;
        result = [NSString stringWithFormat:@":MissingArtwork(%f)", aspectRatio];
    }
    return result;
}

- (NSString *)imageRepresentationType
{
    NSString *result = nil;
    OEDBImage *image = self.boxImage;
    if(image.isLocalImageAvailable && image.relativePath != nil)
        result = IKImageBrowserNSURLRepresentationType;
    else if(image != nil && image.source != nil && [[NSUserDefaults standardUserDefaults] boolForKey:OECoverGridViewAutoDownloadEnabledKey])
        result = IKImageBrowserNSURLRepresentationType;
    else
        result = IKImageBrowserNSImageRepresentationType;
    
    return result;
}

- (id)imageRepresentation
{
    OEDBImage *image = self.boxImage;

    if(image.isLocalImageAvailable && image.relativePath != nil)
    {
        return image.imageURL;
    }

    if(image != nil && image.source != nil && [[NSUserDefaults standardUserDefaults] boolForKey:OECoverGridViewAutoDownloadEnabledKey])
    {
        return image.sourceURL;
    }

    CGFloat aspectRatio = self.system.coverAspectRatio;
    return [[self class] artworkPlacholderWithAspectRatio:aspectRatio];
}

+ (NSImage *)artworkPlacholderWithAspectRatio:(CGFloat)ratio
{
    static NSCache *cache = nil;
    if(cache == nil)
    {
        cache = [[NSCache alloc] init];
        cache.countLimit = 20;
    }
    
    NSString *key = [NSString stringWithFormat:@"%f", ratio];
    NSImage *image = [cache objectForKey:key];
    if(image == nil)
    {
        image = [[NSImage alloc] init];
        image.size = NSMakeSize(300, 300 * ratio);
        [cache setObject:image forKey:key];
        image.cacheMode = NSImageCacheBySize;

    }
    return image;
}

- (NSString *)imageTitle
{
    return self.displayName;
}

- (NSString * _Nullable)imageSubtitle
{
    return nil;
}

- (NSInteger)gridStatus
{
    return self.status.integerValue;
}

- (NSUInteger)gameRating
{
    return self.rating.unsignedIntegerValue;
}

- (void)setImage:(NSImage *)img
{
}

#pragma mark - ListView DataSource Item

- (OEThemeImage *)listViewStatus
{
    NSString *imageKey = [self OE_listViewStatusImageKey];
    return [[OETheme sharedTheme] themeImageForKey:imageKey];
}

- (NSString *)OE_listViewStatusImageKey
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
                  return [doc isKindOfClass:[OEGameDocument class]] && [doc.gameViewController.document.rom.game isEqual:self];
              }];
    
    return doc != nil;
}

- (void)setListViewRating:(NSNumber *)number
{
    self.rating = number;
}

- (NSNumber *)listViewRating
{
    return self.rating;
}

- (void)setListViewTitle:(NSString*)title
{
    self.displayName = title;
}

- (NSString *)listViewTitle
{
    return self.displayName;
}

- (NSString *)listViewLastPlayed
{
    return (self.lastPlayed ?
            [_OEListViewDateFormatter stringFromDate:self.lastPlayed] :
            @"");
}

- (NSString *)listViewConsoleName
{
    return NSLocalizedString([self.system valueForKey:@"name"], @"");
}

- (void)setGridViewRating:(NSNumber *)number
{
    self.rating = number;
}

- (NSNumber *)listViewSaveStateCount
{
    return (self.saveStateCount.unsignedIntegerValue > 0 ? self.saveStateCount : nil);
}

- (NSNumber *)listViewPlayCount
{
    return (self.playCount.unsignedIntegerValue > 0 ? self.playCount : nil);
}

- (NSString *)listViewPlayTime
{
    return (self.playTime.doubleValue > 0 ? OE_localizedStringFromElapsedTime(self.playTime.doubleValue) : @"");
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

NSString * OE_localizedStringFromElapsedTime(NSTimeInterval timeInterval)
{
    static NSDateComponentsFormatter *dcf;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        dcf = [[NSDateComponentsFormatter alloc] init];
        [dcf setUnitsStyle:NSDateComponentsFormatterUnitsStyleAbbreviated];
        [dcf setAllowedUnits:NSCalendarUnitSecond + NSCalendarUnitMinute + NSCalendarUnitHour + NSCalendarUnitDay];
    });
    
    return [dcf stringFromTimeInterval:timeInterval];
}

@end

#pragma mark -

@implementation OEDBSystem (DataSourceAdditions)

- (NSString*)viewControllerClassName
{
    return @"OEGameCollectionViewController";
}

- (nullable NSString *)sidebarID
{
    return self.systemIdentifier;
}

- (NSImage *)sidebarIcon
{
    return self.icon;
}

- (NSString *)sidebarName
{
    return self.name;
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

#pragma mark - Implementation of items that can be presented by OEGameCollectionView

@implementation OEDBSystem (OECollectionViewItemAdditions)

- (NSString *)collectionViewName
{
    return self.name;
}

- (BOOL)isCollectionEditable
{
    return YES;
}

- (NSPredicate *)fetchPredicate
{
    return [NSPredicate predicateWithFormat:@"system == %@", self];
}

- (BOOL)shouldShowSystemColumnInListView
{
    return NO;
}

- (NSInteger)fetchLimit
{
    return 0;
}
- (NSArray*)fetchSortDescriptors
{
    return @[];
}
@end

@implementation OEDBAllGamesCollection (OECollectionViewItemAdditions)

- (NSString *)collectionViewName
{
    return self.sidebarName;
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
    return [NSPredicate predicateWithFormat:@"system.enabled = YES"];
}

- (BOOL)shouldShowSystemColumnInListView
{
    return YES;
}

- (NSInteger)fetchLimit
{
    return 0;
}

- (NSArray*)fetchSortDescriptors
{
    return @[];
}
@end

NS_ASSUME_NONNULL_END
