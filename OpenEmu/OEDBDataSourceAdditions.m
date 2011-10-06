//
//  OEDBRom  (DataSourceAdditions).m
//  OpenEmuMockup
//
//  Created by Christoph Leimbrock on 10.04.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "OEDBDataSourceAdditions.h"
#import "OEDBImage.h"
@implementation OEDBGame (DataSourceAdditions)

#pragma mark -
#pragma mark CoverGridDataSourceItem
- (NSString*)gridTitle{
    return [self valueForKey:@"name"];
}
- (void)setGridTitle:(NSString*)str{
    [self setValue:str forKey:@"name"];
}

- (int)gridStatus{
	return 0;
}

- (void)setGridRating:(NSUInteger)newRating{
    [self setValue:[NSNumber numberWithUnsignedInteger:newRating] forKey:@"rating"];
}

- (NSUInteger)gridRating{
    return [[self valueForKey:@"rating"] unsignedIntegerValue];
}

- (NSImage*)gridImage{
    OEDBImage* boxImage = [self valueForKey:@"boxImage"];
    if(boxImage==nil) return nil;
    
	return [boxImage image];
}

- (NSImage*)gridImageWithSize:(NSSize)aSize{	
    OEDBImage* boxImage = [self valueForKey:@"boxImage"];
    if(boxImage==nil) return nil;
    	
	return [boxImage imageForSize:aSize];
}


- (void)setGridImage:(NSImage *)gridImage{
	[self setBoxImageByImage:gridImage];
}

#pragma mark -
#pragma mark CoverFlowDataSourceItem
- (NSString *)imageUID{
    /*if(self.coverPath)
		return self.coverPath;
	*/
    
	// Create a new UUID
	CFUUIDRef  uuidObj = CFUUIDCreate(nil);
	
	// Get the string representation of the UUID
	NSString  *uuidString = (NSString *)CFUUIDCreateString(nil, uuidObj);
	CFRelease(uuidObj);
	
	return [uuidString autorelease];
}

- (NSString *) imageRepresentationType{
    return IKImageBrowserNSImageRepresentationType;
}

- (id)imageRepresentation{  
	return [self gridImage];
	
    NSManagedObject* boxImage = [self valueForKey:@"boxImage"];
    if(boxImage==nil) return nil;
    
    
    if([boxImage valueForKey:@"data"]){
	  NSImage* cover = [[NSImage alloc] initWithData:[boxImage valueForKey:@"data"]];
	  return [cover autorelease];	  
    }
    
    if([boxImage valueForKey:@"url"]) return nil;
    
    return nil;
}

- (NSString *)imageTitle{
    return [self valueForKey:@"name"];
}

- (NSString *)imageSubtitle{
    return nil;
}

- (NSUInteger)gameRating{
    return [[self valueForKey:@"rating"] unsignedIntegerValue];
}

- (void)setImage:(NSImage*)img{

}
#pragma mark -
#pragma mark ListView DataSource Item
- (NSImage*)listViewStatus:(BOOL)selected{
	/*if(self.fileStatus==1 && !selected){
		NSImage* res = [NSImage imageNamed:@"list_indicators_missing"];
		return res;
	} 
	
	if(self.fileStatus==1 && selected){
		NSImage* res = [NSImage imageNamed:@"list_indicators_missing_selected"];
		return res;
	} 
	
	NSDate* refDate = [NSDate dateWithTimeIntervalSince1970:0];
	if([refDate isEqualToDate:self.romLastPlayed] &&  !selected){
		return [NSImage imageNamed:@"list_indicators_unplayed"];
	}
	
	if([refDate isEqualToDate:self.romLastPlayed] && selected){
		return [NSImage imageNamed:@"list_indicators_unplayed_selected"];
	}*/
	
	return nil;
}

- (void)setListViewRating:(NSNumber*)number{
    [self setValue:number forKey:@"rating"];
}

- (NSNumber*)listViewRating{
    return [self valueForKey:@"rating"];
}
- (NSString*)listViewTitle{
	NSString* title = [self valueForKey:@"name"];
    return title;
}
- (NSString*)listViewLastPlayed{
    return @"";
//	return [NSString stringWithFormat:@"%@", self.romLastPlayed];
}

