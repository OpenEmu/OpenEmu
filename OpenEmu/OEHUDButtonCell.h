//
//  GameViewButton.h
//  OpenEmu
//
//  Created by Christoph Leimbrock on 12.06.11.
//  Copyright 2011 none. All rights reserved.
//

#import <Foundation/Foundation.h>


@interface OEHUDButtonCell : NSButtonCell {
@private
    BOOL blue;
}
@property (getter = isBlue) BOOL blue;
@end
