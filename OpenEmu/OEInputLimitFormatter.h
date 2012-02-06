//
//  OEInputLimitFormatter.h
//  OpenEmu
//
//  Created by Carl Leimbrock on 06.02.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface OEInputLimitFormatter : NSFormatter
- (id)initWithLimit:(int)aLimit;
@property int limit;
@end
