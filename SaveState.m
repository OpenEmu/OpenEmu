// 
//  SaveState.m
//  OpenEmu
//
//  Created by Joshua Weinberg on 7/30/09.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import "SaveState.h"


@implementation SaveState 

@dynamic timeStamp;
@dynamic emulatorID;
@dynamic screenShot;
@dynamic saveData;
@dynamic pathalias;

@dynamic rompath;

- (NSString*) rompath
{
	NSData* aliasData = [self valueForKey:@"pathalias"];
	AliasHandle handle;
	
	//Look mah! IM LEGACY
	OSErr er = PtrToHand([aliasData bytes], (Handle *)&handle, [aliasData length]);
	
	FSRef fileRef;
	Boolean wasChanged;
	FSResolveAlias(NULL, handle, &fileRef, &wasChanged);
	
	DisposeHandle((Handle)handle);
	
	char path[1024];
	
	FSRefMakePath(&fileRef, (UInt8 *)path, 1024);
	
	NSString *nsPath = [NSString stringWithCString:path length:strlen(path)];
	nsPath = [nsPath stringByStandardizingPath];
}

- (id) imageRepresentation
{
	NSImage* image = [[NSImage alloc] initWithData:[self.screenShot valueForKey:@"screenShot"]];
	return [image autorelease];
}

- (NSString *)imageRepresentationType {
    // We use this representation type because we are storing the image as binary data.
	return IKImageBrowserNSImageRepresentationType;
}

- (NSString *)imageUID {
    // This is uses the NSManagedObjectID for the entity to generate a unique string.
    return [[[self objectID] URIRepresentation] description];
}

- (NSString *) imageTitle
{
	return [self.timeStamp descriptionWithCalendarFormat:@"%m/%d/%Y %H:%M:%S" timeZone:nil locale:nil];
	//return [self.rompath lastPathComponent];
}

- (NSString *) imageSubtitle
{
	return [self.timeStamp descriptionWithCalendarFormat:@"%m/%d/%Y %H:%M:%S" timeZone:nil locale:nil];
}

@end
