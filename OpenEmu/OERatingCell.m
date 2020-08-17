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
#import "OERatingCell.h"

@implementation OERatingCell

#pragma mark -

- (void)drawWithFrame:(NSRect)cellFrame inView:(NSView *)controlView
{
	NSImage *image = [NSImage imageNamed:@"list_rating"];

	NSRect sourceRect = [self imageRectForRating:[[self objectValue] intValue]];
	NSRect targetRect = NSMakeRect(floor(cellFrame.origin.x + (cellFrame.size.width - sourceRect.size.width) / 2),
                                   floor(cellFrame.origin.y + (cellFrame.size.height - sourceRect.size.height) / 2),
                                   sourceRect.size.width,
                                   sourceRect.size.height);
    [image drawInRect:targetRect fromRect:sourceRect operation:NSCompositingOperationSourceOver fraction:1.0 respectFlipped:YES hints:nil];
}

- (NSRect)imageRectForRating:(NSInteger)rating
{
	NSRect imageRect = NSMakeRect(55, 55 - (rating * 11), 55, 11);
    
	if(!self.isHighlighted) imageRect.origin.x = 0;
	
	return imageRect;
}

#pragma mark -

- (BOOL)startTrackingAt:(NSPoint)startPoint inView:(NSTableView *)controlView
{
	[self updateRatingWithPoint:startPoint inTableView:controlView];
	return YES;
}

- (BOOL)continueTracking:(NSPoint)lastPoint at:(NSPoint)pos inView:(NSTableView *)controlView
{
	[self updateRatingWithPoint:pos inTableView:controlView];
	return YES;
}

- (void)stopTracking:(NSPoint)lastPoint at:(NSPoint)stopPoint inView:(NSTableView *)controlView mouseIsUp:(BOOL)flag
{
	[self updateRatingWithPoint:stopPoint inTableView:controlView];
}

#pragma mark -

- (NSInteger)ratingForPoint:(NSPoint)aPoint inTableView:(NSTableView *)view;
{
	NSUInteger row = [view rowAtPoint:aPoint];
	NSUInteger col = [view columnAtPoint:aPoint];
	
	NSRect frame = [view frameOfCellAtColumn:col row:row];
	
	CGFloat minRatingX = (frame.size.width - 55) / 2;
	CGFloat posX       = aPoint.x - frame.origin.x - minRatingX;
	CGFloat stepWidth  = 55.0 / 5.0;
	
    return MAX(0, MIN((posX / stepWidth) + 1, 5));
}

- (void)updateRatingWithPoint:(NSPoint)aPoint inTableView:(NSTableView *)view;
{
	[self setObjectValue:@([self ratingForPoint:aPoint inTableView:view])];
}
	   
@end
