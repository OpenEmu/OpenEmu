//
//  OEDBRom  (DataSourceAdditions).m
//  OpenEmuMockup
//
//  Created by Christoph Leimbrock on 10.04.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "OEDBDataSourceAdditions.h"

@implementation OEDBRom (DataSourceAdditions)

#pragma mark -
#pragma mark CoverGridDataSourceItem
- (NSString*)gridTitle{
    return self.title;
}
- (void)setGridTitle:(NSString*)str{
	self.title = str;
}

- (int)gridStatus{
	return self.fileStatus;
}

- (void)setGridRating:(NSUInteger)newRating{
	self.rating = newRating;
}

- (NSUInteger)gridRating{
	return self.rating;
}

- (NSImage*)gridImage{
	if(self.coverArt==nil){
		return [[[NSImage alloc] initWithContentsOfFile:self.coverPath] autorelease];
	}
	return self.coverArt;
}

- (void)setGridImage:(NSImage *)gridImage{
	self.coverArt = gridImage;
}

#pragma mark -
#pragma mark CoverFlowDataSourceItem
- (NSString *) imageUID{
	if(self.coverPath)
		return self.coverPath;
	
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
	if(self.coverArt==nil)
		self.coverArt = [[[NSImage alloc] initWithContentsOfFile:self.coverPath] autorelease];
	
	return self.coverArt;
}

- (NSString *)imageTitle{
    return self.title;
}

- (NSString *) imageSubtitle{
    return nil;
}

- (NSUInteger)gameRating{
	return self.rating;
}

- (void)setImage:(NSImage*)img{
	self.coverArt = img;
}
#pragma mark -
#pragma mark ListView DataSource Item
- (NSImage*)listViewStatus:(BOOL)selected{
	if(self.fileStatus==1 && !selected){
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
	}
	
	return nil;
}

- (void)setListViewRating:(NSNumber*)number{
	self.rating = [number intValue];
}

- (NSNumber*)listViewRating{
	return [NSNumber numberWithInteger:self.rating];
}
- (NSString*)listViewTitle{
	return self.title;
}
- (NSString*)listViewLastPlayed{
	return [NSString stringWithFormat:@"%@", self.romLastPlayed];
}

- (NSString*)listViewConsoleName{
	return self.consoleName;
}

- (void)setGridViewRating:(NSNumber*)number{
	self.rating = [number intValue];
}
@end
#pragma mark -
@implementation OEDBConsole (DataSourceAdditions)
- (NSImage*)sidebarIcon{
	return self.icon;
}
- (NSString*)sidebarName{
	return self.name;
}

- (void)setSidebarName:(NSString*)newName{
	self.name = newName;
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
@end
#pragma mark -
@implementation OEDBCollection (DataSourceAdditions)
- (NSImage*)sidebarIcon{
	return [NSImage imageNamed:@"collections_simple"];
}
- (NSString*)sidebarName{
	return self.name;
}

- (void)setSidebarName:(NSString*)newName{
	self.name = newName;
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
@end
#pragma mark -
@implementation OEDBSmartCollection (DataSourceAdditions)
- (NSImage*)sidebarIcon{
	return [NSImage imageNamed:@"collections_smart"];
}
- (NSString*)sidebarName{
	return self.name;
}

- (void)setSidebarName:(NSString*)newName{
	self.name = newName;
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
@end