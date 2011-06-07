//
//  OEDBRom.h
//  OpenEmuMockup
//
//  Created by Christoph Leimbrock on 10.04.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface OEDBRom : NSObject <NSPasteboardWriting>{
	NSString* title;		// name
	
	NSString* romPath;		// path to rom file
	NSString* coverPath;	// path to artwork
	
	NSUInteger rating;		// star rating
	NSString* consoleName;	// Name of console
	NSDate* romLastPlayed;  // Last play date
    
	NSUInteger fileStatus;	// Status of rom (eg file not found)
	
	NSImage* coverArt;
}
- (id)copyWithZone:(NSZone *)zone;

@property (readwrite, copy) NSString* title;
@property (readwrite, copy) NSString* romPath;
@property (assign) NSUInteger rating;
@property (readwrite, copy) NSString* consoleName;
@property (readwrite, copy) NSString* coverPath;
@property (readwrite, copy) NSDate* romLastPlayed;
@property (assign) NSUInteger fileStatus;


@property (readwrite, retain) NSImage* coverArt;
@end
