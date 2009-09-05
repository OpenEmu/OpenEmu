//
//  NSString+Aliases.h
//  OpenEmu
//
//  Created by Steve Streza on 9/3/09.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface NSString (OEAliases)
+(NSString *)OE_stringWithPathOfAliasData:(NSData *)aliasData;
-(NSString *)  OE_initWithPathOfAliasData:(NSData *)aliasData;

-(NSData *)OE_pathAliasData;
@end
