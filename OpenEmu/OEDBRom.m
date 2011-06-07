//
//  OEDBRom.m
//  OpenEmuMockup
//
//  Created by Christoph Leimbrock on 10.04.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "OEDBRom.h"


@implementation OEDBRom

- (id)copyWithZone:(NSZone *)zone
{
    OEDBRom *copy = [[self class] allocWithZone:zone];
        
	copy.title = self.title;
	copy.romPath = self.romPath;
	copy.rating = self.rating;
	copy.consoleName = self.consoleName;
	copy.coverPath = self.coverPath;
	copy.romLastPlayed = self.romLastPlayed;
	copy.fileStatus = self.fileStatus;
	
	copy.coverArt = self.coverArt;
	
    return copy;
}
#pragma mark -
#pragma mark NSPasteboardWriting
- (NSArray *)writableTypesForPasteboard:(NSPasteboard *)pasteboard{
	return [NSArray arrayWithObject:@"org.openEmu.rom"];
}
 
- (NSPasteboardWritingOptions)writingOptionsForType:(NSString *)type pasteboard:(NSPasteboard *)pasteboard{
	return 0;
}

- (id)pasteboardPropertyListForType:(NSString *)type{
	
	// To do: return appropriate obj
	
	return self.title;
}
#pragma mark -
- (id)valueForKey:(NSString *)key{
	
	if([key isEqualTo:@"romStatus"]){
		return [NSNumber numberWithInteger:self.fileStatus];
	}
	
	if([key isEqualToString:@"romName"] | [key isEqualToString:@"title"]){
		return self.title;
	}
	
	if([key isEqualToString:@"romRating"]){
		return [NSNumber numberWithInteger:self.rating];
	}
	
	if([key isEqualToString:@"romLastPlayed"]){
		return self.romLastPlayed;
	}
	
	if([key isEqualToString:@"consoleName"]){
		return self.consoleName;
	}
	
	return nil;
}

/*
- (id)initWithPasteboardPropertyList:(id)propertyList ofType:(NSString *)type{}
+ (NSArray *)readableTypesForPasteboard:(NSPasteboard *)pasteboard{}
+ (NSPasteboardReadingOptions)readingOptionsForType:(NSString *)type pasteboard:(NSPasteboard *)pasteboard{
	return 0;
}
*/

- (void)dealloc {
    self.coverArt = nil;
	
	self.title = nil;
	self.romPath = nil;
	self.consoleName = nil;
	self.coverPath = nil;
	self.romLastPlayed = nil;
	
	
    [super dealloc];
}

@synthesize title;
@synthesize romPath;
@synthesize rating;
@synthesize consoleName;
@synthesize coverPath;
@synthesize romLastPlayed;
@synthesize fileStatus;

@synthesize coverArt;
@end
