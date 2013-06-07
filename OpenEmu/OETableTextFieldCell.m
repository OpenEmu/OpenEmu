/*
 Copyright (c) 2012, OpenEmu Team

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

#import "OETableTextFieldCell.h"

@interface OETableTextFieldCell ()

- (void)OE_tableTextFieldCellInit;

@end


@implementation OETableTextFieldCell

- (void)OE_tableTextFieldCellInit
{
    [self setFocusRingType:NSFocusRingTypeNone];
}

- (id)initWithCoder:(NSCoder *)decoder
{
    self = [super initWithCoder:decoder];
    if(self)
        [self OE_tableTextFieldCellInit];
    return self;
}

- (id)initTextCell:(NSString *)string
{
    self = [super initTextCell:string];
    if(self)
        [self OE_tableTextFieldCellInit];
    return self;
}

- (id)initImageCell:(NSImage *)image
{
    self = [super initImageCell:image];
    if(self)
        [self OE_tableTextFieldCellInit];
    return self;
}

- (void)OE_prepareForEditing
{
    [self setBackgroundColor:[NSColor whiteColor]];
    [self setTextColor:[NSColor blackColor]];
}

- (void)editWithFrame:(NSRect)rect inView:(NSView *)controlView editor:(NSText *)textObj delegate:(id)object event:(NSEvent *)event
{
    [self OE_prepareForEditing];
    [super editWithFrame:[self titleRectForBounds:rect] inView:controlView editor:textObj delegate:object event:event];
}

- (void)selectWithFrame:(NSRect)rect inView:(NSView *)controlView editor:(NSText *)textObj delegate:(id)object start:(NSInteger)selStart length:(NSInteger)selLength
{
    [self OE_prepareForEditing];
    [super selectWithFrame:[self titleRectForBounds:rect] inView:controlView editor:textObj delegate:object start:selStart length:selLength];
}

- (NSText *)setUpFieldEditorAttributes:(NSText *)textObj
{
    if([textObj isKindOfClass:[NSTextView class]])
    {
        NSTextView *textView = (NSTextView *)textObj;
        [textView setSelectedTextAttributes:@{ NSBackgroundColorAttributeName : [NSColor colorWithDeviceRed:0.788 green:0.871 blue:0.992 alpha:1.0]}]; // copied from OESidebarCell
        [textView setInsertionPointColor:[NSColor colorWithDeviceWhite:0.72 alpha:1.0]];                                                               // copied from OESidebarCell
        [textView setTextContainerInset:NSZeroSize];
        [[textView textContainer] setLineFragmentPadding:0];
    }

    return textObj;
}

@end
