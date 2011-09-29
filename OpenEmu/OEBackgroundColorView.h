//
//  BackgroundColorView.h
//  OpenEmu
//
//  Created by Christoph Leimbrock on 30.03.11.
//  Copyright 2011 none. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface OEBackgroundColorView : NSView {
@private
    NSColor* backgroundColor;
}
@property (retain, nonatomic) NSColor* backgroundColor;

@end
