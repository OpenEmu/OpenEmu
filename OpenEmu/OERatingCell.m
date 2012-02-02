//
//  OERatingCell.m
//  OpenEmuMockup
//
//  Created by Christoph Leimbrock on 20.04.11.
//  Copyright 2011 none. All rights reserved.
//

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
