//
//  OEDBConsole.h
//  OpenEmuMockup
//
//  Created by Christoph Leimbrock on 10.04.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>


@interface OEDBConsole : NSObject <NSCopying>{
	id gameCore;    // actual core associated with the 
		
	NSString* name;
	NSImage* icon;
}
- (id)copyWithZone:(NSZone *)zone;
@property (readwrite, retain) NSString* name;
@property (readwrite, retain) NSImage* icon;

@end
