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

#import "OESidebarFieldEditor.h"
#import "OEBackgroundColorView.h"

@implementation OESidebarFieldEditor
@synthesize container;

static void *const _OESidebarFieldEditorContext = (void *)&_OESidebarFieldEditorContext;

+ (id)fieldEditor
{
    static OESidebarFieldEditor *fieldEditor = nil;
    
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        fieldEditor = [[OESidebarFieldEditor alloc] initWithFrame:NSZeroRect];
        
        [fieldEditor setFieldEditor:YES];
        [fieldEditor setEditable:YES];
        [fieldEditor setSelectable:YES];
        
        [fieldEditor setTextContainerInset:NSMakeSize(1, 2)];
    });
    
    return fieldEditor;
}

- (NSRect)superFrame
{
    return [[self superview] frame];
}

- (void)viewWillMoveToSuperview:(NSView *)newSuperview
{
    if(newSuperview == nil && [self superview] != nil)
    {
        [[self superview] removeObserver:self forKeyPath:@"frame" context:_OESidebarFieldEditorContext];
    }
    else if(newSuperview != nil)
    {
        [newSuperview addObserver:self forKeyPath:@"frame" options:0xF context:_OESidebarFieldEditorContext];
    }
}

- (void)updateContainerFrame
{
    if([self superview] != nil && [self container] != nil)
    {
        NSRect rect = NSInsetRect([[self superview] frame], -1, -1);
        
        rect.size.width -= 1;
        
        [[self container] setFrame:rect];
    }
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    if(context == _OESidebarFieldEditorContext)
        [self updateContainerFrame];
    else
        [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
}

- (void)viewDidMoveToWindow
{
    [super viewDidMoveToWindow];
    
    if([self window]    != nil &&
       [self superview] != nil &&
       [NSStringFromClass([[self superview] class]) isEqualToString:@"_NSKeyboardFocusClipView"])
    {
        OEBackgroundColorView *cont = [[OEBackgroundColorView alloc] initWithFrame:NSInsetRect([self superFrame], -1, -1)];
        cont.backgroundColor = [NSColor colorWithDeviceRed:0.09 green:0.153 blue:0.553 alpha:1.0];
        [self setContainer:cont];
        
        [self updateContainerFrame];
        
        [[[self superview] superview] addSubview:[self container] positioned:NSWindowBelow relativeTo:[self superview]];

        dispatch_after(DISPATCH_TIME_NOW, dispatch_get_main_queue(), ^(void){
            [self setNeedsDisplayInRect:[self bounds] avoidAdditionalLayout:YES];
        });
    }
    else if([self container] != nil)
    {
        [[self container] removeFromSuperview];
        [self setContainer:nil];
    }
}

- (void)setFrameSize:(NSSize)size
{
    size.width = ceil(size.width);
    
    [super setFrameSize:size];
}

- (void)drawRect:(NSRect)dirtyRect
{
    [self setBackgroundColor:[NSColor whiteColor]];    
    [super drawRect:dirtyRect];
}

- (void)dealloc
{
    [[self container] removeFromSuperview];
    [self setContainer:nil];    
}

@end
