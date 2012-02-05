//
//  OECheckBox.h
//  OEPreferencesMockup
//
//  Created by Christoph Leimbrock on 29.05.11.
//  Copyright 2011 none. All rights reserved.
//

#import <Foundation/Foundation.h>


@interface OECheckBox : NSButton 
- (void)setupCell;
- (NSPoint)badgePosition;
@end

@interface OECheckBoxCell : NSButtonCell
@end