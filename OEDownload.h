//
//  OEDownload.h
//  OpenEmu
//
//  Created by Joshua Weinberg on 8/2/09.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class SUAppcastItem, SUAppcast;

@interface OEDownload : NSObject {
	BOOL enabled;
	
	NSString* name;
	SUAppcastItem* appcastItem;
}

@property (assign) BOOL enabled;
@property (retain) SUAppcastItem* appcastItem;

- (id) initWithAppcast: (SUAppcast*) appcast;

@end
