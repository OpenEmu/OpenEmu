//
//  AppData.h
//  ImageTextCellSample
//
//  Created by Martin Kahr on 04.05.07.
//  Copyright 2007 CASE Apps. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface PluginInfo : NSObject <NSCopying> {
	NSDictionary* infoDictionary;
	NSImage*  icon;
	NSBundle* bundle;
}

@property(readonly) NSBundle* bundle;

- (id) initWithBundleAtPath: (NSString*) path;
- (id) initWithInfoDictionary: (NSDictionary*) infoDict icon: (NSImage*) image;

- (NSString*) displayName;
- (NSString*) details;
- (NSImage*) icon;

@end
