// Copyright (c) 2010 Olivier Halligon
// Copyright (c) 2020 OpenEmu Team
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.


// based on OHAutoNIBi18n (https://github.com/AliSoftware/OHAutoNIBi18n)

#import <objc/runtime.h>
@import AppKit;

static inline NSString* oe_localizedString(NSString* string);
static inline void oe_localizeNSButton(NSButton* button);
static inline void oe_localizeNSTextField(NSTextField* textField);

#pragma mark -

@interface NSObject(OEi18n)
-(void)oe_awakeFromNib;
@end


@implementation NSObject(OEi18n)

- (void)oe_awakeFromNib
{
    if([self isKindOfClass:[NSButton class]])
        oe_localizeNSButton((NSButton*)self);
    
    else if([self isKindOfClass:[NSTextField class]])
        oe_localizeNSTextField((NSTextField*)self);
    
    // call the original awakeFromNib method
    [self oe_awakeFromNib]; // this actually calls the original awakeFromNib (and not oe_awakeFromNib) because we did some method swizzling
}

+ (void)load
{
    // swizzle -awakeFromNib with -oe_awakeFromNib as soon as the app (and thus this class) is loaded
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        Method oe_awakeFromNib = class_getInstanceMethod([NSObject class], @selector(oe_awakeFromNib));
        Method awakeFromNib = class_getInstanceMethod([NSObject class], @selector(awakeFromNib));
        method_exchangeImplementations(awakeFromNib, oe_awakeFromNib);
    });
}

@end

#pragma mark -

static inline NSString* oe_localizedString(NSString* string)
{
    if (string == nil || string.length == 0 || string == string.uppercaseString)
        return string;
    
    return [NSBundle.mainBundle localizedStringForKey:string value:nil table:@"OEControls"];
}

static inline void oe_localizeNSButton(NSButton* button) {
    button.title = oe_localizedString(button.title);
}

static inline void oe_localizeNSTextField(NSTextField* textField) {
    textField.stringValue = oe_localizedString(textField.stringValue);
}
