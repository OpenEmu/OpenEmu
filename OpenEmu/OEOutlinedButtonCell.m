/*
 Copyright (c) 2014, OpenEmu Team

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

#import "OEOutlinedButtonCell.h"
#import "OEThemeTextAttributes.h"

@implementation OEOutlinedButtonCell

- (void)drawBezelWithFrame:(NSRect)frame inView:(NSView *)controlView
{
    [[NSGraphicsContext currentContext] saveGraphicsState];

    frame = NSInsetRect(frame, 1.0, 2.0);

    NSDictionary *attribtues = [[self themeTextAttributes] textAttributesForState:OEThemeStateDefault];
    NSColor *color = [attribtues objectForKey:NSForegroundColorAttributeName];
    [color setStroke];

    NSShadow *shadow = [attribtues objectForKey:NSShadowAttributeName];
    [shadow set];

    NSBezierPath *bezierPath = [NSBezierPath bezierPathWithRoundedRect:frame xRadius:3.0 yRadius:3.0];
    [bezierPath setLineWidth:1.0];
    [bezierPath stroke];

    [[NSGraphicsContext currentContext] restoreGraphicsState];
}

- (NSSize)cellSize
{
    NSSize size = [super cellSize];

    size.height -= 2.0;

    return size;
}
@end