- (NSString*)listViewConsoleName{
    return NSLocalizedString([[self valueForKey:@"system"] valueForKey:@"name"], @"");
}

- (void)setGridViewRating:(NSNumber*)number{
    [self setValue:number forKey:@"rating"];
}
@end
#pragma mark -

@implementation OEDBSystem (DataSourceAdditions)
- (NSImage*)sidebarIcon{
	return self.icon;
}
- (NSString*)sidebarName{
    return NSLocalizedString([self valueForKey:@"name"], @"");
}

- (void)setSidebarName:(NSString*)newName{
}

- (BOOL)isSelectableInSdebar{
	return YES;
}
- (BOOL)isEditableInSdebar{
	return NO;
}
- (BOOL)isGroupHeaderInSdebar{
	return NO;
}

- (BOOL)hasSubCollections{
    return NO;
}

- (NSPredicate*)predicate{
	return [NSPredicate predicateWithFormat:@"system == %@", self];
}

@end
#pragma mark -
@implementation OEDBCollection (DataSourceAdditions)
- (NSImage*)sidebarIcon{
	return [NSImage imageNamed:@"collections_simple"];
}
- (NSString*)sidebarName{
    return [self valueForKey:@"name"];
}

- (void)setSidebarName:(NSString*)newName{
    [self setValue:newName forKey:@"name"];
}

- (BOOL)isSelectableInSdebar{
	return YES;
}
- (BOOL)isEditableInSdebar{
	return YES;
}
- (BOOL)isGroupHeaderInSdebar{
	return NO;
}

- (BOOL)hasSubCollections{
    return NO;
}

@end
#pragma mark -
@implementation OEDBSmartCollection (DataSourceAdditions)
- (NSImage*)sidebarIcon{
	return [NSImage imageNamed:@"collections_smart"];
}
@end

#pragma mark -
@implementation OEDBCollectionFolder (DataSourceAdditions)
- (NSImage*)sidebarIcon{
    return [NSImage imageNamed:@"collections_folder"];
}
- (BOOL)hasSubCollections{
    return YES;
}
@end


#pragma mark -
#pragma mark Implementation of items that can be presented by CollectionView
@implementation OEDBSystem (OECollectionViewItemAdditions)
- (NSString*)collectionViewName{
    return [self valueForKey:@"name"];
}
- (BOOL)isCollectionEditable{
    return YES;
}
- (BOOL)removingGamesDeletesThem{
    return YES;
}

- (NSPredicate*)predicate{
	return [NSPredicate predicateWithFormat:@"system == %@", self];
}

@end

@implementation OEDBCollection (OECollectionViewItemAdditions)
- (NSString*)collectionViewName{
    return [self valueForKey:@"name"];
}
- (BOOL)isCollectionEditable{
    return YES;
}

- (BOOL)removingGamesDeletesThem{
    return YES;
}

- (NSArray*)items{
    return nil;
}

- (NSPredicate*)predicate{
	return [NSPredicate predicateWithFormat:@"ANY collections == %@", self];
}
@end

// TODO: check how itunes treats folders
@implementation OEDBCollectionFolder (OECollectionViewItemAdditions)
- (NSString*)collectionViewName{
    return [self valueForKey:@"name"];
}

- (BOOL)isCollectionEditable{
    return YES;
}

- (BOOL)removingGamesDeletesThem{
    return NO;
}

- (NSPredicate*)predicate{
	return [NSPredicate predicateWithValue:NO];
}
@end
@implementation OEDBSmartCollection (OECollectionViewItemAdditions)
- (NSString*)collectionViewName{
    return [self valueForKey:@"name"];
}
- (BOOL)isCollectionEditable{
    return NO;
}
- (BOOL)removingGamesDeletesThem{
    @throw @"bUb";
    return NO;
}
- (NSPredicate*)predicate{
	return [NSPredicate predicateWithValue:NO];
}
@end
@implementation OEDBAllGamesCollection (OECollectionViewItemAdditions)
- (NSString*)collectionViewName{
    return [self valueForKey:@"name"];
}

- (BOOL)isCollectionEditable{
    return YES;
}

- (BOOL)removingGamesDeletesThem{
    return YES;
}

- (NSArray*)items{
    return nil;
}

- (NSPredicate*)predicate{
	return [NSPredicate predicateWithValue:YES];
}
@end
