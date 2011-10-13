//
//  GameViewButton.h
//  OpenEmu
//
//  Created by Christoph Leimbrock on 12.06.11.
//  Copyright 2011 none. All rights reserved.
//

#import <Foundation/Foundation.h>

enum _OEHUDButtonColor 
{
	OEHUDButtonColorDefault,
	OEHUDButtonColorBlue,
	OEHUDButtonColorRed
}
typedef OEHUDButtonColor;

@interface OEHUDButtonCell : NSButtonCell 
{
@private
    OEHUDButtonColor buttonColor;
}
@property OEHUDButtonColor buttonColor;
@end
