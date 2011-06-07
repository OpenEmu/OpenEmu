//
//  AttributedTextFieldCell.h
//  OEPreferencesMockup
//
//  Created by Christoph Leimbrock on 30.05.11.
//  Copyright 2011 none. All rights reserved.
//

#import <Foundation/Foundation.h>


@interface AttributedTextFieldCell : NSTextFieldCell {
@private
    NSDictionary* textAttributes;
}
- (void)setupAttributes;
@property (retain, readwrite) NSDictionary* textAttributes;
@end
