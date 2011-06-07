//
//  OEDBCollection.h
//  OpenEmuMockup
//
//  Created by Christoph Leimbrock on 10.04.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>


@interface OEDBCollection : NSObject <NSCopying> {
    NSString* name;
	NSMutableArray* items;
}

@property (readwrite, retain) NSString* name;
@property (readwrite, retain) NSMutableArray* items;

@end
