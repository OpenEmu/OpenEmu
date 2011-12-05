//
//  OEPopUpButton.h
//  OEPreferencesMockup
//
//  Created by Christoph Leimbrock on 04.06.11.
//  Copyright 2011 none. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "OEMenu.h"
@interface OEControlsPopupButton : NSPopUpButton <OEMenuDelegate>
{
@private
    OEMenu* oemenu;
    NSMenuItem* selectedItem;
}
@property (retain) OEMenu* oemenu;
@end
