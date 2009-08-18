//
//  OEROMFileSystemTypeTransformer.h
//  OpenEmu
//
//  Created by Steve Streza on 8/15/09.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "OEROMFile.h"

@interface OEROMFileSystemTypeTransformer : NSValueTransformer

//creates an autoreleased transformer.
+(id)transformer;

@end

@interface OEROMFileSystemTypeReverseTransformer : OEROMFileSystemTypeTransformer
@end