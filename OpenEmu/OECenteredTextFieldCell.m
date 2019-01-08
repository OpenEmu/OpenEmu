/*
 Copyright (c) 2011, OpenEmu Team
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
     * Neither the name of the OpenEmu Team nor the
       names of its contributors may be used to endorse or promote products
       derived from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY OpenEmu Team ''AS IS'' AND ANY
 EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL OpenEmu Team BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#import "OECenteredTextFieldCell.h"

@implementation OECenteredTextFieldCell

- (void)OE_centeredTextFieldCellInit
{
    [self setWidthInset:3.0];
}

- (id)init
{
    if((self = [super init]))
        [self OE_centeredTextFieldCellInit];

    return self;
}

- (id)initTextCell:(NSString *)aString
{
    if((self = [super initTextCell:aString]))
        [self OE_centeredTextFieldCellInit];

    return self;
}

- (id)initWithCoder:(NSCoder *)coder
{
    if((self = [super initWithCoder:coder]))
        [self OE_centeredTextFieldCellInit];

    return self;
}

- (id)copyWithZone:(NSZone *)zone
{
    OECenteredTextFieldCell *copy = [super copyWithZone:zone];
    
    [copy setWidthInset:[self widthInset]];
    return copy;
}

- (NSRect)titleRectForBounds:(NSRect)bounds
{
    NSRect titleRect = [super titleRectForBounds:bounds];
    NSSize contentSize  = [self cellSize];
    const CGFloat heightInset = (titleRect.size.height - contentSize.height) / 2.0;
    titleRect = NSInsetRect(titleRect, [self widthInset], heightInset);
    return titleRect;
}

#pragma mark -
#pragma mark Drawing
- (void)drawInteriorWithFrame:(NSRect)cellFrame inView:(NSView *)controlView
{
    [[self attributedStringValue] drawInRect:[self titleRectForBounds:cellFrame]];
}

@end
