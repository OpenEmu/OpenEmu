/*
 Copyright (c) 2009, OpenEmu Team
 
 
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

#import "OEGameControllerView.h"
#import "OEGameCore.h"

#import "OEControlsKeyButton.h"
#import "OEControlsKeyLabelCell.h"


@implementation OEGameControllerView
NSRect RoundNSRect(NSRect imageFrame);
NSRect RoundNSRect(NSRect imageFrame){
	imageFrame.origin.x = floorf(imageFrame.origin.x);
	imageFrame.origin.y = floorf(imageFrame.origin.y);
	imageFrame.size.width = ceilf(imageFrame.size.width);
	imageFrame.size.height = ceilf(imageFrame.size.height);
	
	return imageFrame;
}

- (id)initWithFrame:(NSRect)frame
{
    self = [super initWithFrame:frame];
    if (self)
    {
		buttonsAndLabels = [[NSMutableArray alloc] init];
    }
    return self;
}

- (void)dealloc
{
	[buttonsAndLabels release];
    [super dealloc];
}

- (void)addButtonWithName:(NSString *)aName target:(id)aTarget startPosition:(NSPoint)start endPosition:(NSPoint)end
{
}

- (void)addButtonWithName:(NSString *)aName toolTip:(NSString *)aToolTip target:(id)aTarget startPosition:(NSPoint)start endPosition:(NSPoint)end{
}
#pragma mark -
- (void)addButtonWithName:(NSString *)aName label:(NSString*)label target:(id)aTarget{
	NSRect labelRect = NSMakeRect(0, 0, 0, 0);
	NSRect buttonRect = NSMakeRect(0, 0, 0, 0);;
	
	//OEControlsKeyButton* button = [[OEControlsKeyButton alloc] initWithFrame:buttonRect];
	NSButton* button = [[NSButton alloc] initWithFrame:buttonRect];
	[button setBezelStyle:NSRoundRectBezelStyle];
    [button setButtonType:NSPushOnPushOffButton];
	
	[button setTarget:aTarget];
    [button setAction:@selector(selectInputControl:)];
    [button bind:@"title" toObject:aTarget withKeyPath:aName options:nil];
	[buttonsAndLabels addObject:button];
	[button release];
	
	NSTextField* labelField = [[NSTextField alloc] initWithFrame:labelRect];
	NSTextFieldCell* labelFieldCell = [[OEControlsKeyLabelCell alloc] init];
	[labelField setCell:labelFieldCell];
	[labelField setStringValue:label];
	[buttonsAndLabels addObject:labelField];
	[labelFieldCell release];
	[labelField release];
}

- (void)nextColumn{
	[buttonsAndLabels addObject:[NSNull null]];
}

- (void)updateButtons{
	//TODO: Method needs cleanup
	while([[self subviews] count]){
		[[[self subviews] lastObject] removeFromSuperview];
	}
	
	// Determine number of columns that we need (using 4 rows)
	int columns = 1;
	int rows = 0;
	for(NSUInteger i=0; i<[buttonsAndLabels count]; i+=2){
		id x = [buttonsAndLabels objectAtIndex:i];
		if(x == [NSNull null] || rows == 4){
			if(x==[NSNull null]) i--;
			rows = 0;
			columns ++;
		} else {
			rows ++;
		}
	}
	
	// Spacing setup
	float topBorder = 34.0;
	
	float leftBorder = 61.0;
	float rightBorder = 21.0;
	
	float verticalItemSpacing	= 9.0;		// item bottom to top
	float horizontalItemSpacing = 68.0;		// item right to item left
	float labelWidth			= 60.0;		// max value!!!
	float labelHeight			= 24.0;
	float labelButtonSpacing	= 8.0;

	if(columns==2){	
		horizontalItemSpacing = 120;
		labelWidth = 112;
	}
	float buttonHeight = 24.0;
	float buttonWidth = (self.frame.size.width-leftBorder-rightBorder-((columns-1)*horizontalItemSpacing))/columns;
	
	int itemIndex;
	int column = 0;
	int row = 0;
	for(itemIndex=0; itemIndex < [buttonsAndLabels count]; itemIndex++){
		id item = [buttonsAndLabels objectAtIndex:itemIndex];
		if(item == [NSNull null]){
			column ++;
			row = 0;
			continue;
		}
		
		if (row==4) {
			column ++;
			row = 0;
		}
		
		NSRect buttonRect = RoundNSRect(NSMakeRect(leftBorder+column*(buttonWidth+horizontalItemSpacing), topBorder+(3-row)*(verticalItemSpacing+buttonHeight), buttonWidth, buttonHeight));
		[item setFrame:buttonRect];
		
		NSRect labelRect = RoundNSRect(NSMakeRect(buttonRect.origin.x-labelWidth-labelButtonSpacing, buttonRect.origin.y-4, labelWidth, labelHeight));
		[[buttonsAndLabels objectAtIndex:itemIndex+1] setFrame:labelRect];
		
		[self addSubview:item];
		[self addSubview:[buttonsAndLabels objectAtIndex:itemIndex+1]];
		
		row++;
		itemIndex++;
	}
}

- (void)setFrame:(NSRect)frameRect{
	[super setFrame:frameRect];
	[self updateButtons];
}
#pragma mark -
- (BOOL)acceptsFirstResponder{
	return YES;
}

- (void)keyUp:(NSEvent *)theEvent{}
- (void)keyDown:(NSEvent *)theEvent{}
@end
