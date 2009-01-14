//
//  AppData.m
//  ImageTextCellSample
//
//  Created by Martin Kahr on 04.05.07.
//  Copyright 2007 CASE Apps. All rights reserved.
//

#import "PluginInfo.h"

@implementation PluginInfo

@synthesize bundle;

- (id) initWithBundleAtPath: (NSString*) path {
	bundle = [NSBundle bundleWithPath: path];
	if (bundle == nil) {
		[self autorelease];
		return nil;
	}
	
	
	NSDictionary* infoDict = [bundle infoDictionary];
	
	NSString * iconPath = [bundle pathForResource:[infoDict objectForKey:@"CFIconName"] ofType:@"icns"];
	return [self initWithInfoDictionary:[bundle infoDictionary] icon: [[NSImage alloc] initWithContentsOfFile:iconPath]];
}

- (id) initWithInfoDictionary: (NSDictionary*) infoDict icon: (NSImage*) image {
	if (self = [super init]) {
		infoDictionary = [infoDict retain];
		icon = [image retain];
	}
	return self;
}

// When an instance is assigned as objectValue to a NSCell, the NSCell creates a copy.
// Therefore we have to implement the NSCopying protocol
- (id)copyWithZone:(NSZone *)zone {
    PluginInfo *copy = [[[self class] allocWithZone: zone] initWithInfoDictionary:infoDictionary icon: icon];
	return copy;
}

- (void) dealloc {
	[infoDictionary release];
	[icon release];
	[super dealloc];
}

- (NSString*) displayName {
	NSString* displayName = [infoDictionary objectForKey: @"CFBundleName"];
	if (displayName) return displayName;
	return [infoDictionary objectForKey: @"CFBundleExecutable"];
}
- (NSString*) details {
	return [NSString stringWithFormat: @"Version %@", [infoDictionary objectForKey: @"CFBundleVersion"]];
}
- (NSImage*) icon {
	return icon;
}

@end
