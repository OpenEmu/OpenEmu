//
//  OESaveState.m
//  OpenEmu
//
//  Created by Steve Streza on 9/3/09.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import "OESaveState.h"
#import "NSString+Aliases.h"

@interface OESaveState (OEPrivate)

-(void)_OE_writeDataToPlist;
-(NSDictionary *)_OE_readDataFromPlist;
-(void)_OE_setupBundleContents;

@end


@implementation OESaveState

@dynamic romFile, emulatorID, timeStamp, pathAlias;

-(id)initInsertedIntoManagedObjectContext:(NSManagedObjectContext *)context{
	NSEntityDescription *description = [NSEntityDescription entityForName:@"SaveState" inManagedObjectContext:context];
	return ([self initWithEntity:description insertIntoManagedObjectContext:context]);
}

-(void)setBundlePath:(NSString *)path{
	[[NSFileManager defaultManager] createDirectoryAtPath:path
							  withIntermediateDirectories:YES
											   attributes:nil
													error:nil];
	[self setPathAlias:[path OE_pathAliasData]];
	
	[self _OE_setupBundleContents];
}

-(void)_OE_setupBundleContents{
	NSFileManager *manager = [NSFileManager defaultManager];
	[manager createDirectoryAtPath:[self resourcePath]
	   withIntermediateDirectories:YES
						attributes:nil
							 error:nil];
}

-(NSString *)bundlePath{
	return [NSString OE_stringWithPathOfAliasData:[self valueForKey:@"pathAlias"]];
}

-(NSString *)resourcePath{
	return [[self bundlePath] stringByAppendingPathComponent:@"Resources"];
}

-(NSString *)userDescription{
	if(!userDescription){
		userDescription = [[[self _OE_readDataFromPlist] objectForKey:@"userDescription"] copy];
	}
	return userDescription;
}

-(NSString *)emulatorID{
	if(!emulatorID){
		emulatorID = [[[self _OE_readDataFromPlist] objectForKey:@"emulatorID"] copy];
	}
	return emulatorID;
}

-(void)setEmulatorID:(NSString *)newEmulatorID{
	if([emulatorID isEqual:newEmulatorID]) return;
	
	[emulatorID release];
	emulatorID = [newEmulatorID copy];

	[self _OE_writeDataToPlist];
 }

-(void)setUserDescription:(NSString *)newDescription{
	if([userDescription isEqual:newDescription]) return;
	
	[userDescription release];
	userDescription = [newDescription copy];
	
	[self _OE_writeDataToPlist];
}

-(void)_OE_writeDataToPlist{
	NSDictionary *dictionary = [NSDictionary dictionaryWithObjectsAndKeys:
								[self emulatorID], @"emulatorID",
								[self userDescription], @"userDescription",
								nil];
	
	//create the parent directories if needed
	[self _OE_setupBundleContents];
	
	[dictionary writeToFile:[self infoPlistPath] 
				 atomically:YES];
}

-(NSDictionary *)_OE_readDataFromPlist{
	return [NSDictionary dictionaryWithContentsOfFile:[self infoPlistPath]];
}
	 
-(NSString *)infoPlistPath{
	return [[self bundlePath] stringByAppendingPathComponent:@"Info.plist"];
}

-(NSString *)screenshotPath{
	return [[self resourcePath] stringByAppendingPathComponent:@"screenshot.tiff"];
}

-(NSString *)saveDataPath{
	return [[self resourcePath] stringByAppendingPathComponent:@"save.data"];
}

-(NSBundle *)bundle{
	return [NSBundle bundleWithPath:[self bundlePath]];
}

-(NSData *)saveData{
	return [NSData dataWithContentsOfFile:[self saveDataPath]];
}

-(void)setSaveData:(NSData *)saveData{
	[[NSFileManager defaultManager] createFileAtPath:[self saveDataPath]
											contents:saveData
										  attributes:nil];
}

-(NSImage *)screenshot{
	return [[[NSImage alloc] initWithContentsOfFile:[self screenshotPath]] autorelease];
}

-(void)setScreenshot:(NSImage *)newScreenshot{
	[[NSFileManager defaultManager] createFileAtPath:[self screenshotPath]
											contents:[newScreenshot TIFFRepresentation]
										  attributes:nil];
}

-(NSDate *)timeStamp{
	return (NSDate *)[[[NSFileManager defaultManager] attributesOfItemAtPath:[self bundlePath] 
																	   error:nil] objectForKey:NSFileModificationDate];
}

- (id) imageRepresentation
{
	return [self screenshot];
}

- (NSString *)imageRepresentationType
{
    // We use this representation type because we are storing the image as binary data.
    return IKImageBrowserNSImageRepresentationType;
}

- (NSString *)imageUID
{
    // This is uses the NSManagedObjectID for the entity to generate a unique string.
    return [[[self objectID] URIRepresentation] description];
}

- (NSString *)imageTitle
{
    //return [self.timeStamp descriptionWithCalendarFormat:@"%m/%d/%Y %H:%M:%S" timeZone:nil locale:nil];
    return [[self romFile] name];
}

- (NSString *)imageSubtitle
{
    return [self.timeStamp descriptionWithCalendarFormat:@"%m/%d/%Y %H:%M:%S" timeZone:nil locale:nil];
}

@end
