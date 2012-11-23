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
@synthesize widthInset;

- (id)init
{
    if((self = [super init]))
    {
        [self setWidthInset:3];
    }
    
    return self;
}

- (id)initImageCell:(NSImage *)image
{
    if(self = [super initImageCell:image])
    {
        [self setWidthInset:3];
    }
    
    return self;
}

- (id)initTextCell:(NSString *)aString
{
    if((self = [super initTextCell:aString]))
    {
        [self setWidthInset:3];
    }
    
    return self;
}
- (id)initWithCoder:(NSCoder *)coder
{
    if((self = [super initWithCoder:coder]))
    {
        [self setWidthInset:3];
    }
    
    return self;
}

- (id)copyWithZone:(NSZone *)zone
{
    OECenteredTextFieldCell *copy = [super copyWithZone:zone];
    
    [copy setWidthInset:[self widthInset]];
    return copy;
}

- (NSSize)insetForFrame:(NSRect)cellFrame
{
    CGFloat heightInset = (cellFrame.size.height - [self cellSize].height) / 2;
    return (NSSize){[self widthInset], heightInset};
}

#pragma mark -
#pragma mark Drawing

- (void)drawInteriorWithFrame:(NSRect)cellFrame inView:(NSView *)controlView
{
    NSSize contentSize = [self cellSize];
    
    cellFrame.origin.y += (cellFrame.size.height - contentSize.height) / 2.0;
    cellFrame.size.height = contentSize.height;
    
    cellFrame = NSInsetRect(cellFrame, self.widthInset, 0);
    
    [[self attributedStringValue] drawInRect:cellFrame];
}

@end
