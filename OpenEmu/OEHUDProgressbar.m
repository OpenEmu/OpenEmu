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

#import "OEHUDProgressbar.h"
#import "NSImage+OEDrawingAdditions.h"
@implementation OEHUDProgressbar


- (id)init {
    self = [super init];
    if (self) {
        self.minValue = 0.0;
        self.value = 0.5;
        self.maxValue = 1.0;
        
        [self addObserver:self forKeyPath:@"minValue" options:0 context:nil];
        [self addObserver:self forKeyPath:@"value" options:0 context:nil];
        [self addObserver:self forKeyPath:@"maxValue" options:0 context:nil];
    }
    return self;
}

- (void)dealloc {
    [self removeObserver:self forKeyPath:@"minValue"];
    [self removeObserver:self forKeyPath:@"value"];
    [self removeObserver:self forKeyPath:@"maxValue"];
    
    [super dealloc];
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    [self setNeedsDisplay:YES];
}

- (BOOL)isOpaque
{
    return NO;
}

- (void)drawRect:(NSRect)dirtyRect
{
    NSImage* trackImage = [NSImage imageNamed:@"hud_progress_bar_track"];
    [trackImage drawInRect:[self bounds] fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:nil leftBorder:7 rightBorder:7 topBorder:2 bottomBorder:2];
    
    if(self.value == 0.0) return;
    
    NSImage* barImage = [NSImage imageNamed:@"hud_progress_bar"];
    NSRect barRect = (NSRect){{0,0},{(roundf(self.bounds.size.width)*(self.value-self.minValue)/(self.maxValue-self.minValue)),self.bounds.size.height}};
    [barImage drawInRect:barRect fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:nil leftBorder:7 rightBorder:7 topBorder:1 bottomBorder:1];
}

- (void)mouseDown:(NSEvent *)theEvent
{
    if(self.value>=self.maxValue)
    {
        self.value = self.maxValue;
        return;
    }
    self.value = self.maxValue;
    
    if(self.value>self.maxValue)
    {
        self.value = self.minValue;
        return;
    }
}
@synthesize minValue,value,maxValue;
@end
