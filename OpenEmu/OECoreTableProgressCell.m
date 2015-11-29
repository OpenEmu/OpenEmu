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

#import "OECoreTableProgressCell.h"
#import "OETheme.h"
#import "OEThemeImage.h"

#import "OpenEmu-Swift.h"

@implementation OECoreTableProgressCell

- (id)init 
{
    self = [super init];
    if (self) 
    {
        [self setWidthInset:9.0];
    }
    return self;
}
- (id)initImageCell:(NSImage *)image
{
    self = [super initImageCell:image];
    if (self)
    {
        [self setWidthInset:9.0];
    }
    return self;
}

- (id)initTextCell:(NSString *)aString
{
    self = [super initTextCell:aString];
    if (self)
    {
        [self setWidthInset:9.0];
    }
    return self;
}
- (id)initWithCoder:(NSCoder *)aDecoder
{
    self = [super initWithCoder:aDecoder];
    if(self)
    {
        [self setWidthInset:9.0];
    }
    return self;
}

- (id)copyWithZone:(NSZone *)zone
{
    OECoreTableProgressCell *copy = [super copyWithZone:zone];
    [copy setWidthInset:[self widthInset]];
    return copy;
}
@synthesize widthInset;
- (void)drawWithFrame:(NSRect)cellFrame inView:(NSView *)controlView
{
    cellFrame = NSInsetRect(cellFrame, [self widthInset], (cellFrame.size.height-15)/2);
    
    NSRect trackRect = cellFrame;
    NSImage *image = [[OETheme sharedTheme] imageForKey:@"install_progress_track" forState:OEThemeStateDefault];
    [image drawInRect:trackRect fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:nil];
    
    NSRect progressRect = cellFrame;
    float value = [[self objectValue] floatValue];
    if(value==0.0) return;
    if(value>=1.0) value = 1.0;
    progressRect.size.width = value*progressRect.size.width;
    progressRect.size.width = progressRect.size.width < 12 ? 12 : roundf(progressRect.size.width);

    image = [[OETheme sharedTheme] imageForKey:@"install_progress_bar" forState:OEThemeStateDefault];
    [image drawInRect:progressRect fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:nil];
}
@end
