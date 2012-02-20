//
//  OEGridView+OEGridViewCell.h
//  OEGridViewDemo
//
//  Created by Faustino Osuna on 2/11/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import "OEGridView.h"

@interface OEGridView (OEGridViewCell)

- (void)_willBeginEditingCell:(OEGridViewCell *)cell;
- (void)_didEndEditingCell:(OEGridViewCell *)cell;

@end
