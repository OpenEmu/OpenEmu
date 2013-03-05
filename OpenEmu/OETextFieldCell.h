//
//  OETextFieldCell.h
//  OEThemeFactory
//
//  Created by Christoph Leimbrock on 2/25/13.
//
//

#define OETextFieldCommonInit


#import <Cocoa/Cocoa.h>

#import "OETheme.h"
#import "OECell.h"
@interface OETextFieldCell : NSTextFieldCell <OECell>
- (void)setSelectedThemeTextAttributesKey:(NSString*)key;
@property OEThemeTextAttributes *selectedThemeTextAttributes;
@end
