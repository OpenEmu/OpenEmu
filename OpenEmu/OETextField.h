//
//  OETextField.h
//  OEThemeFactory
//
//  Created by Christoph Leimbrock on 2/25/13.
//
//

#import <Cocoa/Cocoa.h>
#import "OETextFieldCell.h"
#import "OEControl.h"

@interface OETextField : NSTextField <OEControl>
@property (nonatomic) OEThemeTextAttributes *selectedThemeTextAttributes;
@end
