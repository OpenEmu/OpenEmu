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
#import "OEDBDataSourceAdditions.h"
#import "NSImage+OEDrawingAdditions.h"
@implementation OERatingCell

#pragma mark -
- (void)drawWithFrame:(NSRect)cellFrame inView:(NSView *)controlView{
	OEUIState state = OEUIStateInactive;
	if([self isHighlighted]) state = OEUIStateActive;
	
	NSImage *image = [NSImage imageNamed:@"list_rating"];
		
	NSRect sourceRect = [self imageRectForValue:[[self objectValue] intValue] andState:state];
	NSRect targetRect = NSMakeRect(cellFrame.origin.x+(cellFrame.size.width-sourceRect.size.width)/2, cellFrame.origin.y+(cellFrame.size.height-sourceRect.size.height)/2, sourceRect.size.width, sourceRect.size.height);
	[image drawInRect:NSIntegralRect(targetRect) fromRect:sourceRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:nil leftBorder:0 rightBorder:0 topBorder:0 bottomBorder:0];
}

- (NSRect)imageRectForValue:(int)rating andState:(OEUIState)state{
	NSRect imageRect = NSMakeRect(55, 55-(rating*11), 55, 11);
	if(state==OEUIStateInactive){
		imageRect.origin.x = 0;
	}
	
	return imageRect;
}
#pragma mark -
- (BOOL)startTrackingAt:(NSPoint)startPoint inView:(NSView *)controlView{
	[self determineRatingForPos:startPoint inTableView:(NSTableView*)controlView];
	return YES;
}
- (BOOL)continueTracking:(NSPoint)lastPoint at:(NSPoint)pos inView:(NSView *)controlView{
	[self determineRatingForPos:pos inTableView:(NSTableView*)controlView];
	return YES;
}
- (void)stopTracking:(NSPoint)lastPoint at:(NSPoint)stopPoint inView:(NSView *)controlView mouseIsUp:(BOOL)flag{
	[self determineRatingForPos:stopPoint inTableView:(NSTableView*)controlView];
}
#pragma mark -
- (void)determineRatingForPos:(NSPoint)pos inTableView:(NSTableView*)view{
	NSUInteger row = [view rowAtPoint:pos];
	NSUInteger col = [view columnAtPoint:pos];
	NSTableColumn *column = [[view tableColumns] objectAtIndex:col];
	
	NSRect frame = [view frameOfCellAtColumn:col row:row];
	
	
	float minRatingX = (frame.size.width-55)/2;
	float posX = pos.x-frame.origin.x-minRatingX;
	float stepWidth = 55/5;
	
	
	int rating = 0;
	if(posX<=0){ rating =  0; }
	else if(posX>=55-stepWidth){ rating = 5; }
	else rating = ((posX/stepWidth)+1);
	
	NSNumber *objValue = [NSNumber numberWithInt:rating];
	[[view dataSource] tableView:view setObjectValue:objValue forTableColumn:column row:row];
	[self setObjectValue:objValue];
}
	   
@end
