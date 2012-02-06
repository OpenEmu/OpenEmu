//
//  OEPopUpButton.h
//  OEPreferencesMockup
//
//  Created by Christoph Leimbrock on 04.06.11.
//  Copyright 2011 none. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "OEMenu.h"


@interface OEPopupButton : NSPopUpButton <OEMenuDelegate>
{
@private
    OEMenu *oemenu;
    NSMenuItem *selectedItem;
}
@property (retain, setter = setOEMenu:) OEMenu *oemenu;
@end
