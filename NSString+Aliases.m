//
//  NSString+Aliases.m
//  OpenEmu
//
//  Created by Steve Streza on 9/3/09.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import "NSString+Aliases.h"

/*
 Use this with APIs that return an OSStatus. If the function returns
 anything but noErr, then it will log the calling function and an
 error message specified by the caller. It will then use a goto
 (this is one time when goto! is useful) to jump to wherever you
 place an OECatch(), skipping the processing steps that likely
 don't matter if there's an error.
 */
#define OEThrowIfError(__cmd, __err, ...) \
	do{ \
		err = (__cmd); \
		if(err != noErr){ \
			NSLog(@"%s " __err, __PRETTY_FUNCTION__, ##__VA_ARGS__); \
			goto OEFail; \
		} \
	}while(0)

#define OECatch() \
OEFail:

@implementation NSString (OEAliases)

+(NSString *)OE_stringWithPathOfAliasData:(NSData *)aliasData{
	if(!aliasData) return nil;
	
	AliasHandle handle = NULL;
	NSString *pathString = nil;
	
    //Look mah! IM LEGACY
    OSErr err = noErr;
	
	OEThrowIfError(PtrToHand([aliasData bytes], (Handle *)&handle, [aliasData length]), @"PtrToHand");;
    
    FSRef fileRef;
    Boolean wasChanged;
    OEThrowIfError(FSResolveAlias(NULL, handle, &fileRef, &wasChanged), @"FSResolveAlias");
    
    char path[1024];
    
    OEThrowIfError(FSRefMakePath(&fileRef, (UInt8 *)path, 1024), @"FSRefMakePath");
    
    pathString = [NSString stringWithCString:path encoding:NSUTF8StringEncoding];
    pathString = [pathString stringByStandardizingPath];	
	
	OECatch();
	if(handle){
		DisposeHandle((Handle)handle);
		handle = NULL;
	}
		
	return pathString;
}

-(NSString *)  OE_initWithPathOfAliasData:(NSData *)aliasData{
	return [[[self class] OE_stringWithPathOfAliasData:aliasData] copy];
}

-(NSData *)OE_pathAliasData{
	AliasHandle handle = NULL;
	Boolean isDirectory;

	OSErr err = noErr;
	NSData *aliasData = nil;
	
	OEThrowIfError(FSNewAliasFromPath( NULL, [self UTF8String], 0, &handle, &isDirectory ), @"FSNewAliasFromPath");
	
	long aliasSize = GetAliasSize(handle);
	aliasData = [NSData dataWithBytes:*handle length:aliasSize];
	
	OECatch();
	if(handle){
		DisposeHandle((Handle)handle);
	}
	
	return aliasData;
}

@end
