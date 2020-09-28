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

#import "OEGridViewFieldEditor.h"

#pragma mark -

@implementation OEGridViewFieldEditor
{
    NSTextField *textView;
}

@synthesize borderColor;

- (id)initWithFrame:(NSRect)frame
{
    if((self = [super initWithFrame:frame]))
    {
        textView = [[NSTextField alloc] initWithFrame:NSMakeRect(10, 10, 80, 80)];
        [[textView cell] setUsesSingleLineMode:YES];
        [[textView cell] setScrollable:YES];
        [textView setBezeled:NO];
        [textView setAllowsEditingTextAttributes:NO];
        [textView setBackgroundColor:NSColor.textBackgroundColor];
        [textView setTextColor:NSColor.textColor];
        [self addSubview:textView];

        [self setAutoresizesSubviews:NO];
        [self setHidden:YES];
        [self setWantsLayer:YES];

        NSFont *fieldEditorFont = [NSFont boldSystemFontOfSize:12];
        [self setAlignment:NSTextAlignmentCenter];
        [self setBorderColor:[NSColor blackColor]];
        [self setFont:fieldEditorFont];
        
        CALayer *layer = [self layer];
        [layer setShadowOpacity:0.45];
        [layer setShadowColor:[[NSColor blackColor] CGColor]];
        [layer setShadowOffset:CGSizeMake(0.0, -6.0)];
        [layer setShadowRadius:5];
    }
    
    return self;
}

#pragma mark -

- (void)drawRect:(NSRect)dirtyRect
{
    [super drawRect:dirtyRect];
    
    [[self borderColor] setStroke];
    
    NSBezierPath *borderPath = [NSBezierPath bezierPathWithRect:NSInsetRect([self bounds], 0.5, 0.5)];
    [borderPath stroke];
    
    [[NSColor whiteColor] setStroke];
    borderPath = [NSBezierPath bezierPathWithRect:NSInsetRect([self bounds], 1.5, 1.5)];
    [borderPath stroke];
}

#pragma mark -

- (void)setFrameSize:(NSSize)newSize
{
    [super setFrameSize:newSize];
    
    if(newSize.width  >= 2) newSize.width -= 2;
    if(newSize.height >= 2) newSize.height -= 2;
    
    [textView setFrameSize:newSize];
    [textView setFrameOrigin:NSMakePoint(1, 1)];
}

#pragma mark - Accessors

- (NSString *)string                            { return [textView stringValue]; }
- (void)setString:(NSString *)newString         { [textView setStringValue:newString]; }

- (NSTextAlignment)alignment                    { return [textView alignment]; }
- (void)setAlignment:(NSTextAlignment)alignment { [textView setAlignment:alignment]; }

- (NSFont *)font                                { return [textView font]; }
- (void)setFont:(NSFont *)newFont               { [textView setFont:newFont]; }

- (id)delegate                                  { return [textView delegate]; }
- (void)setDelegate:(id)delegate                { [textView setDelegate:delegate]; }

@end
