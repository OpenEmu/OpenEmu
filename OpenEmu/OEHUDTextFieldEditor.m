//
//  OEHUDTextFieldEditor.m
//  OpenEmu
//
//  Created by Carl Leimbrock on 24.11.11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#import "OEHUDTextFieldEditor.h"
#import "OEBackgroundImageView.h"

#import "OEUIDrawingUtils.h"
@implementation OEHUDTextFieldEditor

- (id)init{
    self = [super init];
    if (self) {
    }
    return self;
}

+ (id)fieldEditor{
    static OEHUDTextFieldEditor *fieldEditor = nil;
    if (fieldEditor == nil){
        fieldEditor = [[OEHUDTextFieldEditor alloc] initWithFrame:NSZeroRect];
        
        [fieldEditor setFieldEditor:YES];
        [fieldEditor setEditable:YES];
        [fieldEditor setSelectable:YES];
        
        [fieldEditor setTextContainerInset:NSMakeSize(3, 4)];
    }
    return fieldEditor;
}


- (void)setFrameSize:(NSSize)newSize
{
    newSize.width = ceil(newSize.width);
    [super setFrameSize:newSize];
}

- (void)dealloc {
    [super dealloc];
}

@end
