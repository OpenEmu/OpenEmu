//
//  OESearchField.h
//  OpenEmu
//
//  Created by Christoph Leimbrock on 16.04.13.
//
//

#import <Cocoa/Cocoa.h>
#import "OEControl.h"
#import "OESearchFieldCell.h"
@interface OESearchField : NSSearchField <OEControl>
@property (nonatomic) OEThemeTextAttributes *selectedThemeTextAttributes;
@end
