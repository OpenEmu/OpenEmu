//
//  OECenteredTextFieldWithWeblinkCell.h
//  OpenEmu
//
//  Created by Carl Leimbrock on 14.11.11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#import "OECenteredTextFieldCell.h"
#import "OEImageButton.h"
@interface OECenteredTextFieldWithWeblinkCell : OECenteredTextFieldCell
@property SEL buttonAction;
@property (assign) id buttonTarget;
@property (retain) OEImageButtonHoverPressed *buttonCell;
@end
