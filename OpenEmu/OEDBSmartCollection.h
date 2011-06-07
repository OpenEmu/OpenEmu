//
//  OEDBSmarCollection.h
//  OpenEmuMockup
//
//  Created by Christoph Leimbrock on 10.04.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>


@interface OEDBSmartCollection : NSObject <NSCopying> {
	NSString* name;
	NSPredicate* predicate; 
}
@property (readwrite, retain) NSString* name;
@property (readwrite, retain) NSPredicate* predicate;
@end
