//
//  OEGridViewCell+OEGridView.h
//  OEGridViewDemo
//
//  Created by Faustino Osuna on 2/9/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import "OEGridViewCell.h"

@interface OEGridViewCell (OEGridView)

@property (nonatomic, assign, setter = _setIndex:) NSUInteger _index;

@end
