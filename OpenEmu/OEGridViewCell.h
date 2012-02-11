//
//  OEGridViewCell.h
//  OEGridViewDemo
//
//  Created by Faustino Osuna on 2/9/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import <QuartzCore/QuartzCore.h>
#import "OEGridLayer.h"

@class OEGridView;

@interface OEGridViewCell : OEGridLayer
{
@private
    NSUInteger _index;

    CALayer *_foregroundLayer;
    BOOL _selected;
    BOOL _editing;
}

- (void)prepareForReuse;
- (void)didBecomeFocused;
- (void)willResignFocus;
- (void)setSelected:(BOOL)selected animated:(BOOL)animated;

#pragma mark -
@property (nonatomic, readonly) id draggingImage;
@property (nonatomic, assign, getter = isSelected) BOOL selected;
@property (nonatomic, assign, getter = isEditing) BOOL editing;
@property (nonatomic, retain) CALayer *foregroundLayer;
@property (nonatomic, readonly) OEGridView *gridView;
@property (nonatomic, readonly) NSRect hitRect;

@end
